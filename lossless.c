#include "lossless.h"

#include <stdint.h>

ssize_t lossless_sendto(int sockfd, const void *buf, size_t len, int flags)
{
	char ll_buf[2048];
	struct LOSSLESS_HEADER *llhdr;

	char ack_buf[16];
	ssize_t ret;

	uint32_t seq_num = 1;

	llhdr = ll_buf;

	llhdr->seq_num = seq_num;

	memcopy(ll_buf+sizeof(struct LOSSLESS_HEADER), buf, len);

	while(1){
		// send
		ret = sendto(sockfd, ll_buf, len+sizeof(LOSSLESS_HEADER), flags);

		// wait ack
		recv(sockfd, ack_buf, sizeof(ack_buf), 0);
		if(((struct LOSSLESS_HEADER *)ack_buf)->seq_num == seq_num){
			seq_num++;
			return ret;
		}
	}
}

ssize_t lossless_recv(int sockfd, void *buf, size_t len, int flags)
{
	char ll_buf[2048];
	char ack_buf[16];
	uint32_t seq_num = 1;
	uint32_t ack_size = 20;

	ssize_t ret;

	while(1){
		// recv
		ret = recv(sockfd, ll_buf, sizeof(ll_buf), flags);

		if((struct LOSSLESS_HEADER *)->seq_num != seq_num){
			// re-send
			sendto(sockfd, 
			continue;
		}

		// ack
		sendto(sockfd, ack_buf, sizeof(ack_size), 0);
	}
}

