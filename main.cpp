#include <stdio.h>
#include <iostream>
#include "command.h"
#include "thread_pool.h"

using namespace std;

int main() {
	ThreadPool thread_pool;
	thread_pool.InitializeThreads();
	Command command;
	char arg[8] = { 0 };
	for (int i = 1; i < 200; ++i) {
		command.set_cmd(i % 3);
		sprintf(arg, "%d", i);
		command.set_arg(arg);
		thread_pool.AddWork(command);
	}

	sleep(30);//�˴����ڲ����̳߳�����
	thread_pool.ThreadDestroy();
	return 0;
}