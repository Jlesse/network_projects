#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#include <stdbool.h>    /* for boolean*/

/* Set up client socket to the server */
int servSock;
int clntSock;
struct sockaddr_in    servAddr;
struct sockaddr_in    clntAddr;
socklen_t clntAddrLen;
char servIP[20] = "127.0.0.1"; // Server IP address (dotted quad)
unsigned short servPort = 3000; //the port the server is listening on
int bytesRcvd, totalBytesRcvd; //bytes read in a single recv and total
const int MAXPENDING = 5;
typedef enum { CHECKING, SAVINGS} AccountType;
const int MAX = 200;
char buffer[MAX];


//current number of flights in flightData
int numberOfUsers = 0;




//Account data
typedef struct {
    int acountId;
    long balance;
    AccountType accType;
} Account;

//User data
typedef struct {
    char userId[12];
    Account checking[12];
    Account savings[12];
    int checkSize;
    int savingSize;
} UserInfo;

//contains all flight data
UserInfo userArray[100];

void dieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(1);
}

//checks if the userID is in userArray
int findUserID(char* userName){
    char temp[12];
    strcpy(temp,userName);
    for(int i = 0; i<=numberOfUsers ; i++){
        if (strcmp(temp,userArray[i].userId) == 0){
            return i;
        }
    }
    return -1;
}


//create the TCP socket for communcation with the client
void createTCPSocket(){
    printf("%s\n", "creating TCP send sock");
   
    
    
    
    // Create stream socket using TCP
    servSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (servSock < 0)
        dieWithError("socket() failed");
    
    // Construct the local address structure
    memset(&servAddr, 0, sizeof(servAddr)); // Zero out structure
    servAddr.sin_family = AF_INET; // IPv4 address family
    servAddr.sin_addr.s_addr = inet_addr(servIP);   /* Server IP address */
    servAddr.sin_port        = htons(servPort); /* Server port */
    
    // Bind to the local address
    if (bind(servSock, (struct sockaddr *) &servAddr, sizeof(servAddr)) < 0){
         dieWithError("bind() failed");
    }
    
     // Mark the socket so it will listen for incoming connections
    if (listen(servSock, MAXPENDING) < 0){
         dieWithError("listen() failed");
    }
}

void acceptMsgFromClient(){
    // Wait for a client to connect
    clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
    if (clntSock < 0)
        dieWithError("accept() failed");
}

void sendMsgToClient(char* str){
    char sendString[40];
    size_t stringLen = strlen(sendString); // Input length
    memset(sendString,'\0', stringLen);
    strcpy(sendString, str);
    
    
    // Send the string to the server
    ssize_t numBytes = send(clntSock, sendString, stringLen, 0);
    
    if (numBytes < 0)
        dieWithError("send() failed");
    else if (numBytes != stringLen)
        dieWithError("send() sent different number of bytes than expected");
}

//loads in txt file in to flight data array.
void loadInFile(){
    char *dLine = NULL;
    size_t dLen = 0;
    ssize_t read;
    numberOfUsers=0;
    
    FILE * fp;
    
    fp = fopen ("/Users/Julian/Desktop/ATMdata.txt", "r");
    if (fp == NULL) {
        fprintf(stderr, "Can't open input file");
        exit(1);
    }
    int accNum = 0;
    char token[20];
    memset(token, '\0', 20);
    while((read = getdelim(&dLine, &dLen, ' ', fp)) != -1){
        
        strtok(dLine, "\n");
        strcpy(userArray[numberOfUsers].userId, dLine);
        
        //printf("%s",dLine);
        
        
        char *p;
        
        //number of each account per user
        int numOfCheck = 0;
        int numOfSav = 0;
        
        p = (char*)strtok(NULL, "\n");
        memset(token, '\0', 20);
        
        //While p does not equal null or the delimiter.
        while(p != NULL && strcmp(p, " ") != 0){
        
             strcpy(token, p);
        
            
            
            
            
            //If a checking account
            if (strcmp (token, "CHK") == 0) {
                
                userArray[numberOfUsers].checking[numOfCheck].accType = CHECKING;
                
                userArray[numberOfUsers].checking[numOfCheck].acountId = accNum;
                p = (char*)strtok(NULL, "\n");
                memset(token, '\0', 20);
                strcpy(token,p);
                printf("%ld",strtol(token,NULL,10));
                fflush(stdout);
                
                //Assign Checking balance
                userArray[numberOfUsers].checking[numOfCheck].balance = strtol(token,NULL,10);
                
                accNum++;
                numOfCheck++;
                
            }
            
            // a savings account
            else if (strcmp (token, "SAV") == 0){
                
                userArray[numberOfUsers].savings[numOfSav].accType = SAVINGS;
                
                userArray[numberOfUsers].savings[numOfSav].acountId = accNum;
                
                p = (char*)strtok(NULL, "\n");
                memset(token, '\0', 20);
                strcpy(token,p);
                printf("%ld",strtol(token,NULL,10));
                fflush(stdout);
                
                //Assign savings balance
                userArray[numberOfUsers].savings[numOfSav].balance = strtol(token,NULL,10);
                
                accNum++;
                numOfSav++;
            }
            else{
                printf("%s","Error, Bad account type from file...");
            }
            p = (char*)strtok(NULL, "\n");
            memset(token, '\0', 20);
           
            
            
        }
        //increment the size marker for the array size of the checking and savings accounts
        userArray[numberOfUsers].checkSize = numOfCheck;
        userArray[numberOfUsers].savingSize = numOfSav;
        
        numberOfUsers++;
        
        }
    
    printf("\n%s\n", "Loaded FlightInfo");
    fclose(fp);
}

