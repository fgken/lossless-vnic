#include <stdint.h>
#include <netinet/in.h>

struct lossless_context {
	int sock;
	struct sockaddr_in dest_addr;
};

enum {
	LOSSLESS_HANDSHAKE_SYN,
	LOSSLESS_HANDSHAKE_ACKSYN,
	LOSSLESS_HANDSHAKE_ACK,
};

struct lossless_handshake_header {
	uint8_t flag;
	in_addr_t ip;
	int16_t port;
};

struct lossless_header {
	int32_t seq_num;
	int32_t ack_num;
};

int lossless_accept(struct lossless_context *context, uint16_t port);
int lossless_connect(struct lossless_context *context, in_addr_t s_addr, uint16_t port);
ssize_t lossless_sendto(struct lossless_context *context, void *buf, size_t len);
ssize_t lossless_recv(struct lossless_context *context, void *buf, size_t len);

