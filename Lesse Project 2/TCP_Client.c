//
//  main.c
//  TCP_Client
//
//  Created by Julian on 10/28/14.
//  Copyright (c) 2014 Julian. All rights reserved.
//

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset()  and other String functions*/
#include <unistd.h>     /* for close() */
#include <stdbool.h>    /* for boolean values */
#include <sys/mman.h>   /* for mmap*/
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <errno.h>

#define RCVBUFSIZE 32   /* Size of receive buffer */

int sock;                        /* UDP Socket descriptor */
int tcpSockIn;                   /* TCP listening socket descriptor */
int peerSock;                    /* TCP receiving socket descriptor */
int tcpSockOut;                  /* TCP sending socket descriptor */
struct sockaddr_in toAddr;       /*TCP Peer address */
struct sockaddr_in ServAddr;     /*UDP server address */
struct sockaddr_in fromAddr;     /*UDP local address*/
struct sockaddr_in thisAddr;     /*TCP local address*/
unsigned short servPort;         /*server port */
unsigned short recvPort;         /*TCP recvPort */
unsigned short peerPort;         /*Port to send connection request to*/
char *peerIP;                    /*IP address of peer to connect to */
unsigned int fromSize;           /* byte size of recieved UDP message*/
struct sockaddr_in clntAddr;     /* client address for TCP recv*/
int sendSock;                    /* socket for sending messages */
char thisUserID[15];             /*The user Id of this user*/
bool notend;                     /* turns off receive loop */
int * thing;


//Used in clientRequest specifies to server type of request
typedef enum {WHO, ADDRESS_LOOKUP, LOGIN, FAIL, LOGOUT} reqType;

//used in ClientInfo
typedef enum {FOUND, NOT_FOUND} outcome;

//struct to send to server upon login
typedef struct {
    char userID[15];   /*Identifies user*/
    unsigned short port;
    reqType request;
} clientRequest;

//struct for receiving client data from server
typedef struct {
    char userID[15];
    unsigned short port;
    char clientAddress[16];
    outcome isFound;
} ClientInfo;


//used as buffer for ACK from server when logging in
char serverAckResponse[100];

//an array of client ID's for WHO response
char whos[100][15];

//buffer for login message to server
clientRequest msgToServ;

ClientInfo dataItem;

//Error message for failure
void dieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

//creates the UDP socket for communication with the server
void createUDPSocket(){
    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        dieWithError("socket() failed");
    
    /* Construct the server address structure */
    memset(&ServAddr, 0, sizeof(ServAddr));         /* Zero out structure */
    ServAddr.sin_family = AF_INET;                      /* Internet addr family */
    ServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  /* Server IP address */
    ServAddr.sin_port   = htons(servPort);           /* Server port*/
}

// receives message on connected socket and prints
void handleTCPIncoming(int inSock){
    char inBuffer[RCVBUFSIZE];        /* Buffer for echo string */
    long recvMsgSize;                 /* Size of received message */
    printf("%s\n", "receiving message from client");
    
    //loop until the terminating /// is entered
    notend = true;
    while(notend){
        /* Receive message from client */
        if ((recvMsgSize = recv(inSock, inBuffer, RCVBUFSIZE, 0)) < 0)
            dieWithError("recv() failed");
        
        
        /* receive again until end of transmission */
        while (recvMsgSize > 0 && notend == true)      /* zero indicates end of transmission */
        {   if(strcmp(inBuffer,"///")==0){
                notend = false;
                printf("%s\n", "peer ended chat...");
            }
            else{
                printf("%s\n",inBuffer);
                //inBuffer[recvMsgSize] = '\0';  /* Terminate the string! */
                memset(inBuffer,'\0',sizeof(inBuffer));
            
            /* See if there is more data to receive */
            if ((recvMsgSize = recv(inSock, inBuffer, RCVBUFSIZE, 0)) < 0)
                dieWithError("recv() failed");
            }
        }
    }
    
    
}

