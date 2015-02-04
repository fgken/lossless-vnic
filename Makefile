CC=gcc
INCLUDE=-I./lib
SERVER_SRC=server.c lib/lossless.c
SERVER_OBJS:= $(SERVER_SRC)    #$(SERVER_SRC:.c=.o)
CLIENT_SRC=client.c lib/lossless.c
CLIENT_OBJS:= $(CLIENT_SRC)    #$(CLIENT_SRC:.c=.o)

all: server client

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $(SERVER_OBJS)

client: $(CLIENT_OBJS)
	$(CC) $(CFLAGS) $(INCLUDE) -o $@ $(CLIENT_OBJS)

clean:
	rm client server
