/*
 * chat.h
 *
 *  Created on: 2012-8-5
 *      Author: root
 */

#ifndef CHAT_H_
#define CHAT_H_

typedef struct client {
	int sockfd;
	char name[20];
} Client;

typedef struct node {
	Client c;
	struct node *prev;
	struct node *next;
} Node;



int init();
int insert(Client *pc);
int erase(int sockfd);
int size();

void send_all(char* msg);
void process(void* p);
void start();

void getTime(char *time);

#endif /* CHAT_H_ */

