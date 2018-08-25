#include<sys/epoll.h>
#include"http.h"

int get_listen_sock(int port)
{
	int sock=socket(AF_INET,SOCK_STREAM,0);
	if(sock<0) perror("sock:"),exit(-1);

	struct sockaddr_in server_addr;
	server_addr.sin_family=AF_INET;
	server_addr.sin_port=htons(port);
	server_addr.sin_addr.s_addr=htonl(INADDR_ANY);

	int opt=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

	socklen_t len=sizeof(server_addr);
	if(bind(sock,(struct sockaddr*)&server_addr,len)<0)
	{
		perror("bind:");
		exit(-1);
	}
	if(listen(sock,5)<0)
	{
		perror("listen:");
		exit(-1);
	}
	return sock;
}
void ProcessConnect(int listen_sock,int epoll_fd)
{
	struct sockaddr_in client_addr;
	socklen_t len = sizeof(client_addr);
	int client_sock = accept(listen_sock ,(struct sockaddr*)&client_addr,&len);	
	if(client_sock<0) perror("accept:"),exit(-1);

	struct epoll_event ev;
	ev.data.fd=client_sock;
	ev.events=EPOLLIN;

	int ret=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_sock,&ev);
	if(ret<0) perror("epoll_ctl add client_sock\n"),exit(-1);
}

//void* hand_request(void* argc)
void ProsessRequest(int sock,int epoll_fd)
{
//	int sock=(int)argc;
	char buf[MAX];
	char method[64];
	char path[64];
	char *query_string=NULL;
	int cgi=0;
	int content_len=-1;
	char query_str[64]={0};
	int errno;
	if(get_line(sock ,buf,MAX)>0)  //解析请求行，得到method，path ，query_string
	{
		int i=0;
		int j=0;
		while(i<sizeof(buf)-1 && j<sizeof(method)-1 && !isspace(buf[i]))
		{
			method[j++]=buf[i++];
		}
		method[j]='\0';
		i++;  j=0;
		while(i<sizeof(buf)-1 && j<sizeof(path)-1 && !isspace(buf[i]))
		{
			path[j++]=buf[i++];
		}
		path[j]='\0';

		if(strcasecmp("POST",method)==0) // POST方法需要获取请求头部的Content-Length
		{
			cgi=1;
			int ret=0;
			char buf[128];
			do{
				ret=get_line(sock,buf,sizeof(buf));
				if(ret>0 && strncasecmp(buf,"Content-Length: ",16)==0)
				{
					content_len=atoi(buf+16);
				}			
			}while(ret!=1 && strcmp(buf,"\n")!=0);
			if(content_len<0)
			{
				int	errno=500;
				error_handing(sock,errno);
			}
		}
		else if(strcasecmp("GET",method)==0)
		{
			int i=0;
			while(path[i]!='?' &&  path[i]!='\0')
				i++;
			if(path[i]=='?')
			{
				path[i]='\0';
				query_string=path+i+1;
				cgi=1;
				strcpy(query_str,query_string);
				query_string=query_str;  // 注意，path 里含有query的内容 ，这里直接用sprintf会有问题
			}
		}
		char tmp[64];
		strcpy(tmp,path);
		sprintf(path,"ServerRoot%s",tmp);
		if(path[strlen(path)-1] == '/')	
		{
			strcat(path,"index.html");
		}
		printf("%s\n",path);
	}
	struct stat st;
	if(stat(path, &st) < 0)
	{
		errno=404;
		error_handing(sock,errno);
		goto end;
	}
	else
	{
		//判断是不是一个可执行文件
		if(S_ISREG(st.st_mode))
		{
			if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
				cgi=1;
		}
		else if(S_ISDIR(st.st_mode))//是一个文件目录
		{
			strcat(path, "/index.html");
		}
		else
		{
			errno=404;
			error_handing(sock,errno);
			goto end;
		}
	}
	if(cgi==1)
	{
		clear_header(sock);
		exec_handing(sock,path,method,query_string,content_len);
		close(sock);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, NULL);
	}
	else
	{
		clear_header(sock);
		show_homepage(sock, path, st.st_size);
		close(sock);
		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, sock, NULL);
	}
end:
	;
}
int main(int argc , char* argv[])
{	
	if(argc!=2)
	{
		printf("usage: %s [port]!\n",argv[0]);
		exit(-1);
	}
	int listen_sock = get_listen_sock(atoi(argv[1]));

	int epoll_fd=epoll_create(10);
	if(epoll_fd<0) perror("epoll_create:"),exit(-1);
	
	struct epoll_event event;
	event.events=EPOLLIN;
	event.data.fd=listen_sock;
	int ret=epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_sock,&event);
	if(ret<0) perror("epoll_ctl:"),exit(-1);
	for(; ;)
	{
		struct epoll_event RetEvents[10];
		int size=epoll_wait(epoll_fd,RetEvents,sizeof(RetEvents)/sizeof(RetEvents[0]),-1);
		if(size<0)
		{
			perror("epoll_wait\n");
			continue;
		}
		if(size==0)
		{
			printf("epoll timeout\n");
			continue;
		}
		for(int i=0;i<size;i++)
		{
			if(!(RetEvents[i].events & EPOLLIN))
				continue;
			if(RetEvents[i].data.fd==listen_sock)   //如果监听套接字有写事件来
				ProcessConnect(listen_sock,epoll_fd);
			else
				ProsessRequest(RetEvents[i].data.fd,epoll_fd); //客户端套接字有写事件
		}
	}
	return 0;
}
