#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

int main()
{
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        std::cerr << "Socket creation failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // 소켓 재사용 설정
    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
    {
        std::cerr << "Setsockopt failed: " << strerror(errno) << std::endl;
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family      = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port        = htons(12345);

    if (bind(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        std::cerr << "Bind failed: " << strerror(errno) << std::endl;
        return 1;
    }

    // 현재 바인딩된 주소 출력
    struct sockaddr_in actual_addr;
    socklen_t addr_len = sizeof(actual_addr);
    getsockname(sockfd, (struct sockaddr *)&actual_addr, &addr_len);
    std::cout << "Server bound to IP: " << inet_ntoa(actual_addr.sin_addr) << ", Port: " << ntohs(actual_addr.sin_port)
              << std::endl;

    std::cout << "Server is running on port 12345..." << std::endl;

    char buffer[1024];
    struct sockaddr_in clientAddr;
    socklen_t len = sizeof(clientAddr);

    while (true)
    {
        int n     = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&clientAddr, &len);
        buffer[n] = '\0';

        std::cout << "Received from client: " << buffer << std::endl;
        std::cout << "Client IP: " << inet_ntoa(clientAddr.sin_addr) << ", Port: " << ntohs(clientAddr.sin_port)
                  << std::endl;

        const char *reply = "Hello from server!";
        sendto(sockfd, reply, strlen(reply), 0, (struct sockaddr *)&clientAddr, len);
    }

    close(sockfd);
    return 0;
}
