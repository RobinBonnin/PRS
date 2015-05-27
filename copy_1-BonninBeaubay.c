K";
  sprintf(synack+7,"%4d",portData);
  socklen_t adrsize= sizeof(Client);
 

  recvfrom(descConnect,bufferConnect,RCVSIZE,0,Client,&adrsize);

  if (strcmp(bufferConnect,"SYN") ==0){
    printf("Réception synchronisation : %s\n", bufferConnect);
    memset(bufferConnect,0,RCVSIZE);
  }
  else{
    exit(1);
  }
  sendto(descConnect,synack,strlen(synack)+1,0,Client,adrsize);
  recvfrom(descConnect,bufferConnect,RCVSIZE,0, Client,&adrsize);

  if (strcmp(bufferConnect,"ACK") ==0){
    printf("Réception acquittement : %s\n", bufferConnect);
    memset(bufferConnect,0,RCVSIZE);

  }
  else{
    exit(1);
  }
	
}
//Fonction d'ouverture du fichier
 FILE* openFich(int descData,struct sockaddr* Client,socklen_t adrsize){
	
  char bufferInit [RCVSIZE];
  recvfrom(descData,bufferInit,RCVSIZE,0,Client,&adrsize);
  char fichToOpen [RCVSIZE];
  strncpy(fichToOpen,bufferInit,RCVSIZE);

  //phase d'envoi avec acquittement
  FILE* fichier = NULL;
  fichier =fopen(fichToOpen,"rb");

  if(fichier!= NULL){
    printf("Ouverture du fichier avec succès\n");
  }
  else{
    printf("Erreur lors de l'ouverture du fichier\n");
  }
  return fichier;
	
	
	
}
//fonction envoi
void envoi(int descData, int numSeg,FILE* fichier, struct sockaddr* Client,socklen_t adrsize){
	
	//Envoi normal
	char bufferData [RCVSIZE+6];
	memset(bufferData,0,RCVSIZE+6);
	int sntSize=0;
	char message[RCVSIZE];
	memset(message,0,RCVSIZE);
	char StringSeg[6];
	memset(StringSeg,0,6);
	sprintf(StringSeg, "%06d",numSeg);
	memcpy(bufferData,StringSeg,6);
	//char ack [9]="ACK";
	//strncpy(ack+3,StringSeg,6);
    fseek(fichier,RCVSIZE*(numSeg),SEEK_SET);
    sntSize=fread(message,1,RCVSIZE,fichier);
    memcpy(bufferData+6,message,sntSize);
    sendto(descData,bufferData,sntSize+6,0,Client,adrsize);
    
   
    //Gestion de la fin du fichier
    if(sntSize==0){
      memset(bufferData,0,RCVSIZE+6);
      memcpy(bufferData,StringSeg,6);
      memcpy(bufferData+6,"FIN",3);
      sendto(descData,bufferData,9,0,Client,adrsize);
      printf("Fin du fichier\n");
      ex