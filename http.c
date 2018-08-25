#include"http.h"
void error_handing(int sock , int errcode)
{
	char line[SIZE*10];
	char content_type[64]={0};
	char content_length[32]={0};
	sprintf(line, "HTTP/1.0 %d  OK\r\n",errcode);
	send(sock, line, strlen(line), 0);
	sprintf(content_type,"Content-Type: text/html;charset=UTF-8\r\n");
	send(sock, content_type, strlen(content_type), 0);
	sprintf(content_length,"Content-Length: %d\r\n",37);
	send(sock,content_length,strlen(content_length),0);
	send(sock, "\r\n", 2, 0);
	char str[128]="<h1 style=text-align:center>";
	int fd;
	switch(errcode)
	{
		case 404:
			{
			 char* errstr="404 </h1>";
			 strcat(str,errstr);
			int err= write(sock,str,strlen(str)+1);
			 printf("http:19 %d\n",err);
			}
			break;
		case 500: 
			{
			 char* errstr="500 </h1>";
			 strcat(str,errstr);
			 send(sock,str,strlen(str),0);
			}
			break;
		case 503:
			{
			 char* errstr="503 </h1>";
			 strcat(str,errstr);
			 send(sock,str,strlen(str),0);
			}
			break;
		default:
			break;
	}
	close(sock);
}
int get_line(int sock,char* buf, int len) //获取报文中一行并统一将/r/n /r 转换为/n 
{
	char ch='a';
	int i=0;
	while(ch!='\n' && i<len-1)
	{
		if(recv(sock,&ch,1,0)>0)
		{
			if(ch=='\r')
			{
				if(recv(sock,&ch,1,MSG_PEEK)>0 && ch== '\n')
					recv(sock,&ch,1,0);
				else
					buf[i]='\n';
			}
			buf[i++]=ch;
		}
	}
	buf[i]='\0';
	return i;
}
int clear_header(int sock)
{
	int errcode=0;
	char line[SIZE];
	int ret = -1;
	do{
		ret = get_line(sock, line, SIZE);
	}while(ret != 1  &&  strcmp(line,"\n")!=0);

	return ret;
}
void show_homepage(int sock,const char* path, size_t filesize)
{
	int errcode=0;
	int ret = -1;
	int fd = open(path, O_RDONLY);
	if(fd < 0)
	{
		errcode=503;
		error_handing(sock,errcode);
	}

	char content_type[1024]={0};
	char line[SIZE*10];
	sprintf(line, "HTTP/1.0 200 OK\r\n");
	send(sock, line, strlen(line), 0);
	const char* p=path+strlen(path)-1;
	while(*p!='.') p--;
	if(strcmp(p,".css")==0)
		sprintf(content_type,"Content-Type: text/css\r\n");
	if(strcmp(p,".js")==0)
		sprintf(content_type,"Content-Type: application/x-javascript\r\n");
	else
		sprintf(content_type,"Content-Type: text/html\r\n");
	send(sock, content_type,strlen(content_type),0);
	send(sock, "\r\n", 2, 0);
	if(sendfile(sock, fd, NULL,filesize) < 0)
	{
		errcode=503;
		error_handing(sock,errcode);
	}
	close(fd);
}
void exec_handing(int sock,char* path, char* method,char* query_string ,size_t content_len)
{
	pid_t pid;
	int input[2];
	int output[2];
	pipe(input);
	pipe(output);
	char env_method[32];
	char env_path[64];
	char content_type[1024]={0};
	const char* p=path+strlen(path)-1;
	while(*p!='.') p--;
	if(strcmp(p,".css")==0)
		sprintf(content_type,"Content-Type: text/css\r\n");
	if(strcmp(p,".js")==0)
		sprintf(content_type,"Content-Type: application/x-javascript\r\n");
	else
		sprintf(content_type,"Content-Type: text/html;charset=UTF-8\r\n");
	printf("%s\n",content_type);
	pid=fork();
	if(pid<0) perror("fork"),exit(1);
	else if(pid==0)
	{
		close(input[1]);
		close(output[0]);

		char env_method[32]={0};
		char env_path[64]={0};
		char env_query_string[64]={0};
		char env_content_len[8]={0};
		dup2(input[0],0);
		dup2(output[1],1);

		sprintf(env_method,"math=%s",method);
		sprintf(env_path,"path=%s",path);
		sprintf(env_query_string,"query_string=%s",query_string);
		sprintf(env_content_len,"content_len=%d",content_len);

		putenv(env_path);
		putenv(env_method);
		putenv(env_query_string);
		putenv(env_content_len);
		execl(path,path,NULL);
		exit(0);
	}
	else
	{
		close(output[1]);
		close(input[0]);
		char ch;
		char line[SIZE*10];
		sprintf(line, "HTTP/1.0 200 OK\r\n");
		send(sock, line, strlen(line), 0);
		send(sock, content_type, strlen(content_type), 0);
		send(sock,"\r\n", 2, 0);
		while(read(output[0],&ch,1)>0)
		{
			send(sock,&ch,1,0);
		}
		close(output[0]);
		close(input[1]);
	}
}