void recvMsg(){
    // receive username
    int end = recv(clntSock, buffer, MAX - 1, 0);
    buffer[end] = '\0';
}

void login(char* userID){
    int loggedIn = 0;
    
    char status[10];
    int statusLen=0;
    //int counter = 1;
    //while(!loggedIn){
    
    // check if username is in array
    //if in the list
    if(findUserID(userID) != -1){
        strcpy(status,"success");
        //statusLen = strlen(status);
        sendMsgToClient(status);
        //sendMsgToClient("%");
        
        loggedIn = true;
    }else { //if not in the list
        strcpy(status,"fail");
        // statusLen = strlen(status);
        sendMsgToClient(status);
        memset(status,0,strlen(status));
        // memset(Buffer,0,strlen(Buffer));
    }
    
}

void parseMsgFromClient(char* msg){
    char *dLine = msg;
    size_t dLen = 0;
    ssize_t read;
    
    char *p;
    char token[20];
    
    //tokenize
    strtok(dLine, "%");
    
    //if the message is a username
    if(strcmp(dLine, "1")==0){
        p = (char*)strtok(NULL, "%");
        memset(token, '\0', 20);
        strcpy(token, p);
        login(token);
        
        
    }
    //if the message is a transaction
    else if (strcmp(dLine, "2")==0){
        //get next token
        p = (char*)strtok(NULL, "%");
        
        //set accountAction
        char* accountAction;
        strcpy(accountAction,p);
        
        //get next token (username)
        p = (char*)strtok(NULL, "%");
        memset(token, '\0', 20);
        strcpy(token, p);
        
        UserInfo user = userArray[findUserID(token)];
        
        //get next token (account Type)
        p = (char*)strtok(NULL, "%");
        memset(token, '\0', 20);
        strcpy(token, p);
        
        char* accountType = token;
        
        //get next token (account number)
        p = (char*)strtok(NULL, "%");
        memset(token, '\0', 20);
        strcpy(token, p);
        
        int accountNum = atoi(token);
        
        
        
        //if a withdraw
        if (strcmp(accountAction, "w") == 0) {
            
            //get next token (amount)
            p = (char*)strtok(NULL, "%");
            memset(token, '\0', 20);
            strcpy(token, p);
            
            long amount = strtol(token,NULL,10);
            
            //modify user account
            //If a checking account
            if(strcmp(accountType, "CHK")==0){
                
                //find account
                for(int i; i < user.checkSize; i++)
                    
                    //if the account is found
                    if(user.checking[i].acountId == accountNum){
                        
                        //subtract the amount from the checking account
                        user.checking[i].balance -= amount;
                    }
                }
            else if (strcmp(accountType, "SAV")==0){
                //find account
                for(int i; i < user.savingSize; i++)
                    
                    //if the account is found
                    if(user.savings[i].acountId == accountNum){
                        
                        //subtract the amount from the savings account
                        user.savings[i].balance -= amount;
                    }
            
            }
        }
        
        //if a deposit
        else if(strcmp(accountAction, "d") == 0){
            //get next token (amount)
            p = (char*)strtok(NULL, "%");
            memset(token, '\0', 20);
            strcpy(token, p);
            
            long amount = strtol(token,NULL,10);
            
            //modify user account
            //If a checking account
            if(strcmp(accountType, "CHK")==0){
                
                //find account
                for(int i; i < user.checkSize; i++)
                    
                    //if the account is found
                    if(user.checking[i].acountId == accountNum){
                        
                        //add the amount to the checking account
                        user.checking[i].balance += amount;
                    }
            }
            else if (strcmp(accountType, "SAV")==0){
                //find account
                for(int i; i < user.savingSize; i++)
                    
                    //if the account is found
                    if(user.savings[i].acountId == accountNum){
                        
                        //add the amount to the savings account
                        user.savings[i].balance += amount;
                    }
                
            }
        }
        
        //if a balance inquiry
        else if(strcmp(accountAction, "b") == 0){
            //modify user account
            //If a checking account
            if(strcmp(accountType, "CHK")==0){
                
                //find account
                for(int i; i < user.checkSize; i++)
                    
                    //if the account is found
                    if(user.checking[i].acountId == accountNum){
                        
                        //send balance back to user
                        sprintf(buffer, "%ld", user.checking[i].balance);
                        sendMsgToClient(buffer);
                        memset(buffer, '\0', MAX);
                    }
            }
            else if (strcmp(accountType, "SAV")==0){
                //find account
                for(int i; i < user.savingSize; i++)
                    
                    //if the account is found
                    if(user.savings[i].acountId == accountNum){
                        
                        //send balance back to user
                        sprintf(buffer, "%ld", user.savings[i].balance);
                        sendMsgToClient(buffer);
                        memset(buffer, '\0', MAX);
                    }
                
            }
        }
        
        //if a transfer
        else if(strcmp(accountAction, "t") == 0){
            
            //get next token (amount)
            p = (char*)strtok(NULL, "%");
            memset(token, '\0', 20);
            strcpy(token, p);
            long amount = strtol(token,NULL,10);
            
            //if account being transferred from is a savings account
            if(strcmp(accountType, "CHK")==0){
                
                //find account
                for(int i; i < user.checkSize; i++)
                    
                    //if the account is found
                    if(user.checking[i].acountId == accountNum){
                        
                        //get account that is being transferred from
                        user.checking[i].balance -= amount;
                        
                    }
                }
            
            //if account being transferred from is a savings account
            else if (strcmp(accountType, "SAV")==0){
                //find account
                for(int i; i < user.savingSize; i++)
                    
                    //if the account is found
                    if(user.savings[i].acountId == accountNum){
                        
                        //send balance back to user
                        user.savings[i].balance -= amount;
                        
                    }
            }
           
            
            //get next token (account type to transfer to)
            p = (char*)strtok(NULL, "%");
            memset(token, '\0', 20);
            strcpy(token, p);
            accountType = token;
            
            //get next token (type of acount to transfer to)
            p = (char*)strtok(NULL, "%");
            memset(token, '\0', 20);
            strcpy(token, p);
            
            //if account being transferred to is a savings account
            if(strcmp(accountType, "CHK")==0){
                
                //find account
                for(int i; i < user.checkSize; i++)
                    
                    //if the account is found
                    if(user.checking[i].acountId == accountNum){
                        
                        //get account that is being transferred from
                        user.checking[i].balance += amount;
                        
                    }
            }
            
            //if account being transferred from to is a savings account
            else if (strcmp(accountType, "SAV")==0){
                //find account
                for(int i; i < user.savingSize; i++)
                    
                    //if the account is found
                    if(user.savings[i].acountId == accountNum){
                        
                        //send balance back to user
                        user.savings[i].balance += amount;
                        
                    }
            }
            
            
            
        }
        
        //else a tokenizer error
        else{
            dieWithError("parsed bad token");
        }
    }
    else{
        dieWithError("parsed bad option");
    }
}
            
            
            
    
       
        


int main(int argc, const char * argv[]){
    if (argc != 2)         /* Test for correct number of parameters */
    {
        fprintf(stderr,"Usage:  %s <UDP SERVER PORT>\n", argv[0]);
        exit(1);
    }
    servPort = atoi(argv[1]);
    
    loadInFile();
    
    createTCPSocket();
    
    acceptMsgFromClient();
    
    parseMsgFromClient("");
    
    //login();
    
    printf("handling client...\n");
    // while unsuccessful login
    
    close(clntSock);
   
    
        //receive response
        
        //handle repsonse
   while(true){


    }
}

