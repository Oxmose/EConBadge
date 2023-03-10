
#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define PORT 5000
#define SA struct sockaddr

#include "../../ImageConversion/test.cpp"

int main()
{
    int sockfd, connfd;
    struct sockaddr_in servaddr, cli;
    uint32_t i;
    uint32_t buff;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("192.168.0.1");
    servaddr.sin_port = htons(PORT);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
        != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");

    /* Ping */
    buff = 0;
    send(sockfd, &buff, sizeof(buff), 0);

    /* Update Image */
#if 0
    buff = 3;
    write(sockfd, &buff, sizeof(buff));
    for(i = 0; i < 134400 / 4096; ++i)
        write(sockfd, &image + i * 4096, 4096);
    if(134400 % 4096 != 0)
        write(sockfd, &image + i * 4096, 134400 % 4096);
#else
    buff = 3;
    send(sockfd, &buff, sizeof(buff), 0);
    send(sockfd, &Image7color, sizeof(Image7color), 0);
#endif

    /* Wait for ack */
    recv(sockfd, &buff, sizeof(buff), 0);

    // close the socket
    close(sockfd);
    return 0;
}