#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

//typedef enum{Scheduled, Executing} STATUS;

typedef struct task{
	pid_t pid_client;
	int time;
	int pipe;
	char args[300];
	//STATUS status;
} Task;

typedef struct ltask{
	int id;
	Task task;
	struct ltask *prox;
} LTask;
