#include "orchestrator.h"

#define buffer_size 1024

int main(int argc, char** argv){
	/*if(argc <= 1){
		printf("Opções:\n");
		printf("./client [time] -u \"[prog] [args]\"\n");
		printf("./client [time] -p \"prog-1 [args] | ... | prog-n [args]\"\n");
		printf("./client status\n");
		
	}*/
	
	char pipe_name[7];
	char buffer[buffer_size];
	ssize_t read_bytes;
	
	srand(time(NULL));
	int x = rand() % 100;
	sprintf(pipe_name, "pipe%d", x);
	
	mkfifo(pipe_name, 0666);
	
	int server_pipe = open("server_pipe", O_WRONLY);
	write(server_pipe, &pipe_name, 7);
	
	int fd = open(pipe_name, O_RDONLY);
	read_bytes = read(fd, &buffer, buffer_size);
	write(1, &buffer, read_bytes);
	close(fd);
	close(server_pipe);
	unlink(pipe_name);
	
	return 0;
}
