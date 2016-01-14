//
//  Struct CLient.c
//  Struct Client Tester
//
//  Created by Julian on 10/19/14.
//  Copyright (c) 2014 Julian. All rights reserved.
//

#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */

int sock;                        /* Socket descriptor */
struct sockaddr_in ServAddr;     /*server address */
struct sockaddr_in fromAddr;     /* Source address*/
unsigned short servPort;         /*server port */
unsigned int fromSize;           /* In-out of address size for recvfrom() */
const char *servIP;              /* IP address of server */
int sendStructSize;              /* number of expected bytes in the struct*/
int resvStringLen;               /* Length of received response */

// enum types for sending data to server
typedef enum {ALL, PREMIUM , ECONOMY} SeatType; //Enum for Type of seat
typedef enum {CAP, AVAILABLE, PURCHASE} Option; //Enum for option
typedef enum {FAIL, SUCCESS, NO_SEAT} Status;   //Fail if flight not found, No Seat if attempt to purchase more than available
                                                //otherwise success.
typedef enum {TRUE, FALSE} Close;               //Lets the server know to exit the program.


//struct for client to server message;
typedef struct {
    char flightID[6];
    SeatType seat;
    Option op;
    int numberOfSeats;
    Close quit;
} clientRequest;

//struct for server to client message;
typedef struct{
    char flightID[6];
    Status stat;
    int numberOfSeats;
}   serverResponse;

//buffer struct from client
clientRequest msgToServ;
//buffer for struct to client
serverResponse msgToClient;

//Error message for failure
void dieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

