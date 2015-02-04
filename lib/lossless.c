#include "lossless.h"

#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#include "debug.h"

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
	
	output("lossless connection is established!");

	return 0;
}

ssize_t lossless_sendto(struct lossless_context *context, void *buf, size_t len)
{
	char ll_buf[2048];
	struct lossless_header *llhdr;

	char ack_buf[16];
	ssize_t ret;

	uint32_t seq_num = 1;

	return sendto(context->sock, buf, len, 0, (struct sockaddr *)&context->dest_addr,
			sizeof(context->dest_addr));

//	llhdr = (struct lossless_header *)ll_buf;
//
//	llhdr->seq_num = seq_num;
//
//	memcpy(ll_buf+sizeof(struct lossless_header), buf, len);
//
//	while(1) {
//		// send
//		ret = sendto(sockfd, ll_buf, len+sizeof(struct lossless_header), flags,
//				dest_addr, addrlen);
//
//		// wait ack
//		recv(sockfd, ack_buf, sizeof(ack_buf), 0);
//		if (((struct lossless_header *)ack_buf)->seq_num == seq_num){
//			seq_num++;
//			return ret;
//		}
//	}
}

ssize_t lossless_recv(struct lossless_context *context, void *buf, size_t len)
{
	char ll_buf[2048];
	char ack_buf[16];
	uint32_t seq_num = 1;
	uint32_t ack_size = 20;

	ssize_t ret;

	return recv(context->sock, buf, len, 0);

	//while(1) {
	//	// recv
	//	ret = recv(sockfd, ll_buf, sizeof(ll_buf), flags);

	//	if (((struct lossless_header *)ll_buf)->seq_num != seq_num){
	//		// re-send
	//		//sendto(sockfd, 
	//		continue;
	//	}

		// ack
//		sendto(sockfd, ack_buf, sizeof(ack_size), 0);
//	}
}

