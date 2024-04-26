#include "orchestrator.h"

#define buffer_size 312

int func_aux(char *dest[], char* src){
	int r=0;
	char* command = strdup(src);
	char *string;
	string=strtok(command,"|");
	
	while(string!=NULL){
		dest[r]=string;
		string=strtok(NULL,"|");
		r++;
	}
	return r;
}

void command(char *cmd[], char* arg){
	int i=0;
	char* command = strdup(arg);
	char *string;
	string=strtok(command," ");
	
	while(string!=NULL){
		cmd[i]=string;
		string=strtok(NULL," ");
		i++;
	}
	cmd[i] = NULL;
}

int main(int argc, char** argv){
	if(argc <= 1){
		printf("Execução:\n");
		printf("./orchestrator output_folder parallel-tasks sched-policy\n");
		return -1;
	}
	int log_fd;
	log_fd = open(argv[1], O_CREAT | O_TRUNC, 0644);
	close(log_fd);
	
	mkfifo("server_pipe", 0666);
	int server_pipe;
	
	int cont = 0;
	int client_pipe;
	char pipe_name[6];
	Task buffer;
	ssize_t read_bytes;
	
	while(cont < 100){
		server_pipe = open("server_pipe", O_RDONLY);
		while((read_bytes=read(server_pipe, &buffer, buffer_size))>0){
			//printf("%d - %d\n", buffer.pid_client, cont+1);
			//printf("%s - %d ms\n", buffer.args, buffer.time);
			
			sprintf(pipe_name, "%d", buffer.pid_client);
			client_pipe = open(pipe_name, O_WRONLY);
			
			if(strcmp(buffer.args, "status") == 0){
				pid_t pid = fork();
				if(pid == 0) write(client_pipe, "FICHEIRO LOG\n", 13);
			}
			else{
				char r[buffer_size];
				cont++;
				sprintf(r, "TASK %d Received\n", cont);
				write(client_pipe, r, sizeof(r));
				
				char aux[16];
				sprintf(aux, "Task%d", cont); //"../tmp/Task%d"
				int task_fd = open(aux, O_CREAT | O_WRONLY, 0644);
				
				if(buffer.pipe==0){ //-u
					char *cmd[10];
					command(cmd, buffer.args);
					pid_t pid = fork();
					if(pid==0){
						dup2(task_fd, 1);
						close(task_fd);
						execvp(cmd[0], cmd);
						_exit(0);
					}
					else{
						//close(task_fd);
						int status;
						wait(&status);
						if(!WIFEXITED(status)) printf("Erro - Task %d\n", cont);
					}
				}
				else{ //-p
					char *arg[10];
					int N = func_aux(arg, buffer.args);
					int fd[N-1][2];
					
					for(int i=0; i<N; i++){
						char *cmd[10];
						command(cmd, arg[i]);
						if(i==0){
							//printf("%d\n", i);
							pipe(fd[i]);
							pid_t pid = fork();
							if(pid==0){
								close(fd[i][0]);
								dup2(fd[i][1], 1);
								close(fd[i][1]);
								execvp(cmd[0], cmd);
								_exit(0);
							}
							else{
								close(fd[i][1]);
							}
						}
						else if(i==N-1){
							//printf("%d\n", i);
							pid_t pid = fork();
							if(pid==0){
								dup2(fd[i-1][0], 0);
								close(fd[i-1][0]);
								execvp(cmd[0], cmd);
								_exit(0);
							}
							else{
								close(fd[i-1][0]);
							}
						}
						else{
							//printf("%d\n", i);
							pipe(fd[i]);
							pid_t pid = fork();
							if(pid==0){
								dup2(fd[i-1][0], 0);
								close(fd[i-1][0]);
								dup2(fd[i][1], 1);
								close(fd[i][1]);
								
								close(fd[i][0]);
								execvp(cmd[0], cmd);
								_exit(0);
							}
							else{
								close(fd[i][1]);
								close(fd[i-1][0]);
							}
						}
					}
					
					int status;
					for(int i=0; i<N; i++){
						pid_t tpid = wait(&status);
						if(!WIFEXITED(status)){
							printf("pid do filho: %d\n", tpid);
						}
					}
				}
				
				close(task_fd);
			}
		}
		close(server_pipe);
	}
	
	unlink("server_pipe");
	
	return 0;
}
