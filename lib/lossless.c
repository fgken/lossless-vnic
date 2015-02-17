#include "lossless.h"

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "debug.h"
#include "list.h"

void lossless_init()
{
	list_init_head(&send_queue);
	list_init_head(&recv_queue);
}

int set_lossless_handshake_header(uint8_t *buf, uint8_t flag, in_addr_t ip, uint16_t port)
{
	struct lossless_handshake_header *hdr;
	uint32_t hdr_size = sizeof(struct lossless_handshake_header);

	hdr = (struct lossless_handshake_header *)buf;
	hdr->flag = flag;
	hdr->ip = ip;
	hdr->port = port;

	return hdr_size;
}

int lossless_accept(struct lossless_context *context, uint16_t port)
{
	int sock;
	struct sockaddr_in addr;
	struct sockaddr_in dest_addr;

	// create an endpoint for lossless connection
	memset(&addr, 0x00, sizeof(addr));
	sock = socket(AF_INET, SOCK_DGRAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0){
		output("Error: bind failed, errno=%d", errno);
		return -1;
	}

	
	// -------------------------------------------
	// --- lossless connection handshake --
	// -------------------------------------------
	{
		uint8_t buf[256];
		uint32_t size;
		struct lossless_handshake_header *hdr;

		output("initiate a lossless connection");
	
		// === handshake 1: SYN ===
		output("handshake 1 - wait for SYN");
		recv(sock, buf, sizeof(buf), 0);
		hdr = (struct lossless_handshake_header *)buf;

		memset(&dest_addr, 0x00, sizeof(dest_addr));
		dest_addr.sin_family = AF_INET;
		dest_addr.sin_port = htons(hdr->port);
		dest_addr.sin_addr.s_addr = hdr->ip;

		// === handshake 2: ACK+SYN ===
		output("handshake 2 - send ACK+SYN");

		size = set_lossless_handshake_header(buf, LOSSLESS_HANDSHAKE_ACKSYN, 0, 0);
		sendto(sock, buf, size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	
		// === handshake 3: ACK ===
		output("handshake 3 - wait for ACK");
		recv(sock, buf, sizeof(buf), 0);
	}

	// save lossless connection context
	context->sock = sock;
	context->dest_addr = dest_addr;
	context->seq_num = 0;
	context->ack_num = 0;

	output("lossless connection is established!");

	return 0;
}

int lossless_connect(struct lossless_context *context, in_addr_t s_addr, uint16_t port)
{
	int sock;
	struct sockaddr_in addr;
	struct sockaddr_in dest_addr;
	in_addr_t src_ip = inet_addr("127.0.0.1");
	uint16_t src_port = 55555;

	// create an endpoint for lossless connection
	memset(&addr, 0x00, sizeof(addr));
	sock = socket(AF_INET, SOCK_DGRAM, 0);

	addr.sin_family = AF_INET;
	addr.sin_port = htons(src_port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0){
		output("Error: bind failed, errno=%d", errno);
		return -1;
	}

	memset(&dest_addr, 0x00, sizeof(dest_addr));

	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = htons(port);
	dest_addr.sin_addr.s_addr = s_addr;

	// -------------------------------------------
	// --- lossless connection handshake --
	// -------------------------------------------
	{
		uint8_t buf[256];
		uint32_t size;

		output("initiate a lossless connection");

		// === handshake 1: SYN ===
		output("handshake 1 - send SYN");

		size = set_lossless_handshake_header(buf, LOSSLESS_HANDSHAKE_SYN, src_ip, src_port);
		sendto(sock, buf, size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));

		// === handshake 2: ACK+SYN ===
		output("handshake 2 - wait for ACK+SYN");
		recv(sock, buf, sizeof(buf), 0);

		// === handshake 3: ACK ===
		output("handshake 3 - send ACK");
		sendto(sock, buf, size, 0, (struct sockaddr *)&dest_addr, sizeof(dest_addr));
	}

	// save lossless connection context
	context->sock = sock;
	context->dest_addr = dest_addr;
	context->seq_num = 0;
	context->ack_num = 0;
	
	output("lossless connection is established!");

	return 0;
}


ssize_t lossless_sendto(struct lossless_context *context, void *buf, size_t len)
{
	struct lossless_header *packet;
	struct lossless_queue_item *item;
	ssize_t ret = 0;

	packet = (struct lossless_header *)malloc(sizeof(struct lossless_header) + len);

	packet->seq_num = context->seq_num;
	packet->ack_num = context->ack_num;
	packet->len = len;
	memcpy(packet->data, buf, len);

	context->seq_num += sizeof(struct lossless_header) + len;

	item = (struct lossless_queue_item *)malloc(sizeof(struct lossless_queue_item));
	item->data = packet;
	list_add(item->list, &send_queue);

	return ret;
}

void lossless_send_thread(void *param)
{
	struct list_head *pos;
	struct lossless_queue_item *item;

	while(1){
		list_for_each(pos, &send_queue){
			item = list_entry(pos, struct lossless_queue_item, list);
			switch(item->send_state){
				case SEND_STATE_WAIT_SEND:
					sendto( context->sock,
							item->data,
							len,
							0,
							(struct sockaddr *)&context->dest_addr,
							sizeof(context->dest_addr)
							);
					break;
				case SEND_STATE_WAIT_ACK:
					// timeout
					item->send_state = SEND_STATE_WAIT_SEND;
					break;
				default:
					break;
			}
		}

		sleep(1);

		while(1) {
			item = list_entry(send_queue->next, struct lossless_queue_item, list);
			if(item->send_state == SEND_STATE_COMPLETE){
				list_del(send_queue->next);
				//free
			}
			else {
				break;
			}
		}
	}
}

ssize_t lossless_recv(struct lossless_context *context, void *buf, size_t len)
{
	struct lossless_queue_item *item;
	//uint32_t seq_num = 1;
	//uint32_t ack_size = 20;

	ssize_t ret = 0;

	while(list_empty(&recv_queue));

	item = list_entry(recv_queue->next, struct lossless_queue_item, list);

	memcpy(data, item->data->data, item->data->len);

	list_del(recv_queue->next);
	free(item);

	return ret;
}

void lossless_recv_thread(void *param)
{
	struct lossless_header *packet;
	int len = 2000;

	struct lossless_queue_item *item;

	packet = (struct lossless_header *)malloc(len);

	while(1){
		recv(context->sock, packet, len, 0);

		if(packet->seq_num == context->ack_num){
			context->ack_num += packet->all_len;

			// send ACK
			item = (struct lossless_queue_item *)malloc(sizeof(struct lossless_queue_item));
			item->data = packet;
			list_add(&item->list, &recv_queue);
	
			packet = (struct lossless_header *)malloc(len);
		}
	}
}

