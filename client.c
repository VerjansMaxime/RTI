#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include "SockLib.h"



void DispMenu();
void TestLogin(int sock);

//Func
void TestConsult(int sock);
void TestBuy(int sock);
void TestCart(int sock);
void TestCancel(int sock);
void TestCancelAll(int sock);
void TestLogout(int sock);
void TestConfirm(int sock);


int main()
{
    int hsock;
    hsock = SocketClient("192.168.1.15", 50018);
    int choice;
    int end =0;

    do
    {
        
        DispMenu();
        scanf("%d", &choice);
        fflush(stdin);

        switch(choice)
        {
            case 1:
                TestLogin(hsock);
                break;
            case 2:
                TestConsult(hsock);
                break;
            case 3:
                TestBuy(hsock);
                break;
            case 4:
                TestCart(hsock);
                break;
            case 5:
                TestCancel(hsock);
                break;
            case 6:
                TestCancelAll(hsock);
                break;
            case 7:
                TestLogout(hsock);
                break;
            case 8:
                Send(hsock, "EXIT$~");
                end = 1;
                break;
            case 9:
                TestConfirm(hsock);
                break;
            default:
                printf("Invalid choice\n");
                break;
        }



        /*
        char buffer[200];
        int ret;

        printf("Enter message: ");
        putEnd(buffer);

        if(strcmp(buffer, "quit~") == 0)
        {
            printf("End of client\n");
            break;
        }

        Send(hsock, buffer);

        ret = Receive(hsock, buffer);

        if(ret == 0)
        {
            printf("Server disconnected\n");
            break;
        }
        else if(ret == -1)
        {
            printf("Error Receive()\n");
            break;
        }
        else
        {
            printf("Received: %s\n", buffer);
        }

        */
    }while(end ==0);

    close(hsock);
    return  0;
}


void DispMenu()
{
    printf("1. Login\n");
    printf("2. Consult\n");
    printf("3. Buy\n");
    printf("4. Cart\n");
    printf("5. Cancel\n");
    printf("6. Cancel all\n");
    printf("7. Logout\n");
    printf("9. Confirm\n");
    printf("8. Quit\n");

    printf("Enter choice: ");
}

void TestLogin(int sock)
{
    char buffer[200] = "LOGIN$Pierre$Debois$1$~";
    Send(sock, buffer);
    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);

}

void TestConsult(int sock)
{
    char buffer[200] = "CONSULT$25$~";
    Send(sock, buffer);

    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);

    

}

void TestBuy(int sock)
{
    char buffer[200] = "BUY$3$1$~";
    Send(sock, buffer);

    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);


}

void TestCart(int sock)
{
    struct OVESP ovesp;
    int nbrArt = 0;
    char buffer[200] = "CART$~";
    Send(sock, buffer);

    //Pour chaque elem du panier, 1 msg a recevoir
    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);

    ovesp = ParseOVESP(buffer2);

    nbrArt = atoi(ovesp.data1);

    for(int i = 0; i < nbrArt; i++)
    {
        char buffer3[500];
        ret = Receive(sock, buffer3);
        printf("Received: %s\n", buffer3);
    }


}

void TestCancel(int sock)
{
    char buffer[200] = "CANCEL$3$~";
    Send(sock, buffer);

    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);

    strcpy (buffer, "CANCEL$5$~");
    Send(sock, buffer);

    char buffer3[200];
    ret = Receive(sock, buffer3);
    printf("Received: %s\n", buffer3);

}

void TestCancelAll(int sock)
{
    char buffer[200] = "CANCEL ALL$~";
    Send(sock, buffer);

    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);

}

void TestLogout(int sock)
{
    char buffer[200] = "LOGOUT$~";
    Send(sock, buffer);

    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);

}

void TestConfirm(int sock)
{
    char buffer[200] = "CONFIRM$1$~";
    Send(sock, buffer);

    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);

}

