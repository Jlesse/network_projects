//
//  Struct Server.c
//  AirlineServer
//
//  Created by Julian on 10/16/14.
//  Copyright (c) 2014 Julian. All rights reserved.
//
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <errno.h>

int sock;                        /* Socket */
struct sockaddr_in servAddr;     /* Local address */
struct sockaddr_in clntAddr;     /* Client address */
unsigned int cliAddrLen;         /* Length of incoming message */
unsigned short servPort;          /* Server port */
unsigned long recvMsgSize;                 /* Size of received message */
unsigned long sendMessageSize;
//Seat type used in client request
typedef enum {ALL, PREMIUM, ECONOMY} SeatType;
//option usedin client request
typedef enum {CAP, AVAILABLE, PURCHASE} Option;
//Status Used in server response
typedef enum {FAIL, SUCCESS, NO_SEAT} Status;

typedef enum {TRUE, FALSE} Close;

//Holds data from file to be store in the Array
typedef struct flightData{
    char flightID[6];
    int pCap;
    int availPremCap;
    int eCap;
    int availEconCap;
} infoFromFile;

//message from client data;
typedef struct {
    char flightID[6];
    SeatType seat;
    Option op;
    int numberOfSeats;
    Close quit;
    } clientRequest;

//message to client data
typedef struct{
    char flightID[6];
    Status stat;
    int numberOfSeats;
}  serverResponse;

//buffer struct from client
clientRequest msgFromClient;
//buffer for struct to client
serverResponse msgToClient;

//current number of flights in flightData
int arraySize = 0;
//contains all flight data
infoFromFile dataArray[100];

//loads in txt file in to flight data array.
void loadInFile(){
    
    arraySize=0;
    char iD[16];
    int premCap, avPremCap, econCap, avEconCap;
    
    FILE * fp;
    
    fp = fopen ("FlightInfo.txt", "r");
    
    
    int i = 0;
    while(  fscanf(fp, "%s %d %d %d %d", iD, &premCap, &avPremCap, &econCap, &avEconCap) == 5){
        
        strcpy(dataArray[i].flightID, iD);

        dataArray[i].pCap = premCap;
        
        dataArray[i].availPremCap = avPremCap;
        
        dataArray[i].eCap = econCap;
        
        dataArray[i].availEconCap = avEconCap;
        i++;
        arraySize++;
    }
    printf("\n%s\n", "Loaded FlightInfo");
    fclose(fp);
}

//prints the array
void printArray(){
    for (int i = 0; i < arraySize; i++) {
        printf("%s\n", dataArray[i].flightID);
        printf("%d\n", dataArray[i].pCap);
        printf("%d\n", dataArray[i].availPremCap);
        printf("%d\n", dataArray[i].eCap);
        printf("%d\n", dataArray[i].availEconCap);
        
        
        
    }
}

//gets the index of the struct with the matching id
int search(char* flight){
    for(int i = 0; i < arraySize; i++){
        if (strcmp( dataArray[i].flightID, flight) == 0){
            return i;
        }
    }
    return -1;
}

