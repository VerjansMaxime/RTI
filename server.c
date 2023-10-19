#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 
#include "SockLib.h"
#include <mysql.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>


int dex = 0 ;

struct ARTICLE
{
    char id[10];
    char name[50];
    char price[10];
    char qtt[10];
    char img[25];
    int index ; 
};

//Case
void CaseLogin(int hsock, struct OVESP ovesp);
void CaseConsult(int hsock, struct OVESP ovesp);
struct ARTICLE CaseBuy(int hsock, struct OVESP ovesp);
void CaseCart(int hsock, struct ARTICLE * cart);
void CaseCancel(int hsock, struct OVESP ovesp, struct ARTICLE * article);
void CaseCancelAll(int hsock, struct OVESP ovesp, struct ARTICLE * article);
void CaseLogout(int hsock, struct OVESP ovesp);
void CaseConfirm(int hsock,struct OVESP ovesp, struct ARTICLE * article);

void* FctThreadClient(void* p);


#define NB_THREADS_POOL 2
#define TAILLE_FILE_ATTENTE 20
int socketsAcceptees[TAILLE_FILE_ATTENTE];
int indiceEcriture=0, indiceLecture=0;
pthread_mutex_t mutexSocketsAcceptees;
pthread_cond_t condSocketsAcceptees;

int PoolThreads = 2;


int main()
{
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
        char *portPtr = strstr(buffer, "MAX_THREADS");
        if (portPtr != NULL) {
                
            sscanf(portPtr, "MAX_THREADS = %d", &PoolThreads);
                
        }
    }




    pthread_mutex_init(&mutexSocketsAcceptees,NULL);
    pthread_cond_init(&condSocketsAcceptees,NULL);
    for (int i=0 ; i<TAILLE_FILE_ATTENTE ; i++)
        socketsAcceptees[i] = -1;

    // Creation du pool de threads
    printf("Création du pool de threads. %d\n", PoolThreads);
    pthread_t th;
    for (int i=0 ; i<PoolThreads ; i++)
        pthread_create(&th,NULL,FctThreadClient,NULL);



    






    int hsock, hsockService;
    char temp[100];
    hsock = SocketServer();

    while(1)
    {
        printf("Attente d'une connexion...\n");
        if ((hsockService = AcceptConnection(hsock, temp)) == -1)
        {
            perror("Erreur de Accept");
            exit(1);
        }
        printf("Connexion acceptée : IP=%s socket=%d\n",temp,hsockService);
        // Insertion en liste d'attente et réveil d'un thread du pool
        // (Production d'une tâche)
        pthread_mutex_lock(&mutexSocketsAcceptees);
        socketsAcceptees[indiceEcriture] = hsockService; // !!!
        indiceEcriture++;
        if (indiceEcriture == TAILLE_FILE_ATTENTE) indiceEcriture = 0;
        pthread_mutex_unlock(&mutexSocketsAcceptees);
        pthread_cond_signal(&condSocketsAcceptees);



    }



    
    

    printf("End of server\n");
    close(hsock);
    close(hsockService);
    
    return  0;
}

void* FctThreadClient(void* p)
{
    int sService;
    while(1)
    {
        printf("\t[THREAD %p] Attente socket...\n",pthread_self());
        // Attente d'une tâche
        pthread_mutex_lock(&mutexSocketsAcceptees);
        while (indiceEcriture == indiceLecture)
            pthread_cond_wait(&condSocketsAcceptees,&mutexSocketsAcceptees);
        sService = socketsAcceptees[indiceLecture];
        socketsAcceptees[indiceLecture] = -1;
        indiceLecture++;
        if (indiceLecture == TAILLE_FILE_ATTENTE) indiceLecture = 0;
        pthread_mutex_unlock(&mutexSocketsAcceptees);
        // Traitement de la connexion (consommation de la tâche)
        printf("\t[THREAD %p] Je m'occupe de la socket %d\n",
        pthread_self(),sService);
        TraitementConnexion(sService);
    }
}

