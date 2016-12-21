
//Author: Isaiah Egan
//Written for UCSB course CS176A, F16, with Prof. Beldwin
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>

#define BUFLEN 512


//for any critical error
void diep(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char **argv)
{
    struct sockaddr_in serveraddr;
    int sock, i, port, times[BUFLEN], n, numrecv, diff, sum, min, max;
    socklen_t slen;
    double avg, percloss;
    char ping[BUFLEN], *hostname;
    struct hostent *server;
    time_t seconds;
    struct timeval tv, tvE, tvB;

    if (argc !=3)
        diep("Usage error");
     
    hostname = argv[1];
    port = atoi(argv[2]);
    
    //set socket, get hostname
    if ((sock=socket(AF_INET, SOCK_DGRAM, 0))==-1)
        diep("opening socket");
    server = gethostbyname(hostname);
    if (server == NULL) 
        diep("retrieving IP");
        
    //DN resonlution
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(port);
    slen=sizeof(serveraddr);
    
    //set all useful vars
    numrecv = 0;
    sum = 0;
    min = -999;
    max = -999;
    
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    int timeend;
    int timebeg;

    
    for (i=0; i<10; i++) 
    {
        tvE.tv_sec = 0;
        tvE.tv_usec = 0;
        tvB.tv_sec = 0;
        tvB.tv_usec = 0;
        timeend = timebeg = 0;
        bzero(ping, BUFLEN);
        
        seconds = time(NULL);
        
        //write the datagram and set beginning time
        snprintf(ping, sizeof(ping), "PING #%i %s", i, asctime(localtime(&seconds)));
        gettimeofday(&tvB, NULL);
        //send datagram and start timeout
        if (sendto(sock, ping, BUFLEN, 0, (struct sockaddr* ) &serveraddr, slen)==-1)
            diep("sendto()");
        if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
            diep("Setting timeout");
        //check reply
        n = recvfrom(sock, ping, BUFLEN, 0, (struct sockaddr* ) &serveraddr, &slen);
        if(n == -1)
            printf("%s\n", "Timeout reached");
        else if (atoi(&ping[6]) == i)
        {  
            numrecv++;
            gettimeofday(&tvE, NULL);
            timebeg = ((((unsigned long long)tvB.tv_sec)*1000+((unsigned long long)tvB.tv_usec/1000)));
            timeend = ((((unsigned long long)tvE.tv_sec)*1000)+(((unsigned long long)tvE.tv_usec)/1000));
            diff = timeend-timebeg;
            printf("PING received from %s: seq#=%d time=%d ms\n", hostname, i, diff);
            if (min == -999 || diff<min) min = diff;
            if (max == -999 || diff>max) max = diff;
            sum+=diff;
        }
        
    }

    //caclulate stats
    avg = ((double)sum)/numrecv;
    double decloss = ((((double)(i - numrecv)/numrecv)));
    percloss = decloss*100;
    printf("--- ping statistics --- %d packets transmitted, %d received, %f%% packet loss rtt min/avg/max = %d %f %d ms\n",
     i, numrecv, percloss, min, avg, max);
    
    close(sock);
    return 0;
}