//Over writes old contents of flightData.txt with the contents of the array.
void arrayToFile(){
    FILE * fp;
    
    fp = fopen ("FlightInfo.txt", "w");
    rewind(fp);
    
    for(int i = 0;i<arraySize;i++){
        char *id = dataArray[i].flightID;
        int premCap = dataArray[i].pCap;
        int avPremcap = dataArray[i].availPremCap;
        int econcap = dataArray[i].eCap;
        int avEconCap = dataArray[i].availEconCap;
        
        fprintf(fp, "%s\n%d\n%d\n%d\n%d\n", id,premCap,avPremcap,econcap,avEconCap);
    }
    fclose(fp);
    
}
//Error message for failure
void dieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}
// send a message to the client
void sendStruct(){
    //get the size in bytes of the struct to be sent for error checking
    sendMessageSize = sizeof(msgToClient);
    printf("%s\n%d\n%u",msgToClient.flightID, msgToClient.numberOfSeats,msgToClient.stat);
    /* Send datagram back to the client */
    if (sendto(sock,(struct serverRequest *) &msgToClient, sendMessageSize, 0,
               (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != sendMessageSize)
        dieWithError("sendto() sent a different number of bytes than expected");
    
}
//listen for a message from the client.
void receive(){
    /* Block until receive message from a client */
    recvMsgSize = recvfrom(sock,(struct clientRequest *) &msgFromClient, sizeof( clientRequest), 0,(struct sockaddr *) &clntAddr, &cliAddrLen);
        //dieWithError("recvfrom() failed ");
    
}

//retrieves and sends the requested flight info to the client.
void handleMsg(){
    loadInFile();
    receive();
    printf("Handling client %s\n", inet_ntoa(clntAddr.sin_addr));
    if(msgFromClient.quit == TRUE){
        printf("%s","Exiting Server...");
        close(sock);
        exit(0);
    }
    int index = search(msgFromClient.flightID);
    if(index == -1){
        msgToClient.stat = FAIL;
        sendStruct();
    }
    else{
    // get data out array
    struct flightData info = dataArray[index];
    strcpy(msgToClient.flightID,info.flightID);
    // If the client wants the capacity
    if(msgFromClient.op == CAP){
        if(msgFromClient.seat == ALL){
            msgToClient.numberOfSeats = (info.eCap + info.pCap);
            msgToClient.stat = SUCCESS;
        }
        else if (msgFromClient.seat == ECONOMY){
            msgToClient.numberOfSeats = info.eCap;
            msgToClient.stat = SUCCESS;
        }
        else if (msgFromClient.seat == PREMIUM){
            msgToClient.numberOfSeats = info.pCap;
            msgToClient.stat = SUCCESS;
        }
    }
    //If the client wants seat availability
    else if (msgFromClient.op == AVAILABLE){
        if(msgFromClient.seat == ALL){
            msgToClient.numberOfSeats = (info.availEconCap + info.availPremCap);
            msgToClient.stat = SUCCESS;
        }
        else if (msgFromClient.seat == ECONOMY){
            msgToClient.numberOfSeats = info.availEconCap;
            msgToClient.stat = SUCCESS;
        }
        else if (msgFromClient.seat == PREMIUM){
            msgToClient.numberOfSeats = info.availPremCap;
            msgToClient.stat = SUCCESS;
        }

    }
    //If the client wants to purchase seats
    else if (msgFromClient.op == PURCHASE){
        
        int toPurchase = msgFromClient.numberOfSeats;
        
        if (msgFromClient.seat == ECONOMY){
           
            int sum = info.availEconCap - toPurchase;
            
            if(sum < 0){
                msgToClient.stat = NO_SEAT;
                msgToClient.numberOfSeats = info.availEconCap;
            }
            else{
            dataArray[index].availEconCap = sum;
            msgToClient.numberOfSeats = sum;
            msgToClient.stat = SUCCESS;
            }
        }
        else if (msgFromClient.seat == PREMIUM){
            
            int sum = info.availPremCap - toPurchase;
            
            if(sum < 0){
                msgToClient.stat = NO_SEAT;
                msgToClient.numberOfSeats = info.availPremCap;
            }
            else{
            dataArray[index].availPremCap = sum;
            msgToClient.numberOfSeats = sum;
            msgToClient.stat = SUCCESS;
            }
        }

    }
    arrayToFile();
    sendStruct();
    }
}

int main(int argc, char *argv[]){
    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        return 1;
    }
    // assigan the port number
    servPort = atoi(argv[1]);
    
   
    /* Create socket for sending/receiving datagrams */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        dieWithError("socket() failed");
    
    /* Construct local address structure */
    memset(&servAddr, 0, sizeof(servAddr));   /* Zero out structure */
    servAddr.sin_family = AF_INET;                /* Internet address family */
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
    servAddr.sin_port = htons(servPort);      /* Local port */
    
    /* Bind to the local address */
    if (bind(sock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        dieWithError("bind() failed");
    
   for (;;) /* Run forever */
   {
        /* Set the size of the in-out parameter */
        cliAddrLen = sizeof(clntAddr);
        //receive();
    
    
    handleMsg();

    
   }
    return 0;
}