void TraitementConnexion(int hsockService)
{
    struct ARTICLE cart[25];
    for(int i = 0; i < 25; i++)
    {
        strcpy(cart[i].id, "");
        cart[i].index=99;
    }

    printf("hsokService = %d\n", hsockService);
    do
    {
        printf("Waiting for request\n");
        char t[200];
        char buffer[200];
        int ret;
        struct OVESP ovesp;
        ret = Receive(hsockService, buffer);
        ovesp = ParseOVESP(buffer);
        DisplayOVESP(ovesp);

        if(strcmp(ovesp.request, "LOGIN")==0)
            CaseLogin(hsockService, ovesp);
        else if(strcmp(ovesp.request, "CONSULT")==0)
            CaseConsult(hsockService, ovesp);
        else if(strcmp(ovesp.request, "BUY")==0)
        {
            struct ARTICLE art;
            art = CaseBuy(hsockService, ovesp);

            
            for(int i = 0; i < 25; i++)
            {
                if(strcmp(cart[i].id, "") == 0)
                {
                    strcpy(cart[i].id, art.id);
                    strcpy(cart[i].name, art.name);
                    strcpy(cart[i].price, art.price);
                    strcpy(cart[i].qtt, art.qtt);
                    strcpy(cart[i].img, art.img);
                    cart[i].index= art.index ;
                    break;
                }
            }
        }
        else if(strcmp(ovesp.request, "CART")==0)
            CaseCart(hsockService, cart);
        else if(strcmp(ovesp.request, "CANCEL")==0)
            CaseCancel(hsockService, ovesp, cart);
        else if(strcmp(ovesp.request, "CANCELALL")==0)
            CaseCancelAll(hsockService, ovesp, cart);
        else if(strcmp(ovesp.request, "LOGOUT")==0)
            CaseLogout(hsockService, ovesp);
        else if(strcmp(ovesp.request, "EXIT")==0)
            break;
        else if(strcmp(ovesp.request, "CONFIRM")==0)
            CaseConfirm(hsockService,ovesp, cart);
        else
            printf("Invalid request\n");


    }while(1);
}




void CaseLogin(int hsock, struct OVESP ovesp)
{
    MYSQL* connexion = mysql_init(NULL);
    mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);
    int ok =0;

    if(strcmp(ovesp.data3, "1")==0)
    {
        //New user

        char query[200];

        sprintf(query, "INSERT INTO users (username, pwd) VALUES ('%s', '%s');", ovesp.data1, ovesp.data2);
        printf("Query: %s\n", query);
        mysql_query(connexion, query);

        ok = 1;

    }
    else
    {
        //Existing user

        char query[200];

        sprintf(query, "SELECT * FROM users WHERE username='%s' AND pwd='%s';", ovesp.data1, ovesp.data2);
        mysql_query(connexion, query);

        MYSQL_RES *result = mysql_store_result(connexion);

        if(mysql_num_rows(result) == 1)
            ok = 1;
        else
            ok = 0;
    }

    char buffer[200];
    if(ok == 1)
    {
        strcpy(buffer, "LOGIN$OK$~");
    }
    else
    {
        strcpy(buffer, "LOGIN$KO$~");
    }

    mysql_close(connexion);

    Send(hsock, buffer);
}

void CaseConsult(int hsock, struct OVESP ovesp)
{
    MYSQL* connexion = mysql_init(NULL);
    mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);

    char query[200];
    sprintf(query, "SELECT * FROM articles WHERE id='%s';", ovesp.data1);
    mysql_query(connexion, query);

    MYSQL_RES *result = mysql_store_result(connexion);

    char buffer[200];
    if(mysql_num_rows(result) == 1)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        sprintf(buffer, "CONSULT$%s$%s$%s$%s$%s$~", row[0], row[1], row[2], row[3], row[4]);
    }
    else
    {
        strcpy(buffer, "CONSULT$-1$~");
    }

    mysql_close(connexion);

    Send(hsock, buffer);


}

struct ARTICLE CaseBuy(int hsock, struct OVESP ovesp)
{
    struct ARTICLE article;

