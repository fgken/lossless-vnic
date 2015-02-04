#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "lossless.h"

int main()
{
	int sock;
	struct sockaddr_in addr;

	char buf[2048];

	struct lossless_context context;
	lossless_connect(&context, inet_addr("127.0.0.1"), 50000);

	puts("send \"HELLO\"");
	lossless_sendto(&context, "HELLO", sizeof("HELLO"));

	return 0;
}
