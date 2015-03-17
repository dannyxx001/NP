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

using namespace std;

int main(int argc, char *argv[])
{
	int sockfd,n;   //read n char
	struct sockaddr_in servaddr;
	char send_buff[MAXLINE],recv_buff[MAXLINE+1];

	//three arguments
	if(argc != 3)
	{
		cout << "usage: ./client <Server IP> <Server PORT>" << endl;
		exit(-1);
	}

	//prepare socket
	if((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0)
	{
		cout << "socket error" << endl;
		exit(-1);
	}

	//set server address
	bzero(&servaddr,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));
	if(inet_pton(AF_INET,argv[1],&servaddr.sin_addr) <= 0)
	{
		cout << "inet_pton error for " << argv[1] << endl;
		exit(-1);
	}

	//connect server
	if(connect(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)) < 0)
	{
		cout << "connect error" << endl;
		exit(-1);
	}

	//select stdin and sockfd
	int maxfdp;
	bool stdin_eof = false;
	fd_set rset;
	FD_ZERO(&rset);
	while(true)
	{
		if(stdin_eof == 0)
			FD_SET(fileno(stdin),&rset);
		FD_SET(sockfd,&rset);
		maxfdp = max(fileno(stdin),sockfd)+1;
		select(maxfdp,&rset,NULL,NULL,NULL);
		//stdin
		if(FD_ISSET(fileno(stdin),&rset))
		{
			//input EOF
			if(fgets(send_buff,sizeof(send_buff),stdin) == NULL)
			{
				stdin_eof = true;
				shutdown(sockfd,SHUT_WR);
				FD_CLR(fileno(stdin),&rset);
				continue;
			}
			for(int i=0;i<strlen(send_buff)&&send_buff[i]!=' ';i++)
				send_buff[i] = tolower(send_buff[i]);
			//exit close by client
			char tmp[MAXLINE];
			strcpy(tmp,send_buff);
			char *ptr = strtok(tmp," \n");
			if(strcmp(ptr,"exit") == 0)
			{
				stdin_eof = true;
				shutdown(sockfd,SHUT_WR);
				FD_CLR(fileno(stdin),&rset);
				continue;
			}
			write(sockfd,send_buff,strlen(send_buff));
		}
		//sockfd
		if(FD_ISSET(sockfd,&rset))
		{
			if((n = read(sockfd,recv_buff,MAXLINE)) > 0)
			{
				recv_buff[n] = 0;
				if(fputs(recv_buff,stdout) == EOF)
					cout << "fputs error" << endl;
			}
			else if(n == 0)
			{
				if(stdin_eof == true)   //input EOF
					return 0;
				else                    //server close connection
				{
					cout << "server terminated prematurely" << endl;
					exit(0);
				}
			}
		}
	}
	exit(0);

}
