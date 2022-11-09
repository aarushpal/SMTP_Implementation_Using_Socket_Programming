#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <stdbool.h>

#define domain "server.smtp.com"



// Function to receive image from client 
int receive_image(int socket)
{
    int recvsize;
    int size; 
    int readsize;
    int writesize;
    int packetindex; 
    int status;
    char img_array[10240];
    FILE *fpimage;

    //Find the size of the image
    do{
        status = read(socket, &size, sizeof(int));
    }while(status<0);

    printf("RECEIVED :\nPacket is received!");
    //printf("Packet Size : %i\n",status);
    //printf("Image Size : %i\n",size);
    printf(" \n");

    char buffer[] = "Success!";

    do{
        status = write(socket, &buffer, sizeof(int));
    }while(status<0);

    printf("Reply is sent successfully!\n");
    printf(" \n");

    fpimage = fopen("received.jpeg", "w");

    if(fpimage == NULL) 
    {
        printf("Error occured while opening the Image file!\n");
        return -1; 
    }

    // creating a timeout for the socket 
    // timeout after 10 seconds
    // first argument indicates time in seconds
    // second argument indicates time in microseconds
    struct timeval timeout = {10,0};

    fd_set fd_s;
    int bufferfd;

    while(recvsize < size) {

        // clear the socket set
        FD_ZERO(&fd_s);
        // add socket to set
        FD_SET(socket,&fd_s);

        /*The select() call monitors activity on a set of sockets looking for sockets
        ready for reading, writing, or with an exception condition pending.*/

        /* int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
        struct timeval *timeout)
         nfds is the highest-numbered file descriptor in any of the three sets, plus 1
         readfds is a pointer to a set of sockets to be checked for readability
         writefds is a pointer to a set of sockets to be checked for writability
         exceptfds is a pointer to a set of sockets to be checked for errors
         timeout is a pointer to a struct timeval that specifies the maximum amount of time to wait*/

        bufferfd = select(FD_SETSIZE,&fd_s,NULL,NULL,&timeout);
        
        if (bufferfd < 0)
            printf("Error occured due to bad file descriptor set.\n");

        if (bufferfd == 0)
            printf("Error occured due to buffer read timeout expired.\n");    
        
        if(bufferfd > 0)
        {
            do{
                readsize = read(socket, img_array, 10240);
            }while(readsize < 0);

            writesize = fwrite(img_array,1,readsize, fpimage);
            printf("Image size written : %i\n",writesize); 

            if(readsize!=writesize) 
            {
                printf("Error occured while reading!\n");    
            }

            recvsize += readsize;
            packetindex++;
            printf("Total Received Image size : %i\n",recvsize);
            printf(" \n");
        }
    }
}


