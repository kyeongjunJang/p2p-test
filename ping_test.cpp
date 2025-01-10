#include <arpa/inet.h>
#include <iostream>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PACKET_SIZE 32

// 체크섬 계산 함수
unsigned short calculateChecksum(unsigned short *buffer, int size)
{
    unsigned long sum = 0;
    while (size > 1)
    {
        sum += *buffer++;
        size -= sizeof(unsigned short);
    }
    if (size)
    {
        sum += *(unsigned char *)buffer;
    }
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

int main(int argc, char *argv[])
{
    // 권한 체크
    if (getuid() != 0)
    {
        std::cout << "This program requires root privileges. Please run with sudo." << std::endl;
        return 1;
    }

    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <ip_address>" << std::endl;
        return 1;
    }

    // Raw 소켓 생성
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock < 0)
    {
        std::cout << "Socket creation failed" << std::endl;
        return 1;
    }

    // 목적지 주소 설정
    struct sockaddr_in dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin_family = AF_INET;
    inet_pton(AF_INET, argv[1], &(dest.sin_addr));

    // ICMP 패킷 준비
    char sendBuf[PACKET_SIZE];
    char recvBuf[PACKET_SIZE + sizeof(struct iphdr)];
    struct icmphdr *icmpHeader = (struct icmphdr *)sendBuf;

    // ICMP 헤더 설정
    icmpHeader->type             = ICMP_ECHO;
    icmpHeader->code             = 0;
    icmpHeader->checksum         = 0;
    icmpHeader->un.echo.id       = getpid();
    icmpHeader->un.echo.sequence = 0;

    // 체크섬 계산
    icmpHeader->checksum = calculateChecksum((unsigned short *)sendBuf, PACKET_SIZE);

    // 패킷 전송
    if (sendto(sock, sendBuf, PACKET_SIZE, 0, (struct sockaddr *)&dest, sizeof(dest)) <= 0)
    {
        std::cout << "Failed to send packet" << std::endl;
        close(sock);
        return 1;
    }

    // 응답 대기
    struct sockaddr_in from;
    socklen_t fromLen = sizeof(from);
    int bytes         = recvfrom(sock, recvBuf, sizeof(recvBuf), 0, (struct sockaddr *)&from, &fromLen);

    if (bytes <= 0)
    {
        std::cout << "Failed to receive response" << std::endl;
    }
    else
    {
        // IP 헤더를 건너뛰고 ICMP 패킷 확인
        struct icmphdr *icmpResponse = (struct icmphdr *)(recvBuf + sizeof(struct iphdr));
        if (icmpResponse->type == ICMP_ECHOREPLY)
        {
            std::cout << "Reply from " << argv[1] << ": bytes=" << PACKET_SIZE
                      << " id=" << ntohs(icmpResponse->un.echo.id)
                      << " sequence=" << ntohs(icmpResponse->un.echo.sequence) << std::endl;
        }
    }

    // 정리
    close(sock);
    return 0;
}
