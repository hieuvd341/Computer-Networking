#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PACKETSIZE 64

// Function to calculate the checksum for ICMP
unsigned short checksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2) {
        sum += *buf++;
    }
    if (len == 1) {
        sum += *(unsigned char *)buf;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;

    return result;
}

int main() {
    int sockfd;
    char packet[PACKETSIZE];
    struct iphdr *ip = (struct iphdr *)packet;
    struct icmphdr *icmp = (struct icmphdr *)(packet + sizeof(struct ip));
    struct sockaddr_in addr;

    // Creating a raw socket
    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sockfd < 0) {
        perror("Socket Error");
        return 1;
    }

    // Set the IP address to send the packet to (Google's IP)
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("172.217.168.238"); // Google's IP address

    // Constructing the IP header
    ip->ihl = 5;
    ip->version = 4;
    ip->tos = 0;
    ip->tot_len = sizeof(struct iphdr) + sizeof(struct icmphdr);
    ip->id = htons(54321);
    ip->ttl = 64;
    ip->protocol = IPPROTO_ICMP;
    ip->saddr = inet_addr("0.0.0.0"); // Source IP address
    ip->daddr = addr.sin_addr.s_addr;

    // Constructing the ICMP header
    icmp->type = ICMP_ECHO;
    icmp->code = 0;
    icmp->un.echo.sequence = 0;
    icmp->un.echo.id = 0;
    icmp->checksum = 0;
    icmp->checksum = checksum(icmp, sizeof(struct icmphdr));

    // Sending the packet
    if (sendto(sockfd, packet, ip->tot_len, 0, (struct sockaddr *)&addr, sizeof(addr)) <= 0) {
        perror("Packet Sending Failed");
    } else {
        printf("Packet Sent Successfully\n");
    }

    close(sockfd);
    return 0;
}
