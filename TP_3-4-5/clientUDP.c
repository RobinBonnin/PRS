#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define RCVSIZE 1024

int main (int argc, char *argv[]) {

  struct sockaddr_in Connect; 
  struct sockaddr_in Data;
  int valid= 1;
  int portConnect;
  int portData;
  char msg[RCVSIZE];
  char message[RCVSIZE];
  char bufferConnect[RCVSIZE];
  char bufferData[RCVSIZE+6];
  int msgSize;
  char numSeg [6];
  memset(numSeg,0,6);
  int rcvdSize=0;
 

  int sizeFichToWrite=strlen(argv[3]);
  char fichToWrite[sizeFichToWrite];

  if(argc==4) {
    portConnect= atoi(argv[2]);
    printf("Numéro de portConnect :%d\n", portConnect);
    strcpy(fichToWrite,argv[3]);
    printf("Le fichier à ouvrir est: %s\n",fichToWrite);

  }
  else {
    printf("Pas le bon nombre d'arguments en entrée\n");
    return 0;
  }


  //create socket
  int descConnect= socket(AF_INET, SOCK_DGRAM, 0);
  int descData= socket(AF_INET, SOCK_DGRAM, 0);

  // handle error
  if (descConnect < 0) {
    perror("cannot create socket Connect\n");
    return -1;
  }
  if (descData < 0) {
    perror("cannot create socket Data\n");
    return -1;
  }

  setsockopt(descConnect, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));
  setsockopt(descData, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  Connect.sin_family= AF_INET;
  Connect.sin_port= htons(portConnect);
  inet_aton(argv[1],&(Connect.sin_addr.s_addr)); 

  // Phase d'acquitement 

  int adrsize = sizeof(Connect);
  char syn[]  = "SYN";
  char ack[9]  = "ACK";
  strcat(ack,numSeg);

  sendto(descConnect,syn,strlen(syn)+1,0,(struct sockaddr*) &Connect,adrsize);
  recvfrom(descConnect,bufferConnect,RCVSIZE,0, (struct sockaddr*) &Connect,&adrsize);


  if (strncmp(bufferConnect,"SYN-ACK",7) ==0){
    printf("Réception acquittement: %s\n",bufferConnect);
    portData=atoi(bufferConnect+8);
    printf("Le port de données est: %d \n",portData);
    memset(bufferConnect,0,RCVSIZE);
  }
  else{
    return(-1);
  }


  sendto(descConnect,ack,strlen(ack)+1,0,(struct sockaddr*) &Connect,adrsize);


  Data.sin_family= AF_INET;
  Data.sin_port= htons(portData);
  inet_aton(argv[1],&(Data.sin_addr.s_addr));


  //Phase de réception fichier
  FILE* fichier = NULL;
  fichier =fopen(fichToWrite,"wb");

  if(fichier!= NULL){
    printf("Ouverture du fichier avec succès\n");
  }
  else{
    printf("Erreur lors de l'ouverture du fichier\n");
    exit(1);
  }


  printf("En attente de réception\n");


  while(1){

    
    memset(bufferData,0,RCVSIZE+6);
    memset(bufferConnect,0,RCVSIZE);
    memcpy(ack+3 ,numSeg,6);

    sendto(descData,ack,sizeof(ack),0,(struct sockaddr*) &Data,adrsize);
    printf("Acquittement numéro:%d\n",atoi(numSeg));
    memset(ack+3,0,sizeof(ack));
    memset(numSeg,0,6);
    rcvdSize=recvfrom(descData,bufferData,RCVSIZE+6,0, (struct sockaddr*) &Data,&adrsize);
    strncpy(numSeg,bufferData+(rcvdSize-6),6);
    
    //Gestion fin fichier
    if(strncmp(bufferData,"FIN",3) ==0){

      printf("Fin d'échange\n");
      exit(1);

    }

    //Gestion fichier non terminé
    else{

      //printf("Server says: %s\n",bufferData);
      int fw= fwrite(bufferData,1,rcvdSize-6,fichier);
      printf("fwrite= %d\n",fw);
      fflush(fichier);

    }

  }

  /*//Phase de communication normale
    printf("Démarrage de la communication: \n");

    int cont= 1;
    while (cont) {
    fgets(msg, RCVSIZE, stdin);
    sendto(descData,msg,strlen(msg),0,(struct sockaddr*) &Data,adrsize);
    recvfrom(descData,bufferData,RCVSIZE,0, (struct sockaddr*) &Data,&adrsize);
    printf("Server says: %s",bufferData);
    memset(bufferData,0,RCVSIZE);    
    if (strcmp(msg,"stop\n") ==0 || strcmp(bufferData,"stop\n") == 0) {
    cont= 0; 
    }
    }*/

  close(descData);
  fclose(fichier);
  return 0;
}
