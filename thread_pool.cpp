#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <windows.h>
#include "thread_pool.h"
#include "thread_process.h"


bool ThreadPool::bshutdown_ = false;
int ThreadPool::icurr_thread_num_ = THREAD_NUM;
std::vector<Command> ThreadPool::command_;
std::map<pthread_t, int> ThreadPool::thread_id_map_;
pthread_mutex_t ThreadPool::command_mutex_ = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t ThreadPool::command_cond_ = PTHREAD_COND_INITIALIZER;

void ThreadPool::InitializeThreads() {
	for (int i = 0; i < THREAD_NUM; ++i) {
		pthread_t tempThread;
		pthread_create(&tempThread, NULL, ThreadPool::Process, NULL);
		thread_id_map_[tempThread] = 0;
	}
}


//此处注意锁的作用域，避免死锁或者被提前释放
void* ThreadPool::Process(void* arg) {
	ThreadProcess threadprocess;
	Command command;
	while (true) {
		pthread_mutex_lock(&command_mutex_);
		//如果线程需要退出，则此时退出
		if (1 == thread_id_map_[pthread_self()]) {
			pthread_mutex_unlock(&command_mutex_);
			printf("thread %u will exit\n", pthread_self());
		}

		//当线程不需要退出且没有需要处理的任务时，需要缩容的缩容，不要的则等待信号
		if (0 == command_.size() && !bshutdown_) {
			if (icurr_thread_num_ > THREAD_NUM) {
				DeleteThread();
				if (1 == thread_id_map_[pthread_self()]) {
					pthread_mutex_unlock(&command_mutex_);
					printf("thread %u will exit\n", pthread_self());
					pthread_exit(NULL);
				}
			}
			pthread_cond_wait(&command_cond_, &command_mutex_);
		}

		//线程池需要关闭，关闭已有的锁，线程退出
		if (bshutdown_) {
			pthread_mutex_unlock(&command_mutex_);
			printf("thread %u will exit\n", pthread_self());
			pthread_exit(NULL);
		}

		//如果线程池的最大线程数不等于初始线程数，则表明需要扩容
		if (icurr_thread_num_ < command_.size()) {
			AddThread();
		}

		//从容器中取出代办任务
		std::vector<Command>::iterator it = command_.begin();
		command.set_arg(it->get_arg());
		command.set_cmd(it->get_cmd());
		command_.erase(it);
		pthread_mutex_unlock(&command_mutex_);
		//开始处理业务
		switch (command.get_cmd()) {
		case 0:
			threadprocess.Process0(command.get_arg());
			break;
		case 1:
			threadprocess.Process1(command.get_arg());
			break;
		case 2:
			threadprocess.Process2(command.get_arg());
			break;
		default:
			printf("something unknow happend! you should extend new task!");
			break;
		}
	}

	//return NULL; 此处可能产生警告，注意!
}

void ThreadPool::AddWork(Command command) {
	bool bsignal = false;
	pthread_mutex_unlock(&command_mutex_);
	//判断当前任务数，是否已经添加过相应的任务
	if (0 == command_.size()) {
		bsignal = true;
	}

	command_.push_back(command);
	pthread_mutex_unlock(&command_mutex_);
	if (bsignal) {
		pthread_cond_signal(&command_cond_);
		printf("AddWork successfully!")!
	}
	else {
		printf("AddWork failed! Might be Already added!");
	}
}

void ThreadPool::ThreadDestroy(int iwait /* = 2 */) {
	while (0 != command_.size()) {
		sleep(abs(iwait));
	}
	bshutdown_ = true;
	pthread_cond_broadcast(&command_cond_);
	std::map<pthread_t, int>::iterator it = thread_id_map_.begin();
	for (; it != thread_id_map_.end(); ++it) {
		pthread_join(it->first, NULL);
	}
	pthread_mutex_destroy(&command_mutex_);
	pthread_cond_destroy(&command_cond_);
}

void ThreadPool::AddThread() {
	//判断当前的线程数能处理的任务总量和任务总数，以及判断当前线程数，线程上限50个，任务数上限200
	if (((icurr_thread_num_*ADD_FACTOR) < command_.size()) 
		&& (MAX_THREAD_NUM != icurr_thread_num_)) {
		InitializeThreads();
		icurr_thread_num_ += THREAD_NUM;
	}
}

void ThreadPool::DeleteThread() {
	int size = icurr_thread_num_ - THREAD_NUM;
	std::map<pthread_t, int>::iterator it = thread_id_map_.begin();
	for (int i = 0; i < size; ++i, ++it) {
		it->second = 1;
	}
}