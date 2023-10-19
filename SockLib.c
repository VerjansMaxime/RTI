#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "SockLib.h"

#include <fcntl.h>

int SocketServer()
{
    int port;
    char ip[20];
    struct sockaddr_in addrSock;

    //1.1)Get the Server port + IP from the config file
    const char *nomFichier = "config.conf";
    int fd = open(nomFichier, O_RDONLY);
    if (fd == -1) {
        perror("Erreur lors de l'ouverture du fichier");
        return -1;
    }

    char buffer[300];
    int bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) 
    {
        char *portPtr = strstr(buffer, "PORT_OVESP");
        if (portPtr != NULL) {
                
            sscanf(portPtr, "PORT_OVESP = %d", &port);
            printf("Port found : %d\n", port);
                
        }

        char *ipPtr = strstr(buffer, "IP_SERVER");
        if (ipPtr != NULL) {
                
            sscanf(ipPtr, "IP_SERVER = %s", &ip);
            printf("IP found : %s\n", ip);   
        }
        
    }

    int hsock;
    struct sockaddr_in my_addr;
    struct hostent * hst;

    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1)
    {
        printf("Error Socket()\n");
        exit(1);
    }
    else printf("Socket ok\n");

    if((hst = gethostbyname(ip)) == NULL)
    {
        printf("Error gethostbyname()\n");
        exit(1);
    }
    else printf("gethostbyname ok\n");

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(port);
    memcpy(&my_addr.sin_addr, hst->h_addr, hst->h_length);

    if(bind(hsock, (struct sockaddr *)&my_addr, sizeof(my_addr)) == -1)
    {
        printf("Error bind()\n");
        exit(1);
    }
    else printf("bind ok\n");

    printf("Socket created for %s on port: %d\n", ip, port);

        //listen
    if(listen(hsock, 5) == -1)
    {
        printf("Error listen()\n");
        exit(1);
    }
    else printf("Listen ok\n");

    return hsock;

}


int AcceptConnection(int sock, char  * ipClient)
{
    int hsockService;
    socklen_t tailleSockAddrin= sizeof(struct sockaddr_in);
    struct sockaddr_in addrSockClient;


    //accept
    if((hsockService=accept(sock, (struct sockaddr*)&addrSockClient, &tailleSockAddrin) )== -1)
    {
        printf("Error accept()\n");
        exit(1);
    }
    else printf("Accept ok hsockService = %d\n", hsockService);

    ipClient =inet_ntoa(addrSockClient.sin_addr);

    printf("Connection from %s\n", ipClient);

    return hsockService;
}


int SocketClient()
{ 
    int port;
    char ip[20];

    //1.1)Get the Server port + IP from the config file
    const char *nomFichier = "config.conf";
    int fd = open(nomFichier, O_RDONLY);
    if (fd == -1) {
        perror("Erreur lors de l'ouverture du fichier");
        return -1;
    }

    char buffer[300];
    int bytesRead;

    while ((bytesRead = read(fd, buffer, sizeof(buffer))) > 0) 
    {
        char *portPtr = strstr(buffer, "PORT_OVESP");
        if (portPtr != NULL) {
                
            sscanf(portPtr, "PORT_OVESP = %d", &port);
            printf("Port found : %d\n", port);
                
        }

        char *ipPtr = strstr(buffer, "IP_SERVER");
        if (ipPtr != NULL) {
                
            sscanf(ipPtr, "IP_SERVER = %s", &ip);
            printf("IP found : %s\n", ip);   
        }
        
    }

    int hsock;
    struct hostent * infoHost;
    struct sockaddr_in addrSock;

    //Creation de la socket
    hsock = socket(AF_INET, SOCK_STREAM, 0);
    if(hsock == -1)
    {
        printf("Error Socket()\n");
        exit(1);
    }
    else printf("Socket ok\n");

    //Recuperation data machine
    if((infoHost = gethostbyname(ip)) == NULL)
    {
        printf("Error gethostbyname()\n");
        exit(1);
    }
    else printf("gethostbyname ok\n");

    printf("Machine name is: %s\n", infoHost->h_name);
    printf("IP address: %s\n", inet_ntoa(*(struct in_addr*)infoHost->h_addr));

    //Preparation au connect
    memset(&addrSock, 0, sizeof(struct sockaddr_in));
    addrSock.sin_family = AF_INET;
    addrSock.sin_port = htons(port);
    memcpy(&addrSock.sin_addr, infoHost->h_addr, infoHost->h_length);

    if(connect(hsock, (struct sockaddr*)&addrSock, sizeof(struct sockaddr_in)) == -1)
    {
        printf("Error connect()\n");
        exit(1);
    }
    else printf("Connect ok\n");

    printf("Connection to %s on port: %d\n", ip, port);

    return hsock;
}



int Send(int hsock, char *msg)
{
   
    int retour;
    //printf("Valeur de taille: %d\n", taille);

    int taille = strlen(msg);

    if((retour=send(hsock,msg,taille,0))==-1)
    {
        printf("Erreur send %d\n",errno);
        exit(1);
    }
    printf("Send OK, MSG : %s Retour= %d, Taille: %d\n", msg, retour, taille);

    return retour;
}

int Receive(int hsock, char *msg)
{
    int retour;
    char tempo[500]="\0";
    char temp[2];
    printf("Receive...\n");
    int i=0;
    while(1)
    {
        if((retour=recv(hsock,temp,1,0))==-1)
        {
            printf("Erreur recv %d\n",errno);
            exit(1);
        }
        if(temp[0]=='~')
        {
            //strcat(tempo,"\0");
            break;
        }
        if(retour==0)
        {
            printf("Client parti %d\n",errno);
            return 0;
        }
        else
        {

            tempo[i]=temp[0];
        }
        i++;
    }
    strcpy(msg,tempo);
    printf("Receive OK, MSG : %s Retour= %d\n", msg, retour);

    return retour;
}


void putEnd(char * msg)
{
    fgets(msg, 500, stdin);
    msg[strlen(msg)-1] = '~';
    fflush(stdin);
}

struct OVESP ParseOVESP(char * msg)
{
    struct OVESP ovesp;
    char * token;
    char * saveptr;

    int i=0;
    for(token = strtok_r(msg, "$", &saveptr); token != NULL; token = strtok_r(NULL, "$", &saveptr))
    {
        switch(i)
        {
            case 0:
                strcpy(ovesp.request, token);
                break;
            case 1:
                strcpy(ovesp.data1, token);
                break;
            case 2:
                strcpy(ovesp.data2, token);
                break;
            case 3:
                strcpy(ovesp.data3, token);
                break;
            case 4:
                strcpy(ovesp.data4, token);
                break;
            case 5:
                strcpy(ovesp.data5, token);
                break;
            default:
                break;
        }
        i++;
    }
    return ovesp;
}

void DisplayOVESP(struct OVESP ovesp)
{
    printf("<---OVESP--->\n");
    printf("Request: %s\n", ovesp.request);
    printf("Data1: %s\n", ovesp.data1);
    printf("Data2: %s\n", ovesp.data2);
    printf("Data3: %s\n", ovesp.data3);
    printf("Data4: %s\n", ovesp.data4);
    printf("Data5: %s\n", ovesp.data5);
    printf("<----------->\n");
}

