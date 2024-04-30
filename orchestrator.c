#include "orchestrator.h"

#define buffer_size 316

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

int length(LTask *l){
	int r=0;
	LTask aux = *l;
	while(aux!=NULL){
		r++;
		aux=aux->prox;
	}
	return r;
}

void insertOrd(LTask* l, Task t){ //SJF
	LTask new = malloc(sizeof(struct ltask));
	new->task = t;
	new->prox = NULL;
	LTask prev = NULL, aux = *l;
	for(; aux && aux->task.time < t.time; prev=aux, l=&(aux->prox));
	if(prev){
		new->prox = aux;
		prev->prox = new;
	}
	else{
		new->prox = aux;
		aux = new;
	}
}

int removeOrd(LTask *l, Task t){
	LTask prev = NULL;
	LTask aux = *l;
	for(; aux && aux->task.id!=t.id; prev=aux, l=&(aux->prox));
	if(!aux) return 1;
	if(!prev) aux = aux->prox;
	else prev->prox = aux->prox;
	free(aux);
	return 0;
}

void append(LTask *l, Task t){ //FCFS
	LTask new = malloc(sizeof(struct ltask));
	new->task = t;
	new->prox = NULL;
	LTask aux = *l;
	if(!aux) (*l)=new;
	else{
		for(; aux->prox; l=&(aux->prox));
		aux->prox = new;
	}
}

int main(int argc, char** argv){
	if(argc <= 3){
		printf("Execução:\n");
		printf("./orchestrator output_folder parallel-tasks sched-policy\n");
		return -1;
	}
	//int buffer_size = sizeof(struct task);
	
	int log_fd, ls=0;
	log_fd = open(argv[1], O_CREAT | O_RDWR, 0644); //O_TRUNC?
	//close(log_fd); ///
	
	mkfifo("server_pipe", 0666);
	int server_pipe;
	
	int client_pipe;
	char pipe_name[6];
	
	int id_task = 0;
	int n_ptask = atoi(argv[2]);
	
	Task buffer;
	ssize_t read_bytes;
	LTask sch = NULL;
	LTask ex = NULL;
	
	while(id_task < 100){
		server_pipe = open("server_pipe", O_RDONLY);
		while((read_bytes=read(server_pipe, &buffer, buffer_size))>0){
			sprintf(pipe_name, "%d", buffer.pid_client);
			client_pipe = open(pipe_name, O_WRONLY);
			
			if(strcmp(buffer.args, "status") == 0){
				pid_t pid = fork();
				if(pid == 0){
					
					LTask l[2]; //
					l[0] = sch; l[1] = ex; //
					char *aux[] = {"Scheduled:", "Executing:"};
					char r[1024];
					for(int i=0; i<2; i++){
						sprintf(r, "%s\n", aux[i]);
						write(client_pipe, &r, strlen(r));
						if(l[i]==NULL) write(client_pipe, "-----\n", 6); 
						while(l[i]!=NULL){
							sprintf(r, "%d: %s - %dms\n", l[i]->task.id, l[i]->task.args, l[i]->task.time);
							write(client_pipe, &r, strlen(r)); //
							l[i]=l[i]->prox;
						}
					}
					
					sprintf(r, "Completed: FICHEIRO LOG\n");
					write(client_pipe, &r, strlen(r));
					lseek(log_fd, -ls, SEEK_END);
					while((read_bytes=read(log_fd, &buffer, buffer_size))>0){
						sprintf(r, "%d: %s - %dms\n", buffer.id, buffer.args, buffer.time);
						write(client_pipe, &r, strlen(r));
					}
					
				}
			}
			else{
				id_task++;
				buffer.id = id_task;
				char r[1024];
				sprintf(r, "TASK %d Received\n", id_task);
				write(client_pipe, &r, strlen(r));
				
				char aux[16];
				sprintf(aux, "Task%d", id_task);
				int task_fd = open(aux, O_CREAT | O_WRONLY, 0644);
				
				if(strcmp(argv[3], "FCFS")==0 || strcmp(argv[3], "fcfs")==0) append(&sch, buffer);
				else insertOrd(&sch, buffer);
				
				int net = length(&ex);
				if(net < n_ptask){
					Task task = sch->task;
					LTask temp = sch;
					sch = sch->prox;
					free(temp);
					insertOrd(&ex, task);
					
					//log_fd = open(argv[1], O_CREAT|O_WRONLY, 0644); ///
					
					if(task.pipe==0){ //-u
						char *cmd[10];
						command(cmd, task.args);
						pid_t pid = fork();
						if(pid==0){
							dup2(task_fd, 1);
							close(task_fd);
							execvp(cmd[0], cmd);
							removeOrd(&ex, task);
							//write(log_fd, &task, buffer_size); ///
							_exit(0);
						}
						else{
							close(task_fd);
							int status;
							wait(&status);
							if(!WIFEXITED(status)) printf("Erro - Task %d\n", id_task);
						}
						write(log_fd, &task, buffer_size);
						ls += buffer_size;
					}
					else{ //-p
						char *arg[10];
						int N = func_aux(arg, task.args);
						int fd[N-1][2];
						
						for(int i=0; i<N; i++){
							char *cmd[10];
							command(cmd, arg[i]);
							if(i==0){
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
								pid_t pid = fork();
								if(pid==0){
									dup2(fd[i-1][0], 0);
									close(fd[i-1][0]);
									dup2(task_fd, 1);
									close(task_fd);
									execvp(cmd[0], cmd);
									removeOrd(&ex, task);
									//write(log_fd, &task, buffer_size); ///
									_exit(0);
								}
								else{
									close(fd[i-1][0]);
								}
							}
							else{
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
						write(log_fd, &task, buffer_size);
						ls += buffer_size;
					}
					//close(log_fd);///
				}
				close(task_fd);
			}	
			close(client_pipe);
		}
		close(server_pipe);
	}
	close(log_fd);
	unlink("server_pipe");
	
	return 0;
}
