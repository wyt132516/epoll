#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <map>
#include <vector>
#include <iostream>
#include <pthread.h>
#include "command.h"

#define MAX_THREAD_NUM 50   //��ֵĿǰ��Ҫ�趨Ϊ�����߳�����������
#define ADD_FACTOR 4		//��ֵ��ʾһ���߳̿��Դ�������������
#define THREAD_NUM 10		//��ʼ�߳���

class ThreadPool {
public:
	ThreadPool() {};

	static void InitializeThreads();
	void AddWork(Command command);
	void ThreadDestroy(int iwait = 2);

private:
	static void* Process(void* arg);
	static void AddThread();
	static void DeleteThread();
	static bool bshutdown_;
	static int icurr_thread_num_;
	static std::map<pthread_t, int> thread_id_map_;
	static std::vector<Command> command_;
	static pthread_mutex_t command_mutex_;
	static pthread_cond_t command_cond_;
};

#endif