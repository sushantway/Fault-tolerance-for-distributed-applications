#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h>
#include <errno.h>
#include <fcntl.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}
#define SENDER_PORT_NUM1 8081 
#define SENDER_PORT_NUM2 8089 
#define SENDER_PORT_NUM3 8083 
#define SERVER_IP1 "127.0.0.1" 
#define SERVER_IP2 "127.0.0.2" 
#define SERVER_IP3 "127.0.0.3" 
#define SENDER_IP_ADDR "127.0.0.1"
//int server[3];
int portarray[3];

void *processrequests();
pthread_mutex_t mutex;
pthread_t tid[2];
static int clientcount=0,numrequests=0,servertime=rand() % 25,flag=0;
static int ts=rand() % 10;
static int GS = 0;
int portno;
int portno1;
int portno2;
static int flag1;
socklen_t clilen;     
struct hostent *server[3];
void * connection_handler(void *);
struct sockaddr_in serv_addr, cli_addr;

int main(int argc, char *argv[])
{ 
    int *new_sock;
    FILE *output;
    portno = atoi(argv[1]);
    portarray[0] = SENDER_PORT_NUM1;
    portarray[1] = SENDER_PORT_NUM2;
    portarray[2] = SENDER_PORT_NUM3;
    server[0] = gethostbyname("127.0.0.1");
    server[1] = gethostbyname("127.0.0.2");
    server[2] = gethostbyname("127.0.0.3");
    int sockfd, newsockfd,sockfd1;
    socklen_t clilen;
    //char buffer[256];
         //bzero(buffer,256);
         
    int n;
     //portno = 9799;
    
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     //portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     //printf("Port no. is:%d\n",portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
          //printf("\nBefore listen" );
     int lis = listen(sockfd,5);
     //printf("\nLis:%d",lis);
     //printf("\nAfter listen");
     clilen = sizeof(cli_addr);

     //creating array of socket fd's for servers
         

        //sleep(3);
     while(newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen)){
        //printf("\nConnection accepted");
        new_sock = (int *)malloc(1);   
        *new_sock = newsockfd;
         pthread_t pid;
         if(pthread_create(&pid,NULL,connection_handler,(void*)new_sock) < 0)    
         {
            perror("No thread created");
            return 1;
         }

        //printf("handler assigned\n");
     }// end of while 
     close(sockfd);
     return 0; 
 }


 void * connection_handler(void *socknewfd)
{
    char buffer[256];
    char buffer1[256];
    char temp[256];
    char msgbuffer[256];
    char buffer_abort[256];
    int sockfdarray[3];
    int workingports[3];
    int votereceived[3];
    int newsockfd = *(int *)socknewfd;
    int n;
    
    //printf("\nsocket file descriptors created");
    while(1)
     {
        
        int commit_count = 0;
    int abort_count = 0;
        //printf("\nWaiting for client request");
    bzero(buffer,256);
     n = read(newsockfd,buffer,256);
     if (n < 0) error("ERROR reading from socket");
     strncpy(temp,buffer,strlen(buffer));
     char * token = strtok(temp," ");
     //printf("\nReceived message:%s",buffer);
     //char* buff; 
     int workingports[3];
        for(int f = 0;f<3;f++)
        {
            workingports[f] = 0;
            votereceived[f] = 0;
        }
     int alive = 0;
    for(int y = 0;y<3;y++)
    {
        sockfdarray[y] = socket(AF_INET, SOCK_STREAM, 0);
        //printf("\n%d",sockfdarray[y]);
        if (sockfdarray[y] < 0) 
            error("ERROR opening socket");
        //server[y] = gethostbyname("127.0.0.1");
        if (server[y] == NULL) {
            fprintf(stderr,"ERROR, no such host\n");
            exit(0);
            }   
        bzero((char *) &serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server[y]->h_addr,(char *)&serv_addr.sin_addr.s_addr,server[y]->h_length);
        serv_addr.sin_port = htons(portarray[y]);
        if (connect(sockfdarray[y],(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        {
          printf("Server is down %d",y);
        }    
        else
        {
          alive++;
          workingports[y] = 1;
        }
        //printf("\n%d",alive);
        //workingports[y] = 0;
    }
    //printf("\nalive:%d",alive);
     printf("\n---------------------------Start of transaction--------------------");                  
    printf("\nlog: START_2PC");
    //fprintf(fptr,"START_2PC");
    //fclose(fptr);
    //printf("\nFail now");
    //sleep(5);
    for(int f = 0;f<3;f++)
    {
        //bzero(buffer,256);
        
        if(workingports[f] == 1)
        {
          printf("\nSending vote request to server");
          n = write(sockfdarray[f],buffer,strlen(buffer));
          if (n < 0)
          {
            printf("ERROR writing to socket");
          }
        }
    }
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    
    for(int h = 0;h<3;h++)
    {
         bzero(buffer1,256);
         int err_flag = 0;
         //printf("\nFail the server");
        if(setsockopt(sockfdarray[h],SOL_SOCKET,SO_RCVTIMEO,(char*)&timeout,sizeof(timeout))<0)
        {
            printf("\nTimeout failed");
            exit(1);
        }
        while(recv(sockfdarray[h],buffer1,256,0)>0)
        {
            commit_count++;
            votereceived[h] = 1;
        }
       
        //printf("\nCommit-count:%d",commit_count);
        //printf("\nHere is the message: %s",buffer1);
    }
    //printf("\nFail now");
    //sleep(5);
    //printf("\ncommit count:%d",commit_count);
    //printf("\nalive:%d",alive);
    if(commit_count == alive)
    {
        //Sending global commit
        for(int h = 0;h<3;h++)
        {
          if(workingports[h] == 1)
          {
             n = write(sockfdarray[h],"COMMIT GLOBAL",13);
             if (n < 0) error("\nERROR writing to socket");
          }
           
        }
        printf("\nlog: Global_Commit");
        for(int h = 0;h<3;h++)
        {
            if(workingports[h] == 1)
            {
              bzero(buffer1,256);
              n = read(sockfdarray[h],buffer1,256);
              if (n < 0) error("ERROR reading from socket");
            }
        }
         //printf("\nSending response to client %s",buffer1);
         n = write(newsockfd,buffer1,strlen(buffer1));
         if (n < 0)
         {
            error("ERROR writing to socket");
         }
         printf("\n---------------------------------End of transaction--------------------------------");
    }
    else
    {
        //sending global abort
        //printf("\nInside abort");
        int r;
        n = write(newsockfd,"ERR",3);
        if (n < 0)
        {
            error("ERROR writing to socket");
        }
        sprintf(buffer_abort,"ABORT GLOBAL");
        printf("\nlog: Global Abort");
        for(int f = 0;f<3;f++)
        {
          //bzero(buffer,256);
          if(votereceived[f] == 1)
          {
              printf("\nSending Global Abort");
              n = write(sockfdarray[f],buffer,strlen(buffer));
              if (n < 0)
              {
                error("ERROR writing to socket");
              }
          }
        }
       printf("\n---------------------------------End of transaction--------------------------------");  
    }
    for(int i = 0;i<3;i++)
    {
      close(sockfdarray[i]);
    }
   
     }
}