    MYSQL* connexion = mysql_init(NULL);
    mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);

    char query[200];
    sprintf(query, "SELECT * FROM articles WHERE id='%s';", ovesp.data1);
    mysql_query(connexion, query);

    MYSQL_RES *result = mysql_store_result(connexion);

    char buffer[200];
    if(mysql_num_rows(result) == 1)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        int stock = atoi(row[3]);

        if(stock >= atoi(ovesp.data2))
        {
            //Update stock
            int newStock = stock - atoi(ovesp.data2);
            sprintf(query, "UPDATE articles SET stock='%d' WHERE id='%s';", newStock, ovesp.data1);
            mysql_query(connexion, query);

            //Add to article
            strcpy(article.id, row[0]);
            strcpy(article.name, row[1]);
            strcpy(article.price, row[2]);
            strcpy(article.qtt, ovesp.data2);
            strcpy(article.img, row[4]);      
            article.index=dex;
            dex++;              

            strcpy(buffer, "BUY$");
            strcat(buffer, article.id);
            strcat(buffer, "$");
            strcat(buffer, article.qtt);
            strcat(buffer, "$~");
        }else{
            printf("Stock insuffisant.\n");
            strcpy(buffer, "BUY$0$~");
        }
    }
    else
    {
        strcpy(buffer, "BUY$-1$~");
    }

    Send(hsock, buffer);

    mysql_close(connexion);

    //print article
    printf("Article: %s %s %s %s %s %d\n", article.id, article.name, article.price, article.qtt, article.img , article.index);
    return article;
}

void CaseCart(int hsock, struct ARTICLE * article)
{
    //Check number of articles in cart
    int nbrArt = 0;
    for(int i = 0; i < 25; i++)
    {
        if(strcmp(article[i].id, "") != 0)
            nbrArt++;
    }

    //Send number of articles in cart

    char buffer[200];
    sprintf(buffer, "CART$%d$~", nbrArt);
    Send(hsock, buffer);

    //Send each article
    for(int i = 0; i < 25; i++)
    {
        if(strcmp(article[i].id, "") != 0)
        {
            strcpy(buffer, "CART$");
            strcat(buffer, article[i].id);
            strcat(buffer, "$");
            strcat(buffer, article[i].name);
            strcat(buffer, "$");
            strcat(buffer, article[i].price);
            strcat(buffer, "$");
            strcat(buffer, article[i].qtt);
            strcat(buffer, "$");
            strcat(buffer, article[i].img);
            strcat(buffer, "$~");

            Send(hsock, buffer);
        }
    }

}

void CaseCancel(int hsock, struct OVESP ovesp, struct ARTICLE * article)
{
    char idArt[10];
    int qtt;
    //1. Retirer du panier
    for(int i = 0; i < 25; i++)
    {
        if(article[i].index == atoi(ovesp.data1))
        {
            //printf("Article found \n");
            strcpy(idArt, article[i].id);
            strcpy(article[i].id, "");
            qtt = atoi(article[i].qtt);
            article[i].index=99;

        }
    }
    for(int i = 0; i < 25; i++)
    {
        if(article[i].index > atoi(ovesp.data1))
        {
            article[i].index--;
        }
    }
    //2. Remettre en stock
    MYSQL* connexion = mysql_init(NULL);
    mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);

    char query[200];
    sprintf(query, "SELECT * FROM articles WHERE id='%s';", idArt);
    mysql_query(connexion, query);

    MYSQL_RES *result = mysql_store_result(connexion);
    if(mysql_num_rows(result) == 1)
    {
        MYSQL_ROW row = mysql_fetch_row(result);
        int stock = atoi(row[3]);

        //Update stock
        int newStock = stock + qtt;
        sprintf(query, "UPDATE articles SET stock='%d' WHERE id='%s';", newStock,  idArt);
        mysql_query(connexion, query);
    }

    //3 Envoyer confirmation

    char buffer[200];
    strcpy(buffer, "CANCEL$OK$~");
    Send(hsock, buffer);

    mysql_close(connexion);
    dex--;
}

