#include "orchestrator.h"

#define buffer_size 1024
#define max_clients 10

int main(int argc, char** argv){
	
	if(argc <= 1){
		printf("Execução:\n");
		printf("./orchestrator output_folder parallel-tasks sched-policy\n");
		return -1;
	}
	int fd;
	fd = open(argv[1], O_CREAT, 0644); //O_TRUNC???
	close(fd);
	
	int cont = 0;
	int client_pipe;
	char pipe_name[6];
	Task task;
	
	//char buffer[buffer_size];
	ssize_t read_bytes;
	
	mkfifo("server_pipe", 0666);
	
	while(1){
		int server_pipe = open("server_pipe", O_RDONLY);
		while((read_bytes=read(server_pipe, &task, sizeof(task)))>0){
			cont++;
			printf("%d - %d\n", task.pid_client, cont);
			//printf("%s\n", task.args[0]);
			
			sprintf(pipe_name, "%d", task.pid_client);
			client_pipe = open(pipe_name, O_WRONLY);
			char r[buffer_size]; //9
			sprintf(r, "TASK %d\n", cont);
			write(client_pipe, r, strlen(r));
		}
	}
	
	//close(server_pipe);
	//unlink("server_pipe");
	
	return 0;
}
