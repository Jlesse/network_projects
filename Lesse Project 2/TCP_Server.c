//
//  main.c
//  TCP_Server
//
//  Created by Julian on 10/29/14.
//  Copyright (c) 2014 Julian. All rights reserved.
//
#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket() and bind() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_ntoa() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
//#include <errno.h>
#include <stdio.h>      /* for console I/O */
#include <stdbool.h>    /* for boolean values */

int sock;                        /* Socket */
struct sockaddr_in servAddr;     /* Local address */
struct sockaddr_in clntAddr;     /* Client address */
unsigned int cliAddrLen;         /* Length of incoming message */
unsigned short servPort;          /* Server port */
unsigned long recvMsgSize;                 /* Size of received message */
unsigned long sendMessageSize;
int arrSize = 0;

typedef enum {WHO, ADDRESS_LOOKUP,LOGIN, FAIL, LOGOUT} reqType;
typedef enum {FOUND, NOT_FOUND} outcome;


//struct to sent to server upon login
typedef struct {
    char userID[15];        /*Identifies user*/
    unsigned short port;
    reqType request;
} clientRequest;

//struct type for client data to be stored in the array.
typedef struct {
    char userID[15];
    unsigned short port;
    char clientAddress[16];
    outcome isFound;
} ClientInfo;

//list of client ID's their ports and adresses
ClientInfo clientData[100];



//an array of client ID's for WHO response
char whos[100][15];

//buffer for client message
clientRequest msgFromClient;

//instance of client info added to the array
ClientInfo dataItem;

//copies information from msgFromClient into dataItem and stores in client data array
void addUserToArray(){
    strcpy(dataItem.userID,msgFromClient.userID);
    dataItem.port = msgFromClient.port;
    strcpy(dataItem.clientAddress, inet_ntoa(clntAddr.sin_addr));
    clientData[arrSize] = dataItem;
    strcpy(whos[arrSize], msgFromClient.userID);
    arrSize += 1;
}

//Error message for failure
void dieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

//send ACK to client
void sendACKToClient(char * ack){
    //Buffer for login ACK from server
    char serverResponse[100];
    strcpy(serverResponse, ack);
    
    /* Send received datagram back to the client */
    if (sendto(sock, serverResponse, sizeof(serverResponse), 0,
               (struct sockaddr *) &clntAddr, sizeof(clntAddr)) < 0)
        dieWithError("sendto() sent a different number of bytes than expected");
}

//recieves struct from client
void receive(){
    cliAddrLen = sizeof(clntAddr);
    printf("%s\n","Recieving...");
    /* Block until receive message from a client */
    if ((recvMsgSize = recvfrom(sock,(struct clientRequest *) &msgFromClient, sizeof(clientRequest), 0,(struct sockaddr *) &clntAddr, &cliAddrLen) < 0)){
       dieWithError("recvfrom() failed");
    }

}

//checks to see if LOGIN ID is already in use
int isLoginIDInArray(char *iD){
    
    if (arrSize == 0){
        return -1;
    }
    int i;
    
    for(i = 0; strcmp(clientData[i].userID, iD) != 0; i++){
        if(i == arrSize){
            return -1;
        }
    }
    
        return i;
}

//stores info in array and/or sends back relevent ACK
void handleLogin(){
    if(isLoginIDInArray(msgFromClient.userID) != -1){
        sendACKToClient("ID already in use, please pick a different ID.");
    }
    else{
        addUserToArray();
        sendACKToClient("Succesfully logged in.");
    }
}

//send list of logged in clients to client
void sendWhosArray(){
    //get the size in bytes of the struct to be sent for error checking
    sendMessageSize = sizeof(whos);
    /* Send datagram back to the client */
    if (sendto(sock,&whos, sendMessageSize, 0,
               (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != sendMessageSize)
        dieWithError("sendto() sent a different number of bytes than expected");
    
}

//sends the dataItem in response to address look up from client
void sendDataItem(){
    
        //get the size in bytes of the struct to be sent for error checking
        sendMessageSize = sizeof(dataItem);
        /* Send datagram back to the client */
        if (sendto(sock,(struct ClientInfo *) &dataItem, sendMessageSize, 0,
                   (struct sockaddr *) &clntAddr, sizeof(clntAddr)) != sendMessageSize)
            dieWithError("sendto() sent a different number of bytes than expected");
        
    
}

//finds dataItem in array and sends it client
void addressLookUp(){
    int index = isLoginIDInArray(msgFromClient.userID);
    
    if(index != -1){
        strcpy(dataItem.userID, clientData[index].userID);
        dataItem.port = clientData[index].port;
        strcpy(dataItem.clientAddress, clientData[index].clientAddress);
        dataItem.isFound = FOUND;
    }
    else{
        dataItem.isFound = NOT_FOUND;
    }
        sendDataItem();
    
}

void logout(){
    
    for(int i = isLoginIDInArray(msgFromClient.userID) ;i < arrSize ; i++){
        clientData[i] = clientData[i+1];
        strcpy(whos[i], whos[i+1]);
    }
}

//decides what actions to take based on msgFromClient request type
//WHO, ADRESS_LOOKUP,LOGIN, FAIL, QUIT
void handleMsgFromClient(){
    receive();
    switch (msgFromClient.request) {
        case LOGIN:
            handleLogin();
            break;
        case WHO:
            sendWhosArray();
            break;
        case ADDRESS_LOOKUP:
            addressLookUp();
            break;
        case LOGOUT:
            logout();
            break;
        case FAIL:
            dieWithError("Invalid request type from client");
            break;
        default:
            dieWithError("Invalid request type");
            break;
    }
}

    
    int main(int argc, const char * argv[]) {
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
        
        
        while(true){
           
            handleMsgFromClient();
        }
        
        
        
        
    
        
        return 0;
    }