void CaseCancelAll(int hsock, struct OVESP ovesp, struct ARTICLE * article)
{
    //Pour chaque objet du panier
    MYSQL* connexion = mysql_init(NULL);
    mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);

    for(int i = 0; i < 25; i++)
    {
        if(strcmp(article[i].id, "") != 0)
        {
            //1. Remettre en stock
            

            char query[200];
            sprintf(query, "SELECT * FROM articles WHERE id='%s';", article[i].id);
            mysql_query(connexion, query);

            MYSQL_RES *result = mysql_store_result(connexion);
            if(mysql_num_rows(result) == 1)
            {
                MYSQL_ROW row = mysql_fetch_row(result);
                int stock = atoi(row[3]);

                //Update stock
                int newStock = stock + atoi(article[i].qtt);
                sprintf(query, "UPDATE articles SET stock='%d' WHERE id='%s';", newStock, article[i].id);
                mysql_query(connexion, query);
            }

            //2. Retirer du panier
            strcpy(article[i].id, "");

            
        }
        article[i].index=99;
    }

    mysql_close(connexion);
    //3 Envoyer confirmation

    char buffer[200];
    strcpy(buffer, "CANCELALL$OK$~");
    Send(hsock, buffer);
    dex=0;

}

void CaseConfirm(int hsock,struct OVESP ovesp, struct ARTICLE * article)
{
    printf("Confirm\n");
    //1 Calculer le prix total
    float total = 0;
    for(int i = 0; i < 25; i++)
    {
        if(strcmp(article[i].id, "") != 0)
        {
            printf("Article: %s %s %s %s %s\n", article[i].id, article[i].name, article[i].price, article[i].qtt, article[i].img);
            total += atof(article[i].price) * atoi(article[i].qtt);
        }
    }
    printf("Total: %f\n", total);
    //Insertion dans la table facture

    MYSQL* connexion = mysql_init(NULL);
    mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);

    char queryus[500];
    sprintf(queryus, "SELECT * FROM users WHERE username='%s';", ovesp.data1);
    mysql_query(connexion, queryus);

    MYSQL_RES *resultus = mysql_store_result(connexion);

    MYSQL_ROW rowus = mysql_fetch_row(resultus);

    char idUser[10];
    strcpy(idUser,rowus[0]);


    printf("Q1\n");
    char query[500];
    sprintf(query, "INSERT INTO factures (id_user, date, montant, paid) VALUES ('%s', NOW(), '%f', 0);", idUser, total);

    printf("Query: %s\n", query);

    if((mysql_query(connexion, query)==0))
    {
        printf("Insert ok\n");
    }
    else
    {
        printf("Insert ko\n");
    }
    


    printf("Q2\n");
    //Recuperer l'id de la facture  en listant les factures par id et en prenant celle avec l'id le plus haut
    char query2[500];
    sprintf(query2, "SELECT * FROM factures WHERE id_user='%s' ORDER BY id DESC LIMIT 1;", idUser);

    mysql_query(connexion, query2);

    MYSQL_RES *result = mysql_store_result(connexion);
    MYSQL_ROW row = mysql_fetch_row(result);

    printf("Q3\n");
    char idFacture[10];
    strcpy(idFacture,row[0]);

    //2 Pour chaque article du panier : insertion dans la table ventes
    printf("Q4 idFacture: %s\n", idFacture);
    for(int i = 0; i < 25; i++)
    {
        if(strcmp(article[i].id, "") != 0)
        {
            int ret;
            char query3[500];
            sprintf(query3, "INSERT INTO ventes (id_facture, id_produit, quantite) VALUES ('%s', '%s', '%s');", idFacture, article[i].id, article[i].qtt);
            ret = mysql_query(connexion, query3);
            printf("Ret: %d\n", ret);
        }
    }

    //3 Envoyer confirmation

    char buffer[200];
    strcpy(buffer, "CONFIRM$OK$~");
    Send(hsock, buffer);

    mysql_close(connexion);

    //4 Vider le panier

    for(int i = 0; i < 25; i++)
    {
        strcpy(article[i].id, "");
    }



}

void CaseLogout(int hsock, struct OVESP ovesp)
{

}

