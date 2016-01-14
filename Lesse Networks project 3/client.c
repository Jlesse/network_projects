//
//  main.c
//  ATM_client
//
//  Created by Julian on 11/24/14.
//  Copyright (c) 2014 Julian. All rights reserved.
//
#include <stdio.h>              /* for printf() and fprintf() */
#include <sys/socket.h>         /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>          /* for sockaddr_in and inet_addr() */
#include <stdlib.h>             /* for atoi() and exit() */
#include <string.h>             /* for memset() */
#include <unistd.h>             /* for close() */
#include <stdbool.h>            /* for boolean*/

int servSock;                   /* Socket handle for server*/
struct sockaddr_in    servAddr; /* server adress stucture */
unsigned short servPort;
const int BUFFSIZE = 40;
char buffer[BUFFSIZE];

void dieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

void createTCPSendSocket(){
    printf("%s\n", "creating TCP send sock");
 
    

    // Create stream socket using TCP
    servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (servSock < 0)
        dieWithError("socket() failed");
    
    // Construct the server address structure
    memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
    servAddr.sin_family = AF_INET; // IPv4 address family
    servAddr.sin_addr.s_addr = inet_addr("127.0.0.1");   /* Server IP address */
    servAddr.sin_port        = htons(servPort); /* Server port */
}

void sendMsgToServ(char* str){
    
    
    memset(buffer, '\0', 40);
    strcat(buffer, str);
    //Attempt connection to serv
    if (connect(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0)
        dieWithError("connect() failed");
    printf("%s\n", "Connected");
    size_t stringLen = strlen(buffer); // Input length
    
    // Send the string to the server
    ssize_t numBytes = send(servSock, buffer, stringLen, 0);
    
    if (numBytes < 0)
        dieWithError("send() failed");
    else if (numBytes != stringLen)
        dieWithError("send() sent different number of bytes than expected");
}

void rcvMsg(){
    
    int recvMsgSize = 1;
    
    memset(buffer, '\0', strlen(buffer));
    
    if ((recvMsgSize = recv(servSock, buffer, BUFFSIZE, 0)) < 0)
        dieWithError("recv() failed");
    
        /* receive again until end of transmission */
        while (recvMsgSize > 0 )      /* zero indicates end of transmission */
        {
            
            printf("%s\n",buffer);
            //inBuffer[recvMsgSize] = '\0';  /* Terminate the string! */
            //memset(buffer,'\0',sizeof(buffer));
            
            /* See if there is more data to receive */
            if ((recvMsgSize = recv(servSock, buffer, BUFFSIZE, 0)) < 0)
                dieWithError("recv() failed");
        }
}

bool login(){
    int counter;
    char username[12];
    //bool loggedIn = false;
    while(true){
        printf("Please login with your username: \n");
        scanf("%s",username);
        
        //send username to the server
        sendMsgToServ(username);
        
        //recieve from the server
        rcvMsg();
        
        if(strcmp(buffer, "success") ==0){
            return true;
        } else {//else reenter the username
            printf("Error: Invalid Username!\n");
            printf("%s", buffer);
            memset(username,'\0',strlen(username));
            
            counter = 0;
        }
    }
}



int main(int argc, const char * argv[]) {
    
    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }
    
    servPort = atoi(argv[1]);
    
    createTCPSendSocket();
    bool loggedIn = login();
 
    /* Receive the user info*/
    
    while(loggedIn){
        //print the menu
        //printMenu();
    }
}
