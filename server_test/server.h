#ifndef __SERVER_H__
#define __SERVER_H__

#include <sys/socket.h>	//socket bind listen accept
#include <arpa/inet.h>	//hton
#include <unistd.h>		//read write sleep

#include <sys/ioctl.h>	//ioctl ���÷�����
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
	int fd = 0;											//Ҫ�������ļ�������
	int events = 0;										//��Ӧ�ļ����¼�
	void* arg;											//���Ͳ���
	void(*call_back)(void* arg);						//�ص�����

	int status = 0;										//�Ƿ��ڼ�����1->�ں�����ϣ��������� 0->���ڣ���������
	char buf[DATA_SIZE];								//����
	int len = 0;
	long long last_active_time = 0;						//����Ծʱ��
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
