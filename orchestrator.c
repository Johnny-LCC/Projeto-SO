#include "orchestrator.h"

#define buffer_size 1024
#define max_clients 10

int main(int argc, char** argv){
	/*int fd;
	if(argc <= 1){
		printf("Erro na chamada da função\n");
		return 0;
	}
	fd = open(argv[1], O_CREAT, 0644);
	close(fd);*/
	
	int clients_pipes;
	char pipe_names[7];
	//int pointer = 0;
	
	char buffer[buffer_size];
	ssize_t read_bytes;
	
	mkfifo("server_pipe", 0666);
	
	while(1){
		int server_pipe = open("server_pipe", O_RDONLY);
		while((read_bytes=read(server_pipe, &buffer, buffer_size))>0){
			if(strstr(buffer, "pipe")){
				for(int i = 0; buffer[i]!='\0'; i++) pipe_names[i]=buffer[i];
				clients_pipes = open(pipe_names, O_WRONLY);
				//pointer = (pointer+1) % max_clients;
			}
			write(clients_pipes, "TESTE\n", 7);
		}
	}
	
	/*for(int i=0; i<max_clients; i++){
		close(client_pipes[i]);
		unlink(pipe_names[i]);
		free(pipe_names[i]);
	}
	
	close(server_pipe);
	unlink("server_pipe")*/
	
	return 0;
}
