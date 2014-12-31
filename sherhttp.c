#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>

#define BOOL int
#define TRUE 1
#define FALSE 0


int nConCount = 0;

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

void sig_chld()
{
    pid_t pid;
    while((pid = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        printf("Child %d terminated\n", pid);
    }
}

BOOL InitSocket(int nPort, int nBackLog, int* sockfd, struct sockaddr_in* serv_addr)
{
    *sockfd = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr->sin_family = AF_INET;
    serv_addr->sin_addr.s_addr = INADDR_ANY;
    serv_addr->sin_port = htons(nPort);

    int nOne = 1;
    if ( setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&nOne, sizeof(int)) < 0)
    {
        error("setsockopt");
    }

    if ( bind(*sockfd, (struct sockaddr*)serv_addr, sizeof(struct sockaddr_in)) < 0 )
    {
        error("bind");
    }

    listen(*sockfd, nBackLog);

    signal(SIGCHLD, SIG_IGN);
    return TRUE;
}

void DoStuff(int fd, char *pReadBuffer, char *pWriteBuffer)
{
    printf("DoStuff accept a client:%d\n", nConCount++);
    bzero(pReadBuffer, 256);
    bzero(pWriteBuffer, 256);
    if( read(fd, pReadBuffer, 256) < 0)
    {
        error("error on reading");
    }
    printf("read from client:%s\nThen start do stuff...\n", pReadBuffer);
    sleep(5);
    sprintf(pWriteBuffer, "u r %d\n", nConCount-1);
    write(fd, pWriteBuffer, (int)strlen(pWriteBuffer));
    printf("finish doing stuff\n");
}


BOOL Loop(int sockfd, struct sockaddr_in* cli_addr, int clilen, char* buffer, char* writeBuf)
{
    printf("start a loop!\n");
    int fd = accept(sockfd, (struct sockaddr*)cli_addr, &clilen);
    if (fd < 0)
    {
        return FALSE;
    }
    if (fork() == 0)
    {
        close(sockfd);
        DoStuff(fd, buffer, writeBuf);
        close(fd);
        printf("End a loop in child process\n");
        exit(0);
    }

    close(fd);
}


int main()
{
    int sockfd, newsockfd;
    int clilen ;
    int nConCount = 0;
    struct sockaddr_in serv_addr, cli_addr;
    char buffer[256];
    char writeBuf[256];

    if( InitSocket(9000, 5, &sockfd, &serv_addr) != TRUE )
    {
        printf("initsocket error");
        return 0;
    }

    while(TRUE)
    {
        Loop(sockfd, &cli_addr, clilen, buffer, writeBuf);
    }

    close(sockfd);
    return 1;
}