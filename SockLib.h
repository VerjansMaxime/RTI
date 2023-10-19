#ifndef SOCKLIB_H
#define SOCKLIB_H

struct OVESP
{
    char request[20];
    char data1[50];
    char data2[50];
    char data3[50];
    char data4[50];
    char data5[200];
};


int SocketServer();
int AcceptConnection(int sock, char  * ipClient);

int SocketClient();

int Receive(int hsock, char *msg);
int Send(int hsock, char *msg);

void putEnd(char * msg);

struct OVESP ParseOVESP(char * msg);

void DisplayOVESP(struct OVESP ovesp);

#endif