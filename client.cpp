#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <server_ip>" << std::endl;
        return 1;
    }

    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // 서버 도달 가능성 테스트
    std::cout << "Testing server reachability..." << std::endl;
    std::string test_cmd = "nc -zvu " + std::string(argv[1]) + " 88";
    system(test_cmd.c_str());

    // 브로드캐스트 옵션 활성화
    int broadcast = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) < 0)
    {
        std::cerr << "Broadcast setsockopt failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // 타임아웃 설정
    struct timeval tv;
    tv.tv_sec  = 5;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {
        std::cerr << "Setsockopt failed: " << strerror(errno) << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_port        = htons(88);
    serverAddr.sin_addr.s_addr = inet_addr(argv[1]);

    const char *message = "Hello from client!";
    if (sendto(sockfd, message, strlen(message), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Send failed: " << strerror(errno) << std::endl;
        return 1;
    }
    std::cout << "Message sent to server: " << argv[1] << ":" << ntohs(serverAddr.sin_port) << std::endl;

    char buffer[1024];
    socklen_t len = sizeof(serverAddr);
    int n         = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&serverAddr, &len);

    if (n < 0)
    {
        std::cerr << "Receive failed: " << strerror(errno) << std::endl;
        std::cout << "\nDebug information:" << std::endl;
        std::cout << "1. Check if port 88 is open on server:" << std::endl;
        std::cout << "   sudo netstat -ulnp | grep 88" << std::endl;
        std::cout << "2. Check firewall status:" << std::endl;
        std::cout << "   sudo ufw status" << std::endl;
        std::cout << "3. Allow UDP port 88:" << std::endl;
        std::cout << "   sudo ufw allow 88/udp" << std::endl;
        return 1;
    }

    buffer[n] = '\0';
    std::cout << "Server response: " << buffer << std::endl;

    close(sockfd);
    return 0;
}
