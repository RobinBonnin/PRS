#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/time.h>
#include <netinet/in.h>

#define RCVSIZE 1024

// Faire données côté serveur (on communique sur le port connexion)  + envoi côté client sur le nouveau port



//Fonction réception 
void* emission(int *pointeurAck , int descData, struct sockaddr_in Data, FILE* fichier){
	
	
	int adrsize = sizeof(Data);
	char * tabBuffer[4][RCVSIZE];
	int sentSize;
	char numSeg[6];
	memset(numSeg,0,6)
	int i=0;
	int j=0;
	for(i=0;i<4;i++){
		sentSize=fread(tabBuffer[i],1,RCVSIZE,fichier);
		memcpy(message+sentSize,numSeg,6);
		for(j=0;j<sentSize;j++){
		
		sendto(descData,tabBuffer[i],sentSize+6,0,(struct sockaddr*) &Data,adrsize);
	}
	}	
	
}                                        


//fonction ouverture fichier
void ouverture_fichier(char fichToOpen []){ 

  FILE* fichier = NULL;
  fichier =fopen(fichToOpen,"rb");

  if(fichier!= NULL){
    printf("Ouverture du fichier avec succès\n");
  }
  else{
    printf("Erreur lors de l'ouverture du fichier\n");
  }
}

int main (int argc, char *argv[]) {

  struct sockaddr_in Data, Connect;
  int valid = 1;
  int portData=2424;
  int portConnect;
  socklen_t alen;
  char bufferConnect[RCVSIZE];
  char bufferData[RCVSIZE];
  char buffer[RCVSIZE];
  fd_set tabdesc;
  FD_ZERO(&tabdesc); //mise à zéro du tableau de descripteur
  char message[RCVSIZE+6]; //tableau de char de communication
  int messageSize;
  int sntSize=0;
  int numSeg=0;
  char StringSeg[6];
  memset(StringSeg,0,6);
  char * tabBuffer [4];
  //malloc à faire
  //réallouer quand je peux agrandir ma fenetre
  //thread pour dissocier envoie et acquittement
  
  
  int startTime=0;
  int endTime=0;
  
  int ackRcvd=0;
  int nbrToSend=1;

  int sizeFichToOpen=strlen(argv[2]);
  char fichToOpen[sizeFichToOpen];
  
//récupération arguments
  if(argc == 3) {
    portConnect = atoi(argv[1]);
    printf("Le port de Connection est :%d\n",portConnect);
    strcpy(fichToOpen,argv[2]);
    printf("Le fichier à ouvrir est: %s\n",fichToOpen);
  }
  else {
    printf("Pas le bon nombre d'arguments en entrée\n");
    return 0;
  }

  //create socket
  int descData= socket(AF_INET, SOCK_DGRAM, 0);
  int descConnect= socket(AF_INET, SOCK_DGRAM, 0);
  // handle error
  if (descData< 0) {
    perror("cannot create socket data\n");
    return -1;
  }
  if (descConnect < 0) {
    perror("cannot create socket connection\n");
    return -1;
  }

  setsockopt(descData, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));
  setsockopt(descConnect, SOL_SOCKET, SO_REUSEADDR, &valid, sizeof(int));

  Data.sin_family= AF_INET;
  Data.sin_port= htons(portData);
  Data.sin_addr.s_addr= htonl(INADDR_ANY);

  Connect.sin_family= AF_INET;
  Connect.sin_port= htons(portConnect);
  Connect.sin_addr.s_addr= htonl(INADDR_ANY);

  if (bind(descConnect, (struct sockaddr*) &Connect, sizeof(Connect)) == -1 ) {
    perror("Bind fail sur Donnees\n");
    close(descConnect);
    return -1;
  }
  if (bind(descData, (struct sockaddr*) &Data, sizeof(Data)) == -1 ) {
    perror("Bind fail sur Data\n");
    close(descData);
    return -1;
  }

  int adrsize = sizeof(Connect);
  //Phase d'acquittement
  char synack[] ="SYN-ACK 2424";
  char ack[9]="ACK";

  recvfrom(descConnect,bufferConnect,RCVSIZE,0, (struct sockaddr*) &Connect,&adrsize);
  printf("Port de connect:%d\n",Connect.sin_port);

  if (strcmp(bufferConnect,"SYN") ==0){
    printf("Réception synchronisation : %s\n", bufferConnect);
    memset(bufferConnect,0,RCVSIZE);
  }
  else{
    return (-1);
  }
  sendto(descConnect,synack,strlen(synack)+1,0,(struct sockaddr*) &Connect,adrsize);	
  recvfrom(descConnect,bufferConnect,RCVSIZE,0, (struct sockaddr*) &Connect,&adrsize);
  if (strcmp(bufferConnect,"ACK") ==0){
    printf("Réception acquittement : %s\n", bufferConnect);
    memset(bufferConnect,0,RCVSIZE);

  }
  else{
    return(-1);
  }




}

/* 
//phase d'envoi avec acquittement
FILE* fichier = NULL;
  fichier =fopen(fichToOpen,"rb");

  if(fichier!= NULL){
    printf("Ouverture du fichier avec succès\n");
  }
  else{
    printf("Erreur lors de l'ouverture du fichier\n");
  }

 while(1){
		
    memset(bufferData,0,RCVSIZE);
    memset(message,0,RCVSIZE+6);
    memset(bufferConnect,0,RCVSIZE);
    
    recvfrom(descData,bufferConnect,sizeof(bufferConnect),0, (struct sockaddr*) &Data,&adrsize);
    memset(StringSeg,0,6);
    strncpy(StringSeg,bufferConnect+3,6);
    strncpy(ack+3,StringSeg,6);
    printf("Client says: %s\n",ack);
 
    sntSize=fread(message,1,RCVSIZE,fichier);
    	
    if (strcmp(bufferConnect,ack)==0) {

        printf("Réception acquittement :%s\n",StringSeg);
        numSeg+=1; 
        memset(ack+3,0,9);
		sprintf(StringSeg, "%6d",numSeg);
		memcpy(message+sntSize,StringSeg,6);
		sendto(descData,message,sntSize+6,0,(struct sockaddr*) &Data,adrsize);
		//printf("Server send: %s\n",message);
	}
    
    //Gestion de la fin du fichier
    if(sntSize==0){
      sendto(descData,"FIN",3,0,(struct sockaddr*) &Data,adrsize);
      printf("Server send: FIN\n");
      printf("Fin du fichier\n");
      exit(1);
    }
    //Envoi quand fichier non terminé
 
   
 } 
      
  
  
  



  /*printf("Démarrage de la communication:\n");

  //Phase de communication normale
  int cont = 1;
  while (1) {		
  recvfrom(descData,bufferData,RCVSIZE,0, (struct sockaddr*) &Data,&adrsize);
  printf("Client says: %s",bufferData);
  memset(bufferData,0,RCVSIZE);
  fgets(message, RCVSIZE, stdin);
  sendto(descData,message,strlen(message),0,(struct sockaddr*) &Data,adrsize);	    
  if (strcmp(message,"stop\n") ==0 || strcmp(bufferData,"stop\n") == 0) {
  cont= 0; 
  }

  }

  close(descData);
  fclose(fichier); 
  return 0;
}*/
