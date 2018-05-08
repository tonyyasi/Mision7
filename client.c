#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <sys/ioctl.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}
void gotoxy(int x,int y){
    printf("%c[%d;%df",0x1B,y,x);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    int bytesReceived = 0;

    char recvBuff[1024];
    memset(recvBuff, '0', sizeof(recvBuff));

    char buffer[256];
    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(portno);
    
    char ip[50];
    strcpy(ip,argv[1]);
    printf("IP: %s\n", ip);

    serv_addr.sin_addr.s_addr = inet_addr(ip);

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr))<0){
        printf("\n Error : Connect Failed \n");
        return 1;
    }

    //printf("Connected to ip: %s : %d\n",inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
    FILE *fp;
    char fname[100];
    while(1){
        memset(fname, 0, 100);
        read(sockfd, fname, 256);
        strncpy(fname,fname+12,strlen(fname));
        printf("File Name: %s\n",fname);
        printf("Receiving file...");
        fp = fopen(fname, "w"); 
        if(NULL == fp){
            printf("Connection ended");
            return 1;
        }
        long double sz=1;
        //Receive data in chunks of 256 bytes
        while((bytesReceived = read(sockfd, recvBuff, 1024)) > 0 && strcmp(recvBuff,"END") != 0)
        { 
            sz++;
            gotoxy(0,4);
            printf("Received: %llf Mb",(sz/1024));
            fflush(stdout);
            fwrite(recvBuff, 1,bytesReceived,fp);
        }

        if(bytesReceived < 0){
            printf("\n Read Error \n");
        }

        printf("\nFile OK....Completed\n");
        fclose(fp);
    }

    close(sockfd);
    return 0;
}