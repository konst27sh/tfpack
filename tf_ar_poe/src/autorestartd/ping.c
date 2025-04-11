//
// Created by sheverdin on 12/13/23.
//

#include "ping.h"
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/ip_icmp.h>
#include <pthread.h>
#include <string.h>
#include "unistd.h"
#include "stdlib.h"
#include "stdio.h"
#include <errno.h>


typedef enum {
    PING_OK         = 0x10000,
    PING_ERR_TEST   = 0x00008
}ping_error;

typedef unsigned long int ulong;
typedef unsigned short int ushort;
typedef unsigned int uint;

#define PING_PKT_DATA_SZ    64
#define err_ok             (0)
#define err_failed        (-1)
#define err_timeout        (1)

typedef struct {
    struct icmp hdr;
    char data[PING_PKT_DATA_SZ];
}ping_pkt_t;

typedef struct {
    struct ip  ip_hdr;
    ping_pkt_t ping_pkt;
}ip_pkt_t;

static void   prepare_icmp_pkt  (ping_pkt_t *ping_pkt);
static ulong  get_cur_time_ms   ();
static ushort checksum          (void *b, int len);


int ping(port_config_info_t *portConfigInfo,  unsigned long timeout)
{
    ping_error pingResult = PING_ERR_TEST;

    ping_pkt_t ping_pkt;
    prepare_icmp_pkt(&ping_pkt);
    const uint16_t reply_id = ping_pkt.hdr.icmp_hun.ih_idseq.icd_id;

    struct sockaddr_in to_addr;
    to_addr.sin_family = AF_INET;
    if (!inet_aton(portConfigInfo->host, (struct in_addr*)&to_addr.sin_addr.s_addr)) {
        if (portConfigInfo->portNum == 6)
            printf("inet_aton -- port %d\n", portConfigInfo->portNum);
        return  PING_ERR_TEST;
    }

    if (!strcmp(portConfigInfo->host, "255.255.255.255") || to_addr.sin_addr.s_addr == 0xFFFFFFFF) {
        if (portConfigInfo->portNum == 6)
            printf("ping 2 %d\n", portConfigInfo->portNum);
        return PING_ERR_TEST;
    }

    errno = 0;
    portConfigInfo->sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (portConfigInfo->sock < 0) {
        if (portConfigInfo->portNum == 6)
            printf("socket() %s - port = %d\n", strerror(errno), portConfigInfo->portNum);
        return  PING_ERR_TEST;
    }

    const ulong start_time_ms = get_cur_time_ms();
    socklen_t socklen = sizeof(struct sockaddr_in);

    if (sendto(portConfigInfo->sock, &ping_pkt, sizeof(ping_pkt_t), 0, (struct sockaddr*)&to_addr, socklen) <= 0)
    {
        close(portConfigInfo->sock);
        if (portConfigInfo->portNum == 6)
            printf("ping 3 - port = %d\n", portConfigInfo->portNum);
        return PING_ERR_TEST;
    }


    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 500;

    if (portConfigInfo->portNum == 6)
        printf("tv.tv_sec = %ld - tv.tv_usec = %ld\n ", tv.tv_sec, tv.tv_usec);

    for (;;)
    {
        //printf("ping FOR start\n");
        FD_ZERO(&portConfigInfo->rfd);
        FD_SET(portConfigInfo->sock, &portConfigInfo->rfd);
        //printf("ping FOR 1\n");
        int n = select(portConfigInfo->sock + 1, &portConfigInfo->rfd, 0, 0, &tv);
        //if (portConfigInfo->portNum == 6)
        //    printf("ping FOR 2 -- port = %d\n", portConfigInfo->portNum);
        if (n == 0) {
            //if (portConfigInfo->portNum == 6)
            //    printf("ping 4- port = %d\n", portConfigInfo->portNum);
            portConfigInfo->pingResult = PING_ERR_TEST;
            break;
        }
        //printf("ping FOR 3\n");
        if (n < 0) {
            //if (portConfigInfo->portNum == 6)
            //    printf("ping 5 - port = %d\n", portConfigInfo->portNum);
            break;
        }
        //printf("ping FOR 4\n");
        const ulong elapsed_time = (get_cur_time_ms() - start_time_ms);
        if (elapsed_time > timeout) {
           //if (portConfigInfo->portNum == 6)
           //    printf("ping 6 - port = %d -- elapsed_time = %lu\n", portConfigInfo->portNum, elapsed_time);
            portConfigInfo->pingResult = PING_ERR_TEST;
            break;
        }
        else
        {
            //if (portConfigInfo->portNum == 6)
            //    printf("ping FOR 5\n");
            const unsigned long new_timeout = timeout - elapsed_time;
            tv.tv_sec = 1;
            tv.tv_usec = 500;
        }
        //printf("ping FOR 6\n");
        if (FD_ISSET(portConfigInfo->sock, &portConfigInfo->rfd)) {
            //if (portConfigInfo->portNum == 6)
            //    printf("ping FOR FD_ISSET start - port = %d\n", portConfigInfo->portNum);
            ip_pkt_t ip_pkt;
            struct sockaddr_in from_addr;
            socklen = sizeof(struct sockaddr_in);
            if (recvfrom(portConfigInfo->sock, &ip_pkt, sizeof(ip_pkt_t), 0, (struct sockaddr*)&from_addr, &socklen) <= 0)
            {
                //if (portConfigInfo->portNum == 6)
                //    printf("ping recvfrom - port = %d\n", portConfigInfo->portNum);
                break;
            }
            if (to_addr.sin_addr.s_addr == from_addr.sin_addr.s_addr
                && reply_id == ip_pkt.ping_pkt.hdr.icmp_hun.ih_idseq.icd_id)
            {
                if (portConfigInfo->portNum == 6  || portConfigInfo->portNum == 8)
                {
                    printf("PING_OK TO_addr = %u - FROM_addr = %u -- port = %d\n", (in_addr_t)to_addr.sin_addr.s_addr, (in_addr_t)from_addr.sin_addr.s_addr, portConfigInfo->portNum);
                    printf("PING_OK - reply_id = %d - icd_id = %d - port = %d\n", reply_id,  ip_pkt.ping_pkt.hdr.icmp_hun.ih_idseq.icd_id, portConfigInfo->portNum);
                }

                portConfigInfo->pingResult = PING_OK;
                break;
            }
            else
            {
                if (portConfigInfo->portNum == 6 || portConfigInfo->portNum == 8)
                {
                    printf("PING_ERR TO_addr = %u - FROM_addr = %u -- port = %d\n", (in_addr_t) to_addr.sin_addr.s_addr,
                           (in_addr_t) from_addr.sin_addr.s_addr, portConfigInfo->portNum);
                    printf("PING_ERR - reply_id = %d - icd_id = %d - port = %d\n", reply_id,
                           ip_pkt.ping_pkt.hdr.icmp_hun.ih_idseq.icd_id, portConfigInfo->portNum);
                }

            }
            //printf("ping FOR FD_ISSET end\n");
        }
        //printf("ping FOR end\n");
    }

    
    close(portConfigInfo->sock);
    if (portConfigInfo->portNum == 6)
        printf("ping Result = %X - port = %d\n", portConfigInfo->pingResult, portConfigInfo->portNum);
    return portConfigInfo->pingResult;
}

