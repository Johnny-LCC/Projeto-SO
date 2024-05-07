#include "../include/orchestrator.h"

#define buffer_size 1024

int main(int argc, char** argv){
	if(argc <= 1){
		printf("Opções:\n");
		printf("./client execute [time] -u \"[prog] [args]\"\n");
		printf("./client execute [time] -p \"prog-1 [args] | ... | prog-n [args]\"\n");
		printf("./client status\n");
		return -1;
	}
	
	Task task;
	task.pid_client = getpid();
	task.id = -1;
	if(strcmp(argv[1], "execute") == 0){
		task.time = atoi(argv[2]);
		if(strcmp(argv[3], "-p")==0) task.pipe = 1;
		else if (strcmp(argv[3], "-u")==0) task.pipe = 0;
		else{
			printf("Flag inválida\n");
			return -1;
		}
		strcpy(task.args, argv[4]);
	}
	else if(strcmp(argv[1], "status") == 0){
		task.time = task.pipe = 0;
		strcpy(task.args, argv[1]);
	}
	else if(strcmp(argv[1], "rem") == 0){
		task.time = task.pipe = 0;
		strcpy(task.args, argv[1]);
		task.id = atoi(argv[2]);
	}
	else{
		printf("Erro\n");
		return -1;
	}
	
	char pipe_name[6];
	char buffer[buffer_size];
	ssize_t read_bytes;
	
	sprintf(pipe_name, "%d", task.pid_client);
	mkfifo(pipe_name, 0666);
	
	int server_pipe = open("server_pipe", O_WRONLY);
	write(server_pipe, &task, sizeof(task));
	
	int fd = open(pipe_name, O_RDONLY);
	while((read_bytes = read(fd, &buffer, buffer_size)) > 0){
		write(1, &buffer, read_bytes);
	}
	
	close(fd);
	close(server_pipe);
	unlink(pipe_name);
	
	return 0;
}
