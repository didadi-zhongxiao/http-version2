#include "ThreadWork.h"
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include "RequestParse.h"
#include "RequestHandler.h"
#include<string>
#include"iostream"
extern pthread_mutex_t mutx;
extern pthread_cond_t clientfd_cond;

ThreadWork::ThreadWork(int num) :nThreads(num)
{
	for (int i{}; i < nThreads; i++)
		threadsMake(i);
}
void ThreadWork::threadsMake(int i)
{
	pthread_t pid;
	pthread_create(&pid, NULL, &threadMain,this);
	return;
}
void *ThreadWork::threadMain(void *arg)
{
//	pthread_t pid = (pthread_t)arg;
	ThreadWork * twork=(ThreadWork*)arg;
	int fd=0;
	for (;;)
	{
			pthread_mutex_lock(&mutx);
			if(twork->clientSockets.empty())
					pthread_cond_wait(&clientfd_cond,&mutx);
			if(!(twork->clientSockets.empty()))
			{
	//		std::cout<<"not empty"<<std::endl;
			fd = twork->clientSockets.at(0);
			twork->clientSockets.pop_front();
		//	work(fd);
			}
			pthread_mutex_unlock(&mutx);
			work(fd);
			if(fd>0)
				close(fd);
		//	if(fd!=0)
		//		work(fd);
//		}
//		pthread_mutex_unlock(&mutx);
	}
}
void ThreadWork::work(int confd)
{
	//非阻塞套接字，必须保证一次接收完全
//		std::cout<<"workfd"<<std::endl;
	if(confd==0)
		return ;
	char buf[65535] = { 0 };
	int strlen = read(confd, buf, 65535);
	if (strlen <= 0)
	{
//		std::cout<<"pid: "<<pid<<" strlen:"<<strlen<<std::endl;
		return ;
	}
	else
	{
		buf[strlen] = '\0';
		RequestParse requestparse(buf);
		HttpRequest hRequest;
		reply rep;
//		std::cout<<"read buf:"<<buf<<std::endl;
		if (!requestparse.parse(hRequest))
		{
			rep = reply::stock_reply(reply::bad_request);
	//		std::cout<<strlen<<"	badrequest\n";
		}
		else
		{
	//		std::cout<<"parse finished\n";
			RequestHandler reqHandler("/root/test/http/http1");
			reqHandler.handlerRequest(hRequest, rep);
		}
		std::string str=rep.toBuffer();
//		std::string buf;
		std::string buf=str+rep.headers[0].name+": "+
				rep.headers[0].value+"\r\n"+rep.headers[1].name+": "+
				rep.headers[1].value+"\r\n\r\n"+rep.content;
		int writelen=buf.size();
		int len=0;
		while (len < writelen)
		{
			len = write(confd, buf.c_str(), writelen);
		}
//		std::cout<<"writelen:"<<writelen<<"len :"<<len<<"\n";
//		std::cout<<"buf:"<<buf<<std::endl;
	//	close(confd);
	}
}
void ThreadWork::addDeque(int fd)
{
	clientSockets.push_back(fd);
}