void sendPingRequest(port_config_info_t *portConfigInfo)
{
    char command[128];
    sprintf(command, "ping -c 3 %s", portConfigInfo->host);
    FILE *fp = popen(command, "r");
    if (fp == NULL) {
        printf("Error opening pipe!\n");
        return ;
    }

    char response[512];
    while (fgets(response, sizeof(response), fp) != NULL) {

        if (strstr(response, "bytes from") != NULL) {
            pclose(fp);
            portConfigInfo->pingResult = PING_OK;
            return;
        }
    }
    portConfigInfo->pingResult = PING_ERR_TEST;
    pclose(fp);
}


static void prepare_icmp_pkt(ping_pkt_t *ping_pkt)
{
    memset(ping_pkt, 0, sizeof(ping_pkt_t));

    int i = 0;
    for (; i < sizeof(ping_pkt->data) - 1; ++i) {
        ping_pkt->data[i] = i + 'a';
    }
    ping_pkt->data[i] = 0;

    srand(time(NULL));
    const int random_id = rand();
    ping_pkt->hdr.icmp_type = ICMP_ECHO;
    ping_pkt->hdr.icmp_hun.ih_idseq.icd_id = random_id;
    ping_pkt->hdr.icmp_hun.ih_idseq.icd_seq = 0;
    ping_pkt->hdr.icmp_cksum = checksum(ping_pkt, sizeof(ping_pkt_t));
}

static ulong get_cur_time_ms()
{
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);
    ulong time_ms = time.tv_sec * 1000 + (time.tv_nsec / 1000000);
    return time_ms;
}

static ushort checksum(void *b, int len)
{
    ushort *buf = b;
    uint sum = 0;
    ushort result;

    for (sum = 0; len > 1; len -= 2)
    {
        sum += *buf++;
    }
    if (len == 1)
    {
        sum += *(unsigned char *) buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}
