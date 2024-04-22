#include "orchestrator.h"

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
	task.args = argv;
	task.narg = argc;
	task.status = Sent;
	//task.id = 0;
	
	char pipe_name[6];
	char buffer[buffer_size];
	ssize_t read_bytes;
	
	sprintf(pipe_name, "%d", task.pid_client);
	mkfifo(pipe_name, 0666);
	
	int server_pipe = open("server_pipe", O_WRONLY);
	write(server_pipe, &task, sizeof(task));
	
	int fd = open(pipe_name, O_RDONLY);
	read_bytes = read(fd, &buffer, buffer_size);
	write(1, &buffer, read_bytes);
	close(fd);
	close(server_pipe);
	unlink(pipe_name);
	
	return 0;
}
