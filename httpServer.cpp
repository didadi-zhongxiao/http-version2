#include<iostream>
#include<cstring>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>

#include <fcntl.h>
#include <sys/epoll.h>

#include"ThreadWork.h"
using namespace std;
pthread_mutex_t mutx;
pthread_cond_t  clientfd_cond;
int server_socket;
//mutx=PTHREAD_MUTEX_INITIALIZER;

// 将描述符fd设置为非阻塞
void setnonblocking(const int fd)
{
	int old_options = fcntl(fd, F_GETFL);
	fcntl(fd, F_SETFL, old_options | O_NONBLOCK);
}
#define  MAX_SIZE 10000
int main(int argc,char *argv[])
{
	pthread_mutex_init(&mutx,NULL);
	pthread_cond_init(&clientfd_cond,NULL);
	if(argc!=2)
	{
		cout<<"usage"<<argv[0]<<"<IP> <port>"<<endl;
		exit(1);
	}
	server_socket=socket(PF_INET,SOCK_STREAM,0);
	if(server_socket==-1)
		cout<<"socket error"<< endl;
	struct sockaddr_in serv_addr;
	struct sockaddr_in clint_addr;
	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family=AF_INET;
	serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
	serv_addr.sin_port=htons(atoi(argv[1]));
	if(bind(server_socket,(struct sockaddr*)&serv_addr,sizeof(serv_addr))
					== -1)
		cout<<"bind error"<<endl;
	if(listen(server_socket,20)== -1)
		cout<<"listen error"<<endl;
	struct epoll_event ep_events[MAX_SIZE];
	int epfd = epoll_create(1);
//	std::vector<struct epoll_event> ep_events;
	struct epoll_event event;
	event.events = EPOLLIN;
	event.data.fd = server_socket;
	epoll_ctl(epfd, EPOLL_CTL_ADD, server_socket, &event);
	int event_cnt,client_socket;
	socklen_t adr_sz;
	adr_sz = sizeof(clint_addr);
	setnonblocking(server_socket);
	ThreadWork threadWork(6);
	char  buf[65535] = {0};
	int bufLen;
	while (1)
	{
		event_cnt = epoll_wait(epfd, ep_events,MAX_SIZE, -1);
		if (event_cnt == -1)
		{
			std::cout << "epoll_wait error" << std::endl;
			break;
		}
		for (int i{}; i < event_cnt; i++)
		{
			//有新的连接到来
			if (ep_events[i].data.fd == server_socket)
			{
				client_socket = accept(server_socket,
						(struct sockaddr*)&clint_addr, &adr_sz);
				setnonblocking(ep_events[i].data.fd);
				event.events = EPOLLIN|EPOLLET;
				event.data.fd = client_socket;
				epoll_ctl(epfd, EPOLL_CTL_ADD, client_socket, &event);
		//		cout<<"connection reached"<<endl;
			}
			else if(ep_events[i].events & EPOLLIN)
			{
		//		cout<<"request reached"<<endl;
				threadWork.addDeque(ep_events[i].data.fd);
				pthread_cond_signal(&clientfd_cond);
			}
			else if (ep_events[i].events & (EPOLLRDHUP | EPOLLERR))
			{
				close(ep_events[i].data.fd);
			}
		}
	}

	for(;;)
		pause();
	return 0;

}


