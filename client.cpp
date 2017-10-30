#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n,intrst=0;
    float wait_time;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
    printf("\nOk");
    while(1)
    {   
        bzero(buffer,256);
        printf("\nPlease enter the message: ");
        fgets(buffer,255,stdin);
        
        char temp[256];
        int i = 0;
        while(buffer[i] != '\0')
        {
            //printf("\n%c",buffer[i]);
            temp[i] = buffer[i];
            i++;
        }
        i--;
        temp[i] = '\0';
        //printf("%d",i);
        //temp[i] = '\0';
        //int len = strlen(temp);
        //printf("\n%d",len);
       // printf("\n%s",temp);
        if(strcmp(temp,"QUIT") == 0 || strcmp(temp,"quit") == 0)
        {
            printf("\nOK\nConnection closed by foreign host\n");
            exit(1);
        }
        n = write(sockfd,buffer,strlen(buffer));
        if (n < 0)
        {
            error("ERROR writing to socket");
        }  
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) 
        {
            error("ERROR reading from socket");
        }
        if(strcmp(buffer,"ERR") == 0)
        {
            printf("\n%s",buffer);
        }
        else
        {
            printf("\nOK ");
            printf("%s",buffer);
        }
        
    }
    
    close(sockfd);
    return 0;
}
