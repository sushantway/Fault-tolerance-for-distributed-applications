/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

struct records{
    int accnt_num;
    float accnt_bal;
};
struct records rec[1000];
int global_counter = 0;
int number = 100;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
struct sockaddr_in serv_addr, cli_addr;
void * connection_handler(void *);

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    while(1)
    {
     int sockfd, newsockfd, portno;
     int *new_sock;
     socklen_t clilen;
     int n;
     if (argc < 2) {
         fprintf(stderr,"\nERROR, no port provided\n");
         exit(1);
     }
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     char const* const filename = argv[2];
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);

     while(newsockfd = accept(sockfd,(struct sockaddr *) &cli_addr, &clilen))
     {
     if (newsockfd < 0) 
          error("\nERROR on accept");
        new_sock = (int *)malloc(1);   
        *new_sock = newsockfd;
         pthread_t pid;
         if(pthread_create(&pid,NULL,connection_handler,(void*)new_sock) < 0)    
         {
            perror("No thread created");
            return 1;
         }

     }
     //close(sockfd);
     return(0);
 }
 }

 void * connection_handler(void *socknewfd)
{
    char buffer[256];
     char temp[256];
     char temp1[256];
    int newsockfd = *(int *)socknewfd;
    int n; 
        int token_acc = 0;
     int token_bal = 0;
     int update_flag = 0;
     int query_flag = 0;
     int create_flag = 0;
     int GLOBAL_COMMIT = 0;
     int GLOBAL_ABORT = 0;
        //Waiting for vote-request
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("\nERROR reading from socket");
    // printf("\nHere is the message: %s\n",buffer);
        strncpy(temp,buffer,strlen(buffer));
     char * token = strtok(temp," ");
     int iter = 0;
     printf("\n---------------------------Start of transaction--------------------");
     printf("\nlog: Init");
     while(token)
     {
        //printf("\ntoken:%s",token);
        int leng = strlen(token);
        if((strcmp(token,"CREATE") == 0) || (strcmp(token,"create") == 0))
        {
            create_flag = 1;
            //printf("\nThe token is CREATE"); 

        }
        else if((strcmp(token,"QUERY") == 0) || (strcmp(token,"query") == 0))
        {
            query_flag = 1;
            //printf("\nThe token is QUERY");
        }
        else if((strcmp(token,"UPDATE") == 0) || (strcmp(token,"update") == 0))
        {
            update_flag = 1;
            //printf("\nThe token is UPDATE");
            //printf("\nLength:%d",leng);
        }   
        else if((strcmp(token,"QUIT") == 0) || (strcmp(token,"quit") == 0))
        {
            //printf("\nThe token is QUIT");
        }
        if(create_flag == 1 && iter == 1)
        {
            token_bal = atoi(token);
        }
        if(query_flag == 1 && iter == 1)
        {
            token_acc = atoi(token);
        } 
        if(update_flag == 1 && iter == 1)
        {
            token_acc = atoi(token);
        }
        else if(update_flag == 1 && iter == 2)
        {
            token_bal = atoi(token);
        }  
        token = strtok(NULL," ");
        iter++;
     }
     
     //printf("Sending commit back to front end process");
     //sleep(2);
     n = write(newsockfd,"\nI got your message",19);
     if (n < 0) error("\nERROR writing to socket");
     //printf("\nVote sent");
     printf("\nlog: Vote_Commit");
     //printf("\nEntered Ready State");
     //sleep(5);
     //waiting for commit or abort
     bzero(buffer,256);
     n = read(newsockfd,buffer,255);
     if (n < 0) error("\nERROR reading from socket");
     int len = strlen(buffer);
     //printf("\nBuffer length is %d",len);
     //printf("\nHere is the message: %s",buffer);
     //printf("\nChecking if global commit or abort");
     char * token1 = strtok(buffer," ");
     while(token1)
     {
        //printf("\ntoken1:%s",token1);
        if(strcmp(token1,"COMMIT") == 0)
        {
            //printf("\nGlobal commit set to 1");
            GLOBAL_COMMIT = 1;
            break;
        }
        else if(strcmp(token1,"ABORT") == 0)
        {
            //printf("\nNo commit");
            GLOBAL_ABORT = 1;
            break;
        }
        token1 = strtok(NULL," ");
     }
     //printf("\nupdate_flag:%d",update_flag);
     //printf("\ncreate_flag:%d",create_flag);
     //printf("\nquery_flag:%d",query_flag);
     //sleep(2);
     if(GLOBAL_COMMIT == 1)
     {
        printf("\nlog: GLOBAL_COMMIT");
         printf("\n---------------------------------End of transaction--------------------------------");
        global_counter++;
        //printf("\nupdate_flag:%d",update_flag);
        if(update_flag == 1)
        {
            //write update logic
            //printf("\nInside update");
            int rec_found = 0;
            for(int k = 0;k<global_counter;k++)
            {
                if(rec[k].accnt_num == token_acc)
                {
                    rec_found = 1;
                    pthread_mutex_lock(&mutex);
                    rec[k].accnt_bal = token_bal;
                    pthread_mutex_unlock(&mutex);
                    //printf("\nRecords updated rec[k].bal:%.02f",rec[k].accnt_bal);
                    bzero(temp1,256);
                    sprintf(temp1,"%.02f",rec[k].accnt_bal);
                    //printf("\ntemp1:%s",temp1);
                    n = write(newsockfd,temp1,strlen(temp1));
                    if (n < 0)
                    {
                        error("ERROR writing to socket");
                    }
                    break;
                }

            }
            if(rec_found == 0)
            {
                    bzero(temp1,256);
                    sprintf(temp1,"ERR Account %d does not exist",token_acc);
                    //printf("\ntemp1:%s",temp1);
                    n = write(newsockfd,temp1,strlen(temp1));
                    if (n < 0)
                    {
                        error("ERROR writing to socket");
                    }
            }

        }        
        else if(create_flag == 1)
        {
            //write create logic
            //printf("\nInside create");
            rec[global_counter].accnt_num = number;
            rec[global_counter].accnt_bal = token_bal;
            //printf("\nRecord created rec[].acc:%d",rec[global_counter].accnt_num);
            //printf("\nRecord created rec[].bal:%.02f",rec[global_counter].accnt_bal);
            bzero(temp1,256);
            sprintf(temp1,"%d",rec[global_counter].accnt_num);
            //printf("\ntemp1:%s",temp1);
            number++;
            global_counter++;
            n = write(newsockfd,temp1,strlen(temp1));
            if (n < 0)
            {
                error("ERROR writing to socket");
            }

        }
        else if(query_flag == 1)
        {
            //write query logic
            //printf("\nInside query");
            int rec_found = 0;
            for(int k = 0;k<global_counter;k++)
            {
                if(rec[k].accnt_num == token_acc)
                {
                    rec_found = 1;
                    //printf("\nBalance is rec[k].bal:%.02f",rec[k].accnt_bal);
                    bzero(temp1,256);
                    sprintf(temp1,"%.02f",rec[k].accnt_bal);
                    //printf("\ntemp1:%s",temp1);
                    n = write(newsockfd,temp1,strlen(temp1));
                    if (n < 0)
                    {
                        error("ERROR writing to socket");
                    }
                    break;
                }

            }
            if(rec_found == 0)
            {
                    bzero(temp1,256);
                    sprintf(temp1,"ERR Account %d does not exist",token_acc);
                    //printf("\ntemp1:%s",temp1);
                    n = write(newsockfd,temp1,strlen(temp1));
                   if (n < 0)
                    {
                        error("ERROR writing to socket");
                    }
            }
        }
        //printf("\nupdate_flag:%d",update_flag);
     }
     else
     {
        //printf("\nGLOBAL_ABORT");
        printf("\nlog: GLOBAL_ABORT");
         printf("\n---------------------------------End of transaction--------------------------------");
     }
close(newsockfd);
}
