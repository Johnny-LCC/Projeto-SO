#include "orchestrator.h"

#define buffer_size 308

int main(int argc, char** argv){
	
	if(argc <= 1){
		printf("Execução:\n");
		printf("./orchestrator output_folder parallel-tasks sched-policy\n");
		return 0;
	}
	int fd;
	fd = open(argv[1], O_CREAT | O_TRUNC, 0644);
	close(fd);
	
	mkfifo("server_pipe", 0666);
	
	int cont = 0;
	int client_pipe;
	char pipe_name[6];
	Task buffer;
	ssize_t read_bytes;
	
	while(1){
		int server_pipe = open("server_pipe", O_RDONLY);
		while((read_bytes=read(server_pipe, &buffer, buffer_size))>0){
			//printf("%d - %d\n", buffer.pid_client, cont++);
			//printf("%s\n", buffer.args);
			
			sprintf(pipe_name, "%d", buffer.pid_client);
			client_pipe = open(pipe_name, O_WRONLY);
			
			char r[buffer_size]; //9
			sprintf(r, "TASK %d\n", ++cont);
			write(client_pipe, r, 9);
		}
	}
	
	//close(server_pipe);
	//unlink("server_pipe");
	
	return 0;
}