// Sends the struct conatained in msgToServ to the server.
void sendMessage(){
    printf("\n%s\n", "Sending message to server...");
    if(msgToServ.quit == TRUE){
        printf("Stop Server ");
    }
    else{
    printf("%s%s\n", "FlightID: ", msgToServ.flightID);
    
    char *temp;
    switch (msgToServ.op) {
        case 0:
            temp = "check capacity";
            break;
        case 1:
            temp = "check availability";
            break;
        case 2:
            temp = "purchase";
            break;
        default:
            printf("error bad option");
            break;
    }
    printf("%s%s","Option: ", temp);
    memset(&temp, '\0', sizeof(temp));
    
    switch(msgToServ.seat){
        case 0: printf("%s\n"," of all seats.");
            break;
        case 1: printf("%s\n"," of premium seats.");
            break;
        case 2: printf("%s\n"," of economy seats.");
            break;
        default: printf("Error bad seat type");

    }
    printf("");
    fflush(stdout);
    }
    
    sendStructSize = sizeof(msgToServ);
    
    /* Send the struct to the server */
    if (sendto(sock, (struct clientRequest *)&msgToServ, sendStructSize, 0, (struct sockaddr *)
               &ServAddr, sizeof(ServAddr)) != sendStructSize)
        dieWithError("sendto() sent a different number of bytes than expected");
    
}
// receives the message from the client and store it in msgToClient
void recieveMessage(){
    
    fromSize = sizeof(fromAddr);
    recvfrom(sock,(struct serverResponse *) &msgToClient, sizeof(serverResponse), 0,(struct sockaddr *) &fromAddr, &fromSize);
    
    
    if (ServAddr.sin_addr.s_addr != fromAddr.sin_addr.s_addr)
    {
        fprintf(stderr,"Error: received a packet from unknown source.\n");
        exit(1);
    }

}
//print the information forom
void printServerResponse(){
    printf("%s\n"," ");
    printf("%s\n", "Received Message...");
    if(msgToClient.stat == FAIL){
        printf("%s\n%s\n","Flight ID is not in file,","Flight does not exist...");
    }
    else if(msgToClient.stat == NO_SEAT){
        printf("%s%d%s\n","There are only ",msgToClient.numberOfSeats, " available.");
        printf("%s%d%s\n","Cannot purchase ", msgToServ.numberOfSeats, " seats...");
    }
    else{
    printf("%s%s\n", "Flight ID: ", msgToClient.flightID);
    printf("%s%d\n", "Number of seats remaining: ", msgToClient.numberOfSeats);
    }
    
}
//Display the contents on msgToServer
void dispMsgToServer(){
    printf("sending...");
    printf("%s",msgToServ.flightID);
    printf("%d",msgToServ.numberOfSeats);
    switch (msgToServ.op) {
        case 0:
            printf("Capacity\n");
            break;
        case 1:
            printf("Available\n");
            break;
        case 2:
            printf("Purchase\n");
            break;
        default:
            dieWithError("Error: Bad option!");
            break;
    }
    switch (msgToServ.seat) {
        case 0:
            printf("Capacity\n");
            break;
        case 1:
            printf("Available\n");
            break;
        case 2:
            printf("Purchase\n");
            break;
        default:
            dieWithError("Error: Bad option!");
            break;
    }
    
}
//takes input from user and fills msgToServer.
void mainMenu(){
    int option = -1;
    char inputBuffer[100];
    msgToServ.quit = FALSE;
    
    while (option != 0){
        
        option = -1;
        int subOption = -1;
        int seatNumber = -1;
        
        while(option != 0 && option != 1 && option != 2 && option != 3){
        printf("%s\n%s\n%s\n%s\n","1. Check flight capacity","2. Check seat availibity",
               "3. Purchase seats","0. Exit program");
            memset(inputBuffer, '\0', 100);
            memset(stdin, '\0', 10);
            fgets(inputBuffer, 99, stdin);
            sscanf(inputBuffer, "%d", &option);
            if(option != 0 && option != 1 && option != 2 && option != 3 && inputBuffer[0] != '\n'){
            printf("%s%d\n","Invalid option: ", option);
            }
        }

        if(option == 0){
            msgToServ.quit = TRUE;
            sendMessage();
            close(sock);
            exit(0);
            return;
        }
        else if(option != 0){
            char temp[20] = "0000000000";
            
            while (strlen(temp)!= 6) {
                
                memset(temp, '\0', 20);
                printf("%s\n", "Please Enter The 6 char Flight ID");
                scanf("%s", temp);
                if(strlen(temp)>6){
                    printf("%s\n","Flight Id must be exactly 6 chars long");
                }
            }
            
            
            strcpy(msgToServ.flightID,temp);
        
        }
        if (option == 1) {
            while(subOption!=1&&subOption!=2&&subOption!=3){
                memset(inputBuffer, '\0', 100);
                msgToServ.op = CAP;
                printf("%s\n%s\n%s\n%s\n%s\n", "Check flight capacity", "=================",
                   "1. All seats","2. Premium seats","3. Economy seats");
                memset(stdin, '\0', 10);
                fgets(inputBuffer, 99, stdin);
                sscanf(inputBuffer, "%d", &subOption);
                    if(subOption!=1&&subOption!=2&&subOption!=3&&inputBuffer[0]!='\n'){
                    printf("%s\n","Invalid option, please re enter.");
                    }
            }
            
        }
        else if (option == 2){
            while(subOption!=1&&subOption!=2&&subOption!=3){
                memset(inputBuffer, '\0', 100);
                msgToServ.op = AVAILABLE;
                printf("%s\n%s\n%s\n%s\n%s\n", "Check seat availibity", "=================",
                   "1. All seats","2. Premium seats","3. Economy seats");
                memset(stdin, '\0', 10);
                fgets(inputBuffer, 99, stdin);
                sscanf(inputBuffer, "%d", &subOption);
                    if(subOption!=1&&subOption!=2&&subOption!=3){
                    printf("%s\n","Invalid option, please re enter.");
                    }
            }
        }
        else if(option == 3){
            while(subOption!=1&&subOption!=2){
            memset(inputBuffer, '\0', 100);
            msgToServ.op = PURCHASE;
            printf("%s\n%s\n%s\n%s\n\n", "Purchase seats", "=================",
                   "1. Premium seats","2. Economy seats");
            memset(stdin, '\0', 10);
            fgets(inputBuffer, 99, stdin);
            sscanf(inputBuffer, "%d", &subOption);
            if (subOption != 1 && subOption != 2){
                printf("%s\n","Incorrect response must enter 1 or 2.");
                }
            }
            subOption = subOption + 1;
            printf("%s\n", "Enter number of seats: ");
            scanf("%d", &seatNumber);
            
            
        }
        else if (option!= 0){
            printf("%s\n", "invalid Option");
        }
        if (subOption != 1 && subOption != 2 && subOption != 3) {
            printf("%s\n", "Invalid SubOption");
        }
        
      
        else{
            switch (subOption) {
                case 1:
                     msgToServ.seat = ALL;
                    break;
                case 2:
                    msgToServ.seat = PREMIUM;
                    break;
                case 3:
                    msgToServ.seat = ECONOMY;
                    break;
                default:
                    dieWithError("Error: Invalid subOption.");
                    break;
            }
        }
        
        msgToServ.numberOfSeats = seatNumber;
        sendMessage();
        recieveMessage();
        printServerResponse();
        

    }
}


int main(int argc, char *argv[]) {
    
    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }
    
    servPort = atoi(argv[1]);
   
    
    /* Create a datagram/UDP socket */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        dieWithError("socket() failed");
    
    /* Construct the server address structure */
    memset(&ServAddr, 0, sizeof(ServAddr));         /* Zero out structure */
    ServAddr.sin_family = AF_INET;                      /* Internet addr family */
    ServAddr.sin_addr.s_addr = inet_addr("127.0.0.1");  /* Server IP address */
    ServAddr.sin_port   = htons(servPort);           /* Server port*/
    
    mainMenu();
 
    
    
    
    
    return 0;
}