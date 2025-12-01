CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: channel subscriber publisher

channel: channel.o broker.o gateway.o publisher.o
	$(CC) $(CFLAGS) -o channel channel.o broker.o gateway.o publisher.o
subscriber: subscriber.o 
	$(CC) $(CFLAGS) -o subscriber subscriber.o

publisher: publisher.c publisher.h
	$(CC) $(CFLAGS) -DSTANDALONE -o publisher publisher.c
channel.o: channel.c broker.h gateway.h publisher.h
	$(CC) $(CFLAGS) -c channel.c

subscriber.o: subscriber.c subscriber.h
	$(CC) $(CFLAGS) -c subscriber.c
publisher.o: publisher.c publisher.h
	$(CC) $(CFLAGS) -c publisher.c
broker.o: broker.c broker.h
	$(CC) $(CFLAGS) -c broker.c
gateway.o: gateway.c gateway.h
	$(CC) $(CFLAGS) -c gateway.c
clean:
	rm -f *.o channel subscriber publisher
.PHONY: all clean
