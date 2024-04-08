#include "orchestrator.h"

#define buffer_size 1024

int main(int argc, char** argv){
	/*int fd;
	if(argc <= 1){
		printf("Erro na chamada da função\n");
		return 0;
	}
	fd = open(argv[1], O_CREAT, 0644);
	close(fd);*/
	
	int in, out;
	in = mkfifo("pipe_in", 0666);
	out = mkfifo("pipe_out", 0666);
	if(in == -1 || out == -1) printf("ERRO no FIFO\n");
	
	while(1){
		int fd1 = open("pipe_in", O_RDONLY);
		int fd2 = open("pipe_out", O_WRONLY);
		char buffer[buffer_size];
		//char aux[buffer_size];
		ssize_t read_bytes;	
		while((read_bytes=read(fd1, &buffer, buffer_size))>0){
			write(1, &buffer, read_bytes);
			write(fd2, "TASK ACCEPTED\n", 15);
			close(fd2);
		}
	}
	
	return 0;
}
