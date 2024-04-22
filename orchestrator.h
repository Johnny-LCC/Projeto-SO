#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

typedef enum{Sent, Scheduled, Executing, Completed} STATUS;

typedef struct task{
	pid_t pid_client;
	char **args;
	int narg;
	STATUS status;
	//int id;
} Task;
