#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <wait.h>
using namespace std;

#define MAX_SEQUENCE 10

typedef struct
{
	long fib_sequence[MAX_SEQUENCE];
	int sequence_size;
} shared_data;

int main(int argc, char **argv) {
	key_t shmid;
	shared_data *p_pointer, *c_pointer;
	//定义两个指针 p_pointer表示父进程中指向共享内存区域的指针， c_pointer表示子进程中指向共享内存区域的指针
	pid_t pid;

	if (argc != 2) { //如果传入参数不合法就输出样式
		fprintf(stderr, "Usage:./a.out fib_size\n\a");
		exit(1);
	}
	if (atof(argv[1]) > 10 || atof(argv[1]) < 1) {   //判断数组大小合法性
		fprintf(stderr, "use number between 1~10\n\a");
		exit(1);
	}

	shared_data sd;
	sd.sequence_size = atof(argv[1]);    //把命令行参数传给shared_data

	shmid = shmget(IPC_PRIVATE, 1024, S_IRUSR|S_IWUSR);
	// 分配共享内存区
	if ( shmid == -1) {
		fprintf(stderr, "Create Share Memory Error:%s\n\a", strerror(errno));
		exit(1);
	}

	p_pointer = (shared_data*)shmat(shmid, NULL, 0);
	// 连接共享内存区
	memset(p_pointer, '\0', sizeof(shared_data));
	// 数据清零

	for (int i = 0; i < MAX_SEQUENCE; i++) {
		p_pointer->fib_sequence[i] = sd.fib_sequence[i];
	}

	p_pointer->sequence_size = sd.sequence_size;

	pid = fork();
	if (pid < 0) {
		/* error occurred */
		fprintf(stderr, "Fork Failed");
		exit(-1);
	}
	else if (pid == 0) {
		/* child process */
		c_pointer = (shared_data*)shmat(shmid, NULL, 0);
		//连接内存共享区

		c_pointer->fib_sequence[0] = 1;
		c_pointer->fib_sequence[1] = 1;
		for (int i = 2; i < c_pointer->sequence_size; i++) {
			c_pointer->fib_sequence[i] = c_pointer->fib_sequence[i - 1]
					+ c_pointer->fib_sequence[i - 2];
		}
		exit(0);
	} else {
		/* parent process */
		wait(NULL);
		for (int i = 0; i < p_pointer->sequence_size; i++) {
			printf("%d ", (p_pointer->fib_sequence[i]));
		}
		exit(0);
	}

	return 0;
}
