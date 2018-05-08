#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

struct sockaddr_in cli_addr;
char fname[100];

void sendFileToClient(int newsockfd) {
    write(newsockfd, fname,256);

    FILE *fp = fopen(fname,"rb");
    if(fp==NULL){
        printf("File open error");
        perror("Error");
        exit(1);   
    }   

    //Read data from file and send it
    while(1){
        //First read file in chunks of 256 bytes
        unsigned char buff[1024]={0};
        int nread = fread(buff,1,1024,fp);

        //If read was success, send data.
        if(nread > 0){
            write(newsockfd, buff, nread);
        }
        if (nread < 1024){
            if (feof(fp)){
                printf("End of file %s\n",fname);
                printf("File transfer completed for id: %d\n",newsockfd);
            }
            if (ferror(fp))
                printf("Error reading\n");
            break;
        }
    }
    fclose(fp);
}

void SendAllFilesToClient(int arg) {
    //int newsockfd=(int)*arg;
    int newsockfd=arg;
    printf("Connection accepted and id: %d\n",newsockfd);
    printf("Connected to Client: %s:%d\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));
    FILE *fp;
    char path[100];

    fp = popen("ls ../Examples/*", "r");

    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        memset(fname, 0, 100);
        strncpy(fname,path,strlen(path)-1);
        printf("%s\n",fname);
        sendFileToClient(newsockfd);
        sleep(1);
        write(newsockfd, "END",256);
        sleep(1);
    }

    pclose(fp);
    
    //Script to watch path
    fp = popen("fswatch -o ~/Documents/progra/Examples | xargs -n1 ~/Documents/progra/Mision7/script.sh", "r");

    if (fp == NULL) {
        printf("Failed to run command\n" );
        exit(1);
    }
    while (fgets(path, sizeof(path)-1, fp) != NULL) {
        memset(fname, 0, 100);
        strncpy(fname,path,strlen(path)-1);
        sendFileToClient(newsockfd);
        sleep(1);
        write(newsockfd, "END",256);
        sleep(1);
    }

    pclose(fp);

        //When new file
        //senfFileToClient(newsockfd);

    printf("Closing Connection for id: %d\n",newsockfd);
    close(newsockfd);
    shutdown(newsockfd,SHUT_WR);
    sleep(2);
}

int main(int argc, char *argv[])
{
    int sockfd, err,newsockfd, portno;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr;
    pthread_t tid;
    
    if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
    
    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    
    if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
    listen(sockfd,10);
    newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr,&clilen);
    SendAllFilesToClient(newsockfd);
    // while(1){
    //     clilen=sizeof(cli_addr);
    //     printf("Waiting...\n");
    //     newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr,&clilen);
    //     if(newsockfd<0) {
    //           printf("Error in accept\n");
    //           continue; 
    //     }
    //     err = pthread_create(&tid, NULL, &SendAllFilesToClient, &newsockfd);
    //     if (err != 0)
    //         printf("\ncan't create thread :[%s]", strerror(err));
    // }
    //bzero(buffer,256);
    //n = read(newsockfd,buffer,255);
    //if (n < 0) error("ERROR reading from socket");
    //printf("Here is the message: %s\n",buffer);
    //n = write(newsockfd,"I got your message",18);
    //if (n < 0) error("ERROR writing to socket");
    close(newsockfd);
    close(sockfd);
     return 0; 
}