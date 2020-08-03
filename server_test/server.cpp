#include "server.h"
int g_efd;
struct myevent_s g_events[MAX_EVENTS + 1];

void run()
{
	cout << "========  run...  ========" << endl;

	struct epoll_event events[MAX_EVENTS + 1];

	int i = 0, check_pos = 0;
	while (true)
	{
		//��Ծ��ʱȥ��
	/*
		long long now_time = time(NULL);
		for (i = 0; i < 100; i++, check_pos++)
		{
			if (check_pos == MAX_EVENTS)
			{
				check_pos = 0;
			}

			if (g_events[check_pos].status != 1)
			{
				continue;
			}
			long long duration = now_time - g_events[check_pos].last_active_time;
			if (duration >= 6000)
			{
				close(g_events[check_pos].fd);
				cout << "fd: " << g_events[check_pos].fd << " no active del" << endl;
				eventdel(g_efd, &g_events[check_pos]);
			}
		}
*/
		int nfd = epoll_wait(g_efd, events, MAX_EVENTS + 1, -1);
		if (nfd < 0)
		{
			break;
		}

		for (i = 0; i < nfd; i++)
		{
			myevent_s* ev = (myevent_s*)events[i].data.ptr;

			if (((events[i].events & EPOLLIN) &&
				(ev->events & EPOLLIN)) ||
				((events[i].events & EPOLLOUT) &&
				(ev->events & EPOLLOUT)))
			{
				ev->call_back(ev->arg);
			}
		}
	}
}

void init()
{
	cout << "========  hello  ========" << endl;
	cout << "=====  talk server  =====" << endl;

	g_efd = epoll_create(MAX_EVENTS + 1);

	if (g_efd < 0)
	{
		cerr << "g_efd init error" << endl;
	}

	cout << "====  server init ... " << endl;

	int lfd = socket(AF_INET, SOCK_STREAM, 0);

	// ���� ��ַ����
	socklen_t val = 1;
	if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) < 0)
	{
		cerr << "setsockopt  error : " << endl;
		close(lfd);
		return;
	}
	//������
	fcntl(lfd, F_SETFL, O_NONBLOCK);

	//eventset(struct myevent_s *ev , lfd, void(*call_back)(int,int,void*), void* arg));
	eventset(&g_events[MAX_EVENTS], lfd, accept_conn, &g_events[MAX_EVENTS]);
	eventadd(g_efd, (EPOLLIN | EPOLLET), &g_events[MAX_EVENTS]);

	//����Ϣ
	struct sockaddr_in addr;
	bzero(&addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(lfd, (sockaddr*)&addr, sizeof(addr)))
	{
		cerr << "bind error" << endl;
	}

	listen(lfd, 128);

	cout << "=======  server init end  ======= " << endl;
	cout << "\n" << endl;
}

void eventset(struct myevent_s* ev, int fd, void(*call_back)(void*), void* arg)
{
	ev->fd = fd;
	ev->events = 0;
	ev->arg = arg;
	ev->call_back = (void(*)(void* arg))call_back;

	ev->status = 0;
	bzero(ev->buf, sizeof(ev->buf));
	ev->len = 0;
	//��������ò���
	ev->last_active_time = time(NULL);
}

/*�� epoll�����ĺ���� ���һ�� �ļ�������*/
void eventadd(int efd, int events, struct myevent_s* ev)
{
	int op;
	if (ev->status == 1)
	{
		op = EPOLL_CTL_MOD;
	}
	else
	{
		op = EPOLL_CTL_ADD;
		ev->status = 1;
	}

	struct epoll_event epv = { 0, {0} };

	epv.data.ptr = ev;
	epv.events = ev->events = events;

	if (epoll_ctl(efd, op, ev->fd, &epv) < 0)
	{
		cout << "epoll_ctl error" << endl;
		//error
	}
}

/*�� epoll�����ĺ���� ɾ��һ�� �ļ�������*/
void eventdel(int efd, struct myevent_s* ev)
{
	if (ev->status != 1)
	{
		cout << "return del" << endl;
		return;
	}

	struct epoll_event epv = { 0, {0} };

	epv.data.ptr = ev;
	ev->status = 0;

	if (epoll_ctl(efd, EPOLL_CTL_DEL, ev->fd, &epv) < 0)
	{
		cout << "EPOLL_CTL_DEL error" << endl;
		//error
	}
}

//����˻ص�����
void accept_conn(void* arg)
{
	cout << "==== have connect ..." << endl;

	struct myevent_s* ev = (struct myevent_s*)arg;
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);

	int i = 0;

	int client_fd = accept(ev->fd, (struct sockaddr*) & client_addr, &len);
	if (client_fd < 0)
	{
		if (errno != EAGAIN && errno != EINTR)
		{
			cerr << "client connect error" << endl;
			/*����*/
		}
		return;
	}

	do
	{
		for (i = 0; i < MAX_EVENTS; i++)
		{
			cout << "client pos: " << i << endl;
			if (g_events[i].status == 0)
			{
				break;
			}
		}
		if (i == MAX_EVENTS)
		{
			cerr << "max connect " << endl;//
			break;
		}

		fcntl(client_fd, F_SETFL, O_NONBLOCK);

		eventset(&g_events[i], client_fd, recvdata, &g_events[i]);
		eventadd(g_efd, (EPOLLIN | EPOLLET), &g_events[i]);

	} while (0);

	cout << "====  connect success  ====" << endl;
	cout << "\n" << endl;
}

//�ͻ��˶��ص�����
void recvdata(void* arg)
{
	cout << "==== recv ..." << endl;
	struct myevent_s* ev = (struct myevent_s*)arg;

	while (true)
	{
		int len = recv(ev->fd, ev->buf, sizeof(ev->buf), 0);

		if (len > 0)
		{
			ev->len += len;
		}
		else if (len == 0)
		{
			cout << "==== client fd: " << ev->fd << " close" << endl;

			eventdel(g_efd, ev);
			close(ev->fd);
			return;
		}
		//���ڷ�����read
		else
		{
			//��ʾ�жϣ�����
			if (errno == EINTR)
			{
				cerr << "read EINTR" << endl;
				continue;
			}
			//û�������Ƚ����Ժ���˵
			if (errno == EAGAIN)
			{
				cerr << "waiting..." << endl;
				break;
			}
		}
	}
	ev->buf[ev->len] = '\0';
	eventdel(g_efd, ev);

	string read_data = ev->buf;

	cout << read_data << endl;
	eventset(ev, ev->fd, senddata, ev);
	eventadd(g_efd, (EPOLLOUT | EPOLLET), ev);

	cout << "====  recv success  ====" << endl;
}

//�ͻ���д�ص�����
void senddata(void* arg)
{
	cout << "==== send ..." << endl;
	struct myevent_s* ev = (struct myevent_s*)arg;

	string buf = "this server";
	
	int flag = send(ev->fd, buf.c_str(), buf.size(), 0);

	if (flag > 0)
	{
		eventdel(g_efd, ev);
		eventset(ev, ev->fd, recvdata, ev);
		eventadd(g_efd, (EPOLLIN | EPOLLET), ev);

		cout << "====  send success  ====" << endl;
	}
	else
	{
		cout << "client fd: " << ev->fd << "send error" << endl;

		eventdel(g_efd, ev);
		close(ev->fd);
	}
}
