#include "windowclient.h"
#include "ui_windowclient.h"
#include <QMessageBox>
#include <string>
#include <SockLib.h>
#include <stdlib.h>
#include <unistd.h>

using namespace std;

extern WindowClient *w;

int hsock;
int currentArticle = 0;

#define REPERTOIRE_IMAGES "images/"

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);

    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    ui->pushButtonPayer->setText("Confirmer achat");
    setPublicite("!!! Bienvenue sur le Maraicher en ligne !!!");

    // Connection au serveur
    hsock = SocketClient();
}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
  Send(hsock, "EXIT$~");
 
  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{
  char username[50],password[50];
  int nouveauClient = isNouveauClientChecked();
  strcpy(username,getNom());
  strcpy(password,getMotDePasse());

  if (strlen(username) == 0)
  {
    dialogueErreur("Erreur","Le nom est obligatoire");
    return;
  }
  if (strlen(password) == 0)
  {
    dialogueErreur("Erreur","Le mot de passe est obligatoire");
    return;
  }

  // Envoi de la requete au serveur

  char msg[500];
  sprintf(msg,"LOGIN$%s$%s$%d$~",username,password,nouveauClient);
  Send(hsock,msg);

  // Reception de la reponse du serveur

  char reponse[500];
  
  Receive(hsock,reponse);
  struct OVESP ovesp = ParseOVESP(reponse);
  // Traitement de la reponse du serveur

  if(strcmp(ovesp.request,"LOGIN") == 0)
  {
    if(strcmp(ovesp.data1,"OK") == 0)
    {
      loginOK();
      dialogueMessage("Information","Vous êtes connecté");

      //Envoi Consult 1 au serveur
      char msg[500];
      sprintf(msg,"CONSULT$1$~");
      Send(hsock,msg);

      // Reception de la reponse du serveur

      char reponse[500];
      Receive(hsock,reponse);
      struct OVESP ovesp = ParseOVESP(reponse);

      // Traitement de la reponse du serveur

      if(strcmp(ovesp.request,"CONSULT") == 0)
      {
        if(strcmp(ovesp.data1,"-1") != 0)
        {
          // Affichage de l'article
          setArticle(ovesp.data2,atof(ovesp.data3),atoi(ovesp.data4),ovesp.data5);
          currentArticle = atoi(ovesp.data1);
        }
        else
        {
          dialogueErreur("Erreur Consult",ovesp.data2);
        }
      }



    }
    else
    {
      dialogueErreur("Erreur Login",ovesp.data2);
    }
  }
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
  logoutOK();
  currentArticle = 0;
  
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
  if(currentArticle == 21)
  {
    currentArticle = 1;

  }
  else
  {
    currentArticle++;
  }

  //Envoi Consult 1 au serveur
  char msg[500];
  sprintf(msg,"CONSULT$%d$~",currentArticle);
  Send(hsock,msg);

  // Reception de la reponse du serveur

  char reponse[500];
  Receive(hsock,reponse);
  struct OVESP ovesp = ParseOVESP(reponse);

  // Traitement de la reponse du serveur

  if(strcmp(ovesp.request,"CONSULT") == 0)
  {
    if(strcmp(ovesp.data1,"-1") != 0)
    {
      // Affichage de l'article
      //dialogueMessage("Prix: ",ovesp.data3);
      setArticle(ovesp.data2,atof(ovesp.data3),atoi(ovesp.data4),ovesp.data5);
      printf("Article price: %f\n",atof(ovesp.data3));
    }
    else
    {
      dialogueErreur("Erreur Consult",ovesp.data2);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
  if(currentArticle == 1)
  {
    currentArticle = 21;

  }
  else
  {
    currentArticle--;
  }

  //Envoi Consult 1 au serveur

  char msg[500];
  sprintf(msg,"CONSULT$%d$~",currentArticle);
  Send(hsock,msg);

  // Reception de la reponse du serveur

  char reponse[500];
  Receive(hsock,reponse);
  struct OVESP ovesp = ParseOVESP(reponse);

  // Traitement de la reponse du serveur

  if(strcmp(ovesp.request,"CONSULT") == 0)
  {
    if(strcmp(ovesp.data1,"-1") != 0)
    {
      // Affichage de l'article
      setArticle(ovesp.data2,atof(ovesp.data3),atoi(ovesp.data4),ovesp.data5);
    }
    else
    {
      dialogueErreur("Erreur Consult",ovesp.data2);
    }
  }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
  /*
  char buffer[200] = "BUY$3$1$~";
    Send(sock, buffer);

    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);
  
  */

  int quantite = getQuantite();
  if (quantite == 0)
  {
    dialogueErreur("Erreur","La quantité doit être supérieure à 0 ! ");
    return;
  }else{
    char msg[500];
    sprintf(msg,"BUY$%d$%d$~",currentArticle,quantite);
    Send(hsock,msg);

    // Reception de la reponse du serveur

    char reponse[500];
    Receive(hsock,reponse);
    struct OVESP ovesp = ParseOVESP(reponse);

    // Traitement de la reponse du serveur

    if(strcmp(ovesp.request,"BUY") == 0)
    {
      if(atoi(ovesp.data2) > 0)
      {
        //Consult sur l'article
        char msgConsult[500];
        sprintf(msgConsult,"CONSULT$%d$~",currentArticle);
        Send(hsock,msgConsult);

        // Reception de la reponse du serveur

        char reponseConsult[500];
        Receive(hsock,reponseConsult);
        struct OVESP ovesp4 = ParseOVESP(reponseConsult);

        // Traitement de la reponse du serveur



        // Affichage de l'article
        ajouteArticleTablePanier(ovesp4.data1,atof(ovesp4.data3),quantite);
        
        //Calcul du total
        float total = 0;

        //Envoi CART au serveur
        char msg[500];
        sprintf(msg,"CART$~");
        Send(hsock,msg);

        // Reception de la reponse du serveur

        char reponse[500];
        Receive(hsock,reponse);
        struct OVESP ovesp2 = ParseOVESP(reponse);

        // Traitement de la reponse du serveur

        int nbrArt = atoi(ovesp2.data1);
        int ret;
        for(int i = 0; i < nbrArt; i++)
        {
            char buffer3[500];
            ret = Receive(hsock, buffer3);
            
            struct OVESP ovesp3 = ParseOVESP(buffer3);
            total += atof(ovesp3.data3)*atoi(ovesp3.data4);
        }

        setTotal(total);
      }
      else
      {
        dialogueErreur("Erreur Buy",ovesp.data2);
      }
    }
  }


}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{
  /*
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
  */

  int indice = getIndiceArticleSelectionne();
  if (indice == -1)
  {
    dialogueErreur("Erreur","Aucun article n'est sélectionné");
    return;
  }

  //Envoi Cancel au serveur
  char msg[500];
  sprintf(msg,"CANCEL$%d$~",indice);
  Send(hsock,msg);

  // Reception de la reponse du serveur

  char reponse[500];
  Receive(hsock,reponse);
  struct OVESP ovesp = ParseOVESP(reponse);

  // Traitement de la reponse du serveur

  if(strcmp(ovesp.request,"CANCEL") == 0)
  {
    if(strcmp(ovesp.data1,"OK") == 0)
    {
      printf("Indice: %d\n",indice);
      //Suppression de l'article
      ui->tableWidgetPanier->removeRow(indice);

      //Calcul du total
      float total = 0;

      //Envoi CART au serveur
      char msg[500];
      sprintf(msg,"CART$~");
      Send(hsock,msg);

      // Reception de la reponse du serveur

      char reponse[500];
      Receive(hsock,reponse);
      struct OVESP ovesp2 = ParseOVESP(reponse);

      // Traitement de la reponse du serveur

      int nbrArt = atoi(ovesp2.data1);
      int ret;
      for(int i = 0; i < nbrArt; i++)
      {
          char buffer3[500];
          ret = Receive(hsock, buffer3);
          
          struct OVESP ovesp3 = ParseOVESP(buffer3);
          total += atof(ovesp3.data3)*atoi(ovesp3.data4);
      }

      setTotal(total);
    }
    else
    {
      dialogueErreur("Erreur Cancel",ovesp.data2);
    }
  }

}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{
  /*
  char buffer[200] = "CANCEL ALL$~";
    Send(sock, buffer);

    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);
  */

  //Envoi Cancel ALL au serveur
  char msg[500];
  sprintf(msg,"CANCELALL$~");
  Send(hsock,msg);

  // Reception de la reponse du serveur

  char reponse[500];
  Receive(hsock,reponse);
  struct OVESP ovesp = ParseOVESP(reponse);

  // Traitement de la reponse du serveur

  if(strcmp(ovesp.request,"CANCELALL") == 0)
  {
    if(strcmp(ovesp.data1,"OK") == 0)
    {
      //Suppression de tous les articles
      videTablePanier();
      float total = 0;
      setTotal(total);
    }
    else
    {
      dialogueErreur("Erreur Cancel ALL",ovesp.data2);
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{
  /* 
  char buffer[200] = "CONFIRM$1$~";
    Send(sock, buffer);

    char buffer2[200];
    int ret = Receive(sock, buffer2);
    printf("Received: %s\n", buffer2);
  */

  //Envoi Confirm au serveur
  char username[50];
  strcpy(username,getNom());

  char msg[500];
  sprintf(msg,"CONFIRM$%s$~", username);
  Send(hsock,msg);

  // Reception de la reponse du serveur

  char reponse[500];
  Receive(hsock,reponse);
  struct OVESP ovesp = ParseOVESP(reponse);

  // Traitement de la reponse du serveur

  if(strcmp(ovesp.request,"CONFIRM") == 0)
  {
    if(strcmp(ovesp.data1,"OK") == 0)
    {
      //Suppression de tous les articles
      videTablePanier();
      float total = 0;
      setTotal(total);
      dialogueMessage("Information","Votre commande a été validée");
    }
    else
    {
      dialogueErreur("Erreur Confirm",ovesp.data2);
    }
  }
}