//creates the TCP socket for listening, used in child process
void createRecvSock(){
    
    
    /* Create a sock stream/TCP socket */
    if ((tcpSockIn = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        dieWithError("socket() failed");
    
    /* Construct the address structure */
    memset(&thisAddr, 0, sizeof(thisAddr));               /* Zero out structure */
    thisAddr.sin_family = AF_INET;                        /* Internet addr family */
    thisAddr.sin_addr.s_addr = htonl(INADDR_ANY);         /*IP address */
    thisAddr.sin_port   = htons(recvPort);                /*port*/
    
    // Bind to the local address
    if (bind(tcpSockIn, (struct sockaddr*) &thisAddr, sizeof(thisAddr)) < 0)
        dieWithError("bind() failed");
    
    //char inBuffer[100];
}

void listenForInChat(){
    // listen for incoming connections
    if (listen(tcpSockIn, 5) < 0)
        dieWithError("listen() failed");
}

int acceptIncomingChat(){
    // Set length of client address
    unsigned int clntAddrLen = sizeof(clntAddr);
   
    // Wait for a client to connect
    peerSock = accept(tcpSockIn, (struct sockaddr *) &clntAddr, &clntAddrLen);
    return peerSock;
    
}

//create the TCP socket for connecting and sending, used in parent process.
void createTCPSendSocket(){
    printf("%s\n", "creating TCP send sock");
    printf("%s\n", "enter /// to stop chat");
    char sendString[40];
    memset(sendString, '\0', 40);
    strcpy(sendString, "User: ");
    strcat(sendString, thisUserID);
    strcat(sendString, " wants to chat.");
    // Create stream socket using TCP
    sendSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sendSock < 0)
        dieWithError("socket() failed");
    
    // Construct the peer address structure
    memset(&toAddr, 0, sizeof(toAddr)); // Zero out structure
    toAddr.sin_family = AF_INET; // IPv4 address family
    toAddr.sin_addr.s_addr = inet_addr(peerIP);   /* Server IP address */
    toAddr.sin_port        = htons(peerPort); /* Server port */
    
    
    
    
 
    
    //Attempt connection to peer
    if (connect(sendSock, (struct sockaddr *) &toAddr, sizeof(toAddr)) < 0)
        dieWithError("connect() failed");
    printf("%s\n", "Connected");
    size_t stringLen = strlen(sendString); // Input length
    
    // Send the string to the server
    ssize_t numBytes = send(sendSock, sendString, stringLen, 0);
    
    if (numBytes < 0)
        dieWithError("send() failed");
    else if (numBytes != stringLen)
        dieWithError("send() sent different number of bytes than expected");
}

//sends TCP messages
void sendTCPmsg(){
    char mesg[150];
    bool goSend = true;
    //send message until /// is entered.
    while (goSend)
    {
        scanf(" %[^\n]s",mesg);
        if(strcmp(mesg,"///")==0){
            goSend = false;
        }
        unsigned long mesgLen = strlen(mesg);
        send(sendSock, mesg, mesgLen,0);
        memset(mesg,0,sizeof(mesg));
    }
    close(sendSock);
}

//sends msgToServ to the server
void sendStructToServ(){
    //set send sze parameter
    int sendStructSize = sizeof(msgToServ);
    
    /* Send the struct to the server */
    if (sendto(sock, (struct clientRequest *)&msgToServ, sendStructSize, 0, (struct sockaddr *)
               &ServAddr, sizeof(ServAddr)) != sendStructSize)
        dieWithError("sendto() sent a different number of bytes than expected");
    
}

//receives an acknowledgment string from the server
void receiveACKfromServer(){
    
    fromSize = sizeof(serverAckResponse);
    unsigned int f = sizeof(fromAddr);
    recvfrom(sock, serverAckResponse, fromSize, 0,(struct sockaddr *) &fromAddr, &f);
    
    
    if (ServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }
    
}

//receive User Id array
void receiveWhosArray(){
    fromSize = sizeof(whos);
    unsigned int f = sizeof(fromAddr);
    recvfrom(sock, &whos, fromSize, 0,(struct sockaddr *) &fromAddr, &f);
    
    
    if (ServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }
}

//receive requested ID and port number from server
void receiveDataItem(){
    fromSize = sizeof(fromAddr);
    recvfrom(sock,(struct ClientInfo *) &dataItem, sizeof(ClientInfo), 0,(struct sockaddr *) &fromAddr, &fromSize);
    
    
    if (ServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }
    
    printf("%s\n", dataItem.userID);
    printf("%s\n", dataItem.clientAddress);
    printf("%hd\n", dataItem.port);
    
    if (dataItem.isFound == NOT_FOUND) {
        printf("%s", "UserID was not found.");
    }
    
    peerIP = dataItem.clientAddress;
    peerPort = dataItem.port;
}

//prints list of logged in users
void printWhosArray(){
    printf("%s\n","Logged in users");
    printf("%s\n","===============");
    for(int i = 0; strcmp(whos[i], "")!=0 ;i++){
        printf("%s\n",whos[i]);
    }
}

//gets necessary input from user and sends login info to the server for storage
void login(){
    bool badInput = true;
    bool unavailableId = true;
    char inputBuffer[100];
    
    //char userID[15];
    // memset(userID,'\0',15);
    printf("%s\n", "Welcome to the super cool chat application!");
    printf("%s\n", "===========================================");
    while(unavailableId){
        while(badInput){
            memset(inputBuffer, '\0', 100);
            printf("%s\n", "Please enter a unique userId: ");
            memset(stdin, '\0', 15);
            fgets(inputBuffer, 99, stdin);
            if (strlen(inputBuffer)>14){
                
                printf("%s","ID input was to large, input an ID of 14 chars or less.");
            }
            else{
                badInput = false;
                inputBuffer[strlen(inputBuffer)-1]= '\0';
                strcpy(msgToServ.userID,inputBuffer);
                strcpy(thisUserID, inputBuffer);
            }
            memset(inputBuffer, '\0', 100);
            printf("%s\n", "Please enter a port number to listen on: ");
            memset(stdin, '\0', 15);
            scanf("%hd",&recvPort);
            
        }
        msgToServ.port = recvPort;
        msgToServ.request = LOGIN;
        sendStructToServ();
        printf("%s\n","Sending...");
        receiveACKfromServer();
        fflush(stdin);
        printf("%s", "Reveived ACK: ");
        printf("%s\n", serverAckResponse);
        
        if(strcmp(serverAckResponse,"Succesfully logged in.") == 0){
            unavailableId = false;
        }
    }
}

//retrieves the peer port and address from the server
void addressLookup(){
    bool badInput = true;
    char inputBuffer[100];
    
    
    while(badInput){
        memset(inputBuffer, '\0', 100);
        printf("%s", "Enter the user ID you would like initiate a chat with: ");
        memset(stdin, '\0', 15);
        fgets(inputBuffer, 99, stdin);
        if (strlen(inputBuffer)>14){
            
            printf("%s","ID input was to large, input an ID of 14 chars or less.");
        }
        else{
            badInput = false;
            inputBuffer[strlen(inputBuffer)-1]= '\0';
            strcpy(msgToServ.userID,inputBuffer);
        }
        
    }
    msgToServ.request = ADDRESS_LOOKUP;
    sendStructToServ();
    receiveDataItem();
    if(dataItem.isFound == NOT_FOUND){
        printf("ID not found...");
    }
}

//sets a boolean to let the parent know if a connection has been established
void notifyParentofConnect(){
    
}

//takes prompts user for input to select an action
void mainMenu(){
    char inputBuff[20];
    int option = 0;
    bool badOption = true;
    while(badOption){
        printf("    MAIN MENU    \n");
        printf("==================\n");
        printf("1.See list of available users\n");
        printf("2. Initiate a talk session\n");
        printf("3. Logout and quit\n");
        memset(stdin, '\0', 10);
        fgets(inputBuff, 19, stdin);
        sscanf(inputBuff, "%d", &option);
        
        
        if(option != 1 && option != 2 && option != 3){
            printf("%s", "invalid option please select 1 2 or 3");
        }
        else{
            badOption = false;
        }
        switch (option) {
            case 1:
                msgToServ.request = WHO;
                sendStructToServ();
                receiveWhosArray();
                printWhosArray();
                break;
            case 2:
                printf("%s\n","Enter the ID of the user you would like to connect with: ");
                scanf("%s", msgToServ.userID);
                msgToServ.request = ADDRESS_LOOKUP;
                sendStructToServ();
                receiveDataItem();
                createTCPSendSocket();
                sendTCPmsg();
                break;
            case 3:
                printf("%s\n", "Logout");
                msgToServ.request = LOGOUT;
                strcpy(msgToServ.userID, thisUserID);
                sendStructToServ();
                exit(0);
                
                break;
            default:
                break;
                
        }
        badOption = true;
    }
    
    
}

int main(int argc, const char * argv[]) {
    
    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }
    
    servPort = atoi(argv[1]);
    
    createUDPSocket();
    login();
    
    
    
    
    // Fork child process and report any errors
    pid_t processID = fork();
    if (processID < 0)
        dieWithError("fork() failed");
    if (processID == 0) {          // The child process
       
        createRecvSock();
        for(;;){
            listenForInChat();           //parent UDP socket
            acceptIncomingChat();
            handleTCPIncoming(peerSock);
        }
        
    }
    else{       // The parent process
     
        mainMenu();
        close(peerSock); // parent closes incoming chat socket handle
    }
    
    
    return 0;
}
