#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/socket.h>	//socket bind listen accept
#include <arpa/inet.h>	//hton
#include <unistd.h>		//read write sleep

#include <sys/ioctl.h>	//ioctl 设置非阻塞
#include <sys/epoll.h>	//epoll

#include <fcntl.h>

#include <string.h>		//bzero
#include <errno.h>
#include <time.h>

#include <map>
#include <iostream>

#include"config.h"
using namespace std;

struct myevent_s
{
	int fd = 0;											//要监听的文件描述符
	int events = 0;										//对应的监听事件
	void* arg;											//泛型参数
	void(*call_back)(void* arg);						//回调函数

	int status = 0;										//是否在监听：1->在红黑树上（监听）， 0->不在（不监听）
	char buf[DATA_SIZE];								//数据
	int len = 0;
	long long last_active_time = 0;						//最后活跃时间
};

extern int g_efd;
extern struct myevent_s g_events[MAX_EVENTS + 1];

void eventset(struct myevent_s* ev, int fd, void(*call_back)(void*), void* arg);
void eventadd(int efd, int events, struct myevent_s* ev);
void eventdel(int efd, struct myevent_s* ev);

void accept_conn(void* arg);
void recvdata(void* arg);
void senddata(void* arg);

void init();
void run();

#endif
