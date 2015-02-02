#include <stdint.h>

struct lossless_context {

	int control_sock;
};

struct lossless_header {
	int32_t seq_num;
	int32_t ack_num;
};

