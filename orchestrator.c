#include "../include/orchestrator.h"

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

void insertOrd(LTask *l, Task x){ //SJF
	LTask new = malloc(sizeof(struct ltask));
	new->task = x;
	new->prox = NULL;
	LTask prev = NULL;
	for(; *l && (*l)->task.time <= x.time; prev=(*l), l=&((*l)->prox));
	if(prev){
		new->prox = (*l);
		prev->prox = new;
	}
	else{
		new->prox = (*l);
		*l = new;
	}
}

int rem(LTask *l, Task t){
	LTask prev = NULL;
	for(; (*l) && (*l)->task.id!=t.id; prev=(*l), l=&((*l)->prox));
	if(!(*l)) return 1;
	if(!prev) (*l) = (*l)->prox;
	else prev->prox = (*l)->prox;
	return 0;
}

void append(LTask *l, Task t){ //FCFS
	LTask new = malloc(sizeof(struct ltask));
	new->task = t;
	new->prox = NULL;
	if(!(*l)) (*l)=new;
	else{
		for(; (*l)->prox; l=&((*l)->prox));
		(*l)->prox = new;
	}
}

int main(int argc, char** argv){
	if(argc <= 3){
		printf("Execução:\n");
		printf("./orchestrator output_folder parallel-tasks sched-policy\n");
		return -1;
	}
	
	int esc;
	if(strcmp(argv[3], "FCFS")==0 || strcmp(argv[3], "fcfs")==0) esc = 0;
	else if(strcmp(argv[3], "SJF")==0 || strcmp(argv[3], "sjf")==0) esc = 1;
	else{
		printf("Erro - escalonamento inválido\n");
		return -1;
	}
	
	char log_file[32];
	sprintf(log_file, "../tmp/%s", argv[1]);
	int log_fd;
	log_fd = open(log_file, O_CREAT | O_TRUNC | O_RDWR, 0644);
	if(log_fd == -1) perror("LOG: ");
	close(log_fd); //
	
	mkfifo("server_pipe", 0666);
	int server_pipe;
	
	int client_pipe;
	char pipe_name[6];
	
	int id_task = 0;
	int n_ptask = atoi(argv[2]);
	
	Task buffer;
	int buffer_size = sizeof(struct task);
	ssize_t read_bytes;
	LTask sch = NULL;
	LTask ex = NULL;
	
	while(id_task < 1000){
		server_pipe = open("server_pipe", O_RDONLY);
		while((read_bytes=read(server_pipe, &buffer, buffer_size))>0){
			sprintf(pipe_name, "%d", buffer.pid_client);
			client_pipe = open(pipe_name, O_WRONLY);
			
			if(strcmp(buffer.args, "status") == 0){
				pid_t pid = fork();
				if(pid == 0){
					LTask l[2];
					l[0] = sch; l[1] = ex;
					char *aux[] = {"Scheduled:", "Executing:"};
					char r[1024];
					for(int i=0; i<2; i++){
						sprintf(r, "%s\n", aux[i]);
						write(client_pipe, &r, strlen(r));
						if(l[i]==NULL) write(client_pipe, "-----\n", 6); 
						while(l[i]!=NULL){
							sprintf(r, "%d: %s\n", l[i]->task.id, l[i]->task.args);
							write(client_pipe, &r, strlen(r));
							l[i]=l[i]->prox;
						}
					}
					
					sprintf(r, "Completed:\n");
					write(client_pipe, &r, strlen(r));
					
					int fd;
					fd = open(log_file, O_RDONLY);
					if (fd == -1) perror("STATUS: ");
					Task comp;
					while((read_bytes=read(fd, &comp, buffer_size))>0){
						sprintf(r, "%d: %s - %dms\n", comp.id, comp.args, comp.time);
						write(client_pipe, &r, strlen(r));
					}
					close(fd);
					close(client_pipe);
				}
				else{
					close(client_pipe);
				}
			}
			else if((strcmp(buffer.args, "rem") == 0)){
					close(client_pipe);
					rem(&ex, buffer);
			}
			else{
				id_task++;
				buffer.id = id_task;
				char r[1024];
				sprintf(r, "TASK %d Received\n", id_task);
				write(client_pipe, &r, strlen(r));
				close(client_pipe);
				
				if(esc == 0) append(&sch, buffer);
				else insertOrd(&sch, buffer);
			}
		}
		
		int net = length(&ex);
		if(net < n_ptask){
			Task task = sch->task;
			LTask temp = sch;
			sch = sch->prox;
			free(temp);
			insertOrd(&ex, task);
			
			char aux_rem[32];
			sprintf(aux_rem, "./client rem %d", task.id);
			char *rem[3];
			command(rem, aux_rem);
			
			struct timeval start, end;
								
			if(task.pipe==0){ //-u
				char *cmd[10];
				command(cmd, task.args);
				
				pid_t pid = fork();
				if(pid==0){
					char aux[16];
					sprintf(aux, "../tmp/Task%d", id_task);
					int task_fd = open(aux, O_CREAT | O_WRONLY, 0644);
					if(task_fd == -1) perror("TASK: ");
				
					dup2(task_fd, 1);
					close(task_fd);
					gettimeofday(&start, NULL);
					
					pid_t ppid = fork();
					if(ppid == 0){
						execvp(cmd[0], cmd);
						_exit(0);
					}
					
					int status;
					wait(&status);
					gettimeofday(&end, NULL);
					task.time = (end.tv_sec*1000 + end.tv_usec) - (start.tv_sec*1000 + start.tv_usec);
					
					int fd;
					fd = open(log_file, O_WRONLY);
					if(fd == -1) perror("EXECUTE: ");
					lseek(fd, 0, SEEK_END);
					write(fd, &task, buffer_size);
					close(fd);
					
					execvp(rem[0], rem);
					_exit(0);
				}
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
							char aux[16];
							sprintf(aux, "../tmp/Task%d", id_task);
							int task_fd = open(aux, O_CREAT | O_WRONLY, 0644);
							if(task_fd == -1) perror("TASK: ");
						
							dup2(fd[i-1][0], 0);
							close(fd[i-1][0]);
							dup2(task_fd, 1);
							close(task_fd);
							gettimeofday(&start, NULL);
							
							pid_t ppid = fork();
							if(ppid == 0){
								execvp(cmd[0], cmd);
								_exit(0);
							}
							
							int status;
							wait(&status);
							gettimeofday(&end, NULL);
							task.time = (end.tv_sec*1000 + end.tv_usec) - (start.tv_sec*1000 + start.tv_usec);
							
							int fd;
							fd = open(log_file, O_WRONLY);
							if(fd == -1) perror("EXECUTE: ");
							lseek(fd, 0, SEEK_END);
							write(fd, &task, buffer_size);
							close(fd);
							
							execvp(rem[0], rem);
							
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
					wait(&status);
					if(!WIFEXITED(status)){ 
						write(client_pipe, "Task Error\n", 11);
					}
				}
			}
		}
		
		close(server_pipe);
	}
	unlink("server_pipe");
	
	return 0;
}
