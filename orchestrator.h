#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

typedef struct task{
	pid_t pid_client;
	int id;
	int time;
	int pipe;
	char args[300];
} Task;

typedef struct ltask{
	Task task;
	struct ltask *prox;
} *LTask;
