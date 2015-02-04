#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lossless.h"

int main()
{
	int sock;
	struct sockaddr_in addr;
	uint32_t size;

	char buf[2048];

	struct lossless_context context;
	lossless_accept(&context, 50000);

	size = lossless_recv(&context, buf, sizeof(buf));
	printf("recv: %s\n", buf);

	return 0;
}

