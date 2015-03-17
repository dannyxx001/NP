#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <string>
#define MAXLINE 1024
#define LISTENNQ 10

using namespace std;

int main(int argc, char *argv[])
{
	int listenfd,n;
	struct sockaddr_in servaddr,cliaddr;
	char send_buff[MAXLINE],recv_buff[MAXLINE+1];	

	if(argc != 2)
	{
		cout << "usage: ./server.exe <Port>" << endl;
		exit(-1);
	}

	//set server address
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family =AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));

	//prepare socket
	if((listenfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		cout << "socket error" << endl;
		exit(-1);
	}

	//bind socket with addr
	if(bind(listenfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
	{
		cout << "bind error" << endl;
		exit(-1);
	}

	//listen
	if(listen(listenfd,LISTENNQ) < 0)
	{
		cout << "listen error" << endl;
		exit(-1);
	}

	//init
	int maxfd = listenfd;
	int max_clients = -1;
	int n_ready,j;
	int client[10];

	for(int i=0;i<10;i++)
		client[i] = -1;
	socklen_t clilen;

	fd_set rset,allset;
	FD_ZERO(&allset);
	FD_SET(listenfd,&allset);

	while(true)
	{
		rset = allset;
		n_ready = select(maxfd+1,&rset,NULL,NULL,NULL);
		
		//new client
		if(FD_ISSET(listenfd,&rset))
		{
			for(j=0;j<10;j++)
			{
				if(client[j] < 0)
				{
					clilen = sizeof(cliaddr);
					client[j] = accept(listenfd,(struct sockaddr *)&cliaddr,&clilen);
					FD_SET(client[j],&allset);
					if(client[j] > maxfd)
						maxfd = client[j];
					if(j > max_clients)
						max_clients = j;
					snprintf(send_buff,sizeof(send_buff),"[Server] Welcome!!!\n");
					write(client[j],send_buff,strlen(send_buff));
					break;
				}
			}
			if(j == 10)
				cout << "too many clients" << endl;
			if(--n_ready <= 0)
				continue;
		}
		//service all clients
		for(j=0;j<=max_clients;j++)
		{
			if(client[j] < 0)
				continue;
			if(FD_ISSET(client[j],&rset))
			{
				if((n = read(client[j],recv_buff,MAXLINE)) == 0) //client send EOF
				{
					close(client[j]);
					FD_CLR(client[j],&allset);
					client[j] = -1;
				}
				else
				{
					recv_buff[n] = 0;
					cout << recv_buff << endl;
					snprintf(send_buff,sizeof(send_buff),"[Server] Not first come!\n");
					write(client[j],send_buff,strlen(send_buff));
				}
			}
		}
	}
	return 0;
}
