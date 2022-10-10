#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <regex.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <math.h>
#include <errno.h>
#include <time.h>
#define uint32 int
#define uint8 char
#define int32 int
#define int16 short
#define uint16 short

//#include "wrtd-common.h"
#define PORT 5044

int main(int argc, char *argv[])
{

    struct wrtd_message {
        unsigned char hw_detect[3]; /* LXI */
        unsigned char domain;       /* 0 */
        unsigned char event_id[16];
        uint32 seq;
        uint32 ts_sec;
        uint32 ts_ns;
        uint16 ts_frac;
        uint16 ts_hi_sec;
        uint8 flags;
        uint8 zero[2];
        uint8 pad[1];
    } ;      




    struct wrtd_message  msgbuf;
    struct sockaddr_in     servaddr; 
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        perror("socket");
        return -1;
    }
    memset(&servaddr, 0, sizeof(servaddr)); 
        
    // Filling server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = inet_addr("224.0.23.159") ;
    
    msgbuf.hw_detect[0] = 'L';
    msgbuf.hw_detect[1] = 'X';
    msgbuf.hw_detect[2] = 'I';
    strcpy((char *)msgbuf.event_id,"ciccio");
    
    struct timespec desired_tp, cur_tp;
    if ((clock_gettime(CLOCK_REALTIME, &cur_tp)) != 0)
    {
        perror("clock_getime");
        return -1;
    }
    msgbuf.seq = 1;
    msgbuf.ts_sec = cur_tp.tv_sec + 10;
    msgbuf.ts_ns = cur_tp.tv_nsec;
    msgbuf.ts_frac = 0;
    msgbuf.ts_hi_sec = 0;

    if(sendto(fd, &msgbuf, sizeof(msgbuf), 
        MSG_CONFIRM, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr)) < 0)
    {
        perror("sendto");
        return -1;
    }

    return 0;
}