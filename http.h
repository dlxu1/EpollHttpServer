#ifndef __SERVER_H__
#define __SERVER_H__
#include<stdio.h>
#include<sys/stat.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<pthread.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<fcntl.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<sys/sendfile.h>
#include<sys/types.h>
#define MAX 1024
#define SIZE 256

void error_handing(int sock , int errcode);
int get_line(int sock,char* buf, int len);
int clear_header(int sock);
void show_homepage(int sock,const char* path ,size_t filesize);
void exec_handing(int sock,char* path, char* method,char* query_string ,size_t content_len);

#endif //__SERVER_H__
