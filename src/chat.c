/*

 * chat.c
 *
 *  Created on: 2012-8-5
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include "chat.h"

Node *head = NULL;

int init() { //带头节点的循环双链表
	head = malloc(sizeof(Node));
	head->prev = head;
	head->next = head;
	return 0;
}

int insert(Client *pc) { //头插法
	Node *p = malloc(sizeof(Node));
	p->c = *pc;

	/*p->next = head->next;
	 head->next->prev = p;
	 p->prev = head;
	 head->next = p;*/

	p->prev = head->prev;
	p->next = head;
	head->prev->next = p;
	head->prev = p;
	return 0;
}

int erase(int sockfd) {
	/*
	Node* p = head->next;
		while(p!=head){
			if(p->c.sockfd == sockfd){
				p->prev->next = p->next;
				p->next->prev = p->prev;
				free(p);
				break;
			}
			p = p->next;
		}
	 */

	 Node *q = head;
	 Node *p = head->next;
	 while (p != head) {
	 if (p->c.sockfd == sockfd) {
	 q->next = q->next->next;
	 q->next->prev = p;
	 free(p);
	 break;
	 }
	 p = p->next;
	 q = q->next;
	 }
	return 0;
}

int size() {
	Node *p = head->next;
	int cnt = 0;
	while (p != head) {
		++cnt;
		p = p->next;
	}
	return cnt;
}

void start() {
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == sockfd) {
		perror("socket!");
		exit(1);
	}

	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));

	struct sockaddr_in ser_addr;
	ser_addr.sin_family = AF_INET;
	ser_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	ser_addr.sin_port = htons(8888);
	memset(ser_addr.sin_zero, '\0', sizeof(ser_addr.sin_zero));

	int ret = bind(sockfd, (struct sockaddr *) &ser_addr,
			sizeof(struct sockaddr));
	if (-1 == ret) {
		perror("bind!");
		exit(1);
	}

	ret = listen(sockfd, 10);
	if (-1 == ret) {
		perror("listen!");
		exit(1);
	}
	printf("my chat server is ready!\n");

	while (1) {
		int clientfd = accept(sockfd, NULL, NULL);
		if (-1 == clientfd) {
			perror("accept!");
			continue;
		}

		pthread_t tid;
		ret = pthread_create(&tid, NULL, (void *) process, &clientfd);
		if (!ret) {
			printf("pthread_create %u is create successful!\n",
					(unsigned int) pthread_self());
		} else {
			printf("pthread_create error!\n");
			exit(1);
		}
	}
	return;
}

void process(void* p) {
	int clientfd = *(int *) p;

	struct sockaddr_in c_addr;
	unsigned int c_len = sizeof(struct sockaddr);
	getpeername(clientfd, (struct sockaddr *) &c_addr, &c_len);
	printf("A new client is connected:ip=%s:port=%d\n", inet_ntoa(
			c_addr.sin_addr), ntohs(c_addr.sin_port));

	char welcome[128] = "welcome to my chat server!\n";
	send(clientfd, welcome, strlen(welcome), 0);

	char login_msg[128] = "please input your nick name>";
	send(clientfd, login_msg, strlen(login_msg), 0);

	char name[20] = { '\0' };
	recv(clientfd, name, sizeof(name), 0);
	char *str = strchr(name, '\r');
	if (str)
		*str = '\0';
	printf("name = %s\n", name);

	char online_msg[128] = {'\0'};  //"Client  is online!\r\n";//
	sprintf(online_msg, "Client [%s] is online!\n", name);
	send_all(online_msg);

	Client c;
	strcpy(c.name, name);
	c.sockfd = clientfd;
	insert(&c);
	printf("All the login number is :%d\n", size());

	while (1) {
		char buf[128] = { '\0' };
		int n = recv(clientfd, buf, sizeof(buf), 0);
		printf("n:%d,buf:%s\n", n, buf);
		if (n <= 2) {
			char offline_msg[128] = { '\0' };
			sprintf(offline_msg, "client [%s] was offline!\n", name);
			send_all(offline_msg);
			break;
		}
		char all_buf[256] = { '\0' };
		char time2[32] = { '\0' };
		char time[32] = { '\0' };
		getTime(time2);
		strncpy(time,time2,strlen(time2)-2);
		sprintf(all_buf, "%s [%s]:%s", c.name, time, buf);
		send_all(all_buf);
	}
	printf("A new client is disconnected:ip=%s:port=%d\n", inet_ntoa(
			c_addr.sin_addr), ntohs(c_addr.sin_port));

	erase(clientfd);
	close(clientfd);
}

void send_all(char* msg) {
	Node* p = head->next;
	while (p != head) {
		send(p->c.sockfd, msg, strlen(msg), 0);
		p = p->next;
	}
	return ;
}

void getTime(char *str) {
	long t = time(NULL);
	struct tm* tm1 = localtime(&t);
	char buf[128];
	strftime(buf, sizeof(buf), "%H:%M:%S\n", tm1);
	strcpy(str, buf);
	return;
}