int main(int argc, char *argv[])
{
    int sockfd, newsockfd, n, portno, clilen;
    struct sockaddr_in serv_addr, cli_addr;
    char servers[2][100] = {"smtp.gmail.com","smtp.yahoo.com"};
    char buff[10240];            //used to buffer the incoming data from client
    char command[50];           //used to read command RECEIVED from client

    // creating socket
    sockfd = socket(AF_INET, SOCK_STREAM,0);
    if(sockfd < 0)
    {
        perror("\nError occured while creating the socket!\n");
    }

    // initializing socket structure
    bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);
	

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{
		perror("\nSocket binding failed!\n");
	}

    // listening for client
    listen(sockfd,5);
    clilen = sizeof(cli_addr);
    // accepting client
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
    {
    	perror("\nError occured while accepting on socket!\n");
    }

    //Implementing responses which SMTP Server sends to Client for Acknowledgement in the process of Email Receiving
    do
    {
        // bzero is used to initialize buffer
        bzero(buff,10240);
        n = read(newsockfd,buff,10239);
        if (n < 0)
        {
            printf("\nError occured while reading from socket!\n");
            break;
        }
        else
        {
            buff[n] = '\0';
            
            // HELO command
            if(strstr(buff,"HELO")!=NULL)
            {
                printf("RECEIVED : %s",buff);
                bzero(buff,10240);
                strcpy(buff,"250 Hello ");
                strcat(buff, domain);
                printf("SENT : %s\n\n",buff);
                n = write(newsockfd,buff,strlen(buff));
                if (n<0) {  perror("Error occured while writing to socket!"); }
            }

            //MAIL FROM command
            else if(strstr(buff,"MAIL FROM")!=NULL)
            {
                printf("RECEIVED : %s",buff);
                bzero(buff,10240);
                strcpy(buff,"250 OK");
                printf("SENT : %s\n\n",buff);
                n = write(newsockfd,buff,strlen(buff));
                if (n<0) {  perror("Error occured while writing to socket!"); }
            }

            //RCPT TO command
            else if(strstr(buff,"RCPT TO")!=NULL)
            {
                printf("RECEIVED : %s",buff);
                bzero(buff,10240);
                strcpy(buff,"250 OK");
                printf("SENT : %s\n\n",buff);
                n = write(newsockfd,buff,strlen(buff));
                if (n<0) {  perror("Error occured while writing to socket!"); }
            }

            //DATA command
            else if(strstr(buff,"DATA")!=NULL)
            {
                printf("RECEIVED : %s",buff);
                bzero(buff,10240);
                strcpy(buff,"354 Send message content; end with <CRLF>.<CRLF>");
                printf("SENT : %s\n\n",buff);
                n = write(newsockfd,buff,strlen(buff));
                if (n < 0) {  perror("Error occured while writing to socket!"); }

                bzero(buff,10240);
                n = read(newsockfd,buff,10239);
                if (n < 0){ printf("\nError occured while reading from socket!\n"); break; }
                printf("\n\n----------| Received Email Header & Content |----------\n\n%s\n",buff);
                printf("-------------------------------------------------------\n\n");
                                        
                bzero(buff,10240);
                n = read(newsockfd,buff,10239);
                if (n < 0){ printf("\nError occured while reading from socket!\n"); break; }
            
                if(strstr(buff,".")!=NULL){
                    printf("RECEIVED : %s",buff);
                    bzero(buff,10240);
                  
                    strcpy(buff,"250 OK, message accepted for delivery.");
                    printf("SENT : %s\n\n",buff);
                    n = write(newsockfd,buff,strlen(buff));
                    if (n < 0) {  perror("Error occured while writing to socket!"); }
                }
            }

            //ATTACHMENT command
            else if(strstr(buff,"Attachment") != NULL)
            {
            	printf("RECEIVED : %s",buff);
		        bzero(buff,10240);
		        strcpy(buff,"420 Send attachment");
		        printf("SENT : %s\n\n",buff);
		        n = write(newsockfd,buff,strlen(buff));
		        if (n < 0) {  perror("Error occured while writing to socket!"); }

		        if(receive_image(newsockfd) != -1)
		        {
		            printf("Attached Image file received!\n");
		            bzero(buff,10240);
		            strcpy(buff,"250 Image received successfully");
		            printf("SENT : %s\n\n",buff);
		            n = write(newsockfd,buff,strlen(buff));
		            if (n < 0) {  perror("Error occured while writing to socket!"); }
		        } 
            }
            
            // QUIT COMMAND
            else if(strstr(buff,"QUIT") != NULL)
            {
                break;
            }   
        }
    }while(strcmp(buff,"QUIT") != 0);

    
    printf("RECEIVED : %s",buff);
    bzero(buff,10240);
    strcpy(buff,"221 Bye");
    printf("SENT : %s\n\n",buff);
    n = write(newsockfd,buff,strlen(buff));
    if (n<0) {  perror("Error occured while writing to socket!"); }
    printf("\nConnection closed successfully with the client!\n\n");

    return 0;
}
