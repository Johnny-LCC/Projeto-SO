CC = gcc
CFLAGS = -Wall -g -Include
LDFLAGS = 

all: folders server client

server: bin/orchestrator

client: bin/client

folders:
	@mkdir -p src include obj bin tmp
	
bin/orchestrator: obj/orchestrator.o
	$(CC) $(CFLAGS) $^ -o $@
	
bin/client: obj/client.o
	$(CC) $(LDFLAGS) $^ -o $@
	
obj/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@
	
clean:
	rm -f obj/* tmp/* bin/*
