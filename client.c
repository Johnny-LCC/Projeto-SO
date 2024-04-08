#include "orchestrator.h"

#define buffer_size 1024

int main(int argc, char** argv){
	/*if(argc <= 1){
		printf("Opções:\n");
		printf("./client [time] -u \"[prog] [args]\"\n");
		printf("./client [time] -p \"prog-1 [args] | ... | prog-n [args]\"\n");
		printf("./client status\n");
		
	}*/
	
	int fd1 = open("pipe_in", O_WRONLY);
	int fd2 = open("pipe_out", O_RDONLY);
	
	char buffer[buffer_size];
	//char aux[buffer_size];
	ssize_t read_bytes, bytes;	
	read_bytes=read(0, &buffer, buffer_size);
	write(fd1, &buffer, read_bytes);
	while((bytes=read(fd2, &buffer, buffer_size))>0){
		write(1, &buffer, bytes);
	}
	close(fd1);
	//close(fd2);
	
	return 0;
}
