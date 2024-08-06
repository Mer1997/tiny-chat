#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

const int PORT = 7997;
const char* SERVER_ADDRESS = "127.0.0.1"; // IPv4 地址

int main() {
    int sock = 0;
    struct sockaddr_storage server_addr;
    socklen_t addrlen;
    char buffer[1024] = {0};

    // 创建 socket
    sock = socket(AF_INET, SOCK_STREAM, 0); // 使用 IPv4
    if (sock < 0) {
        perror("Socket creation error");
        return -1;
    }

    // 设置服务器地址
    struct sockaddr_in *addr_in = (struct sockaddr_in *)&server_addr;
    addr_in->sin_family = AF_INET;
    addr_in->sin_port = htons(PORT);
    if (inet_pton(AF_INET, SERVER_ADDRESS, &addr_in->sin_addr) <= 0) {
        perror("Invalid address / Address not supported");
        return -1;
    }
    addrlen = sizeof(*addr_in);

    // 连接到服务器
    if (connect(sock, (struct sockaddr *)&server_addr, addrlen) < 0) {
        perror("Connection failed");
        return -1;
    }

    std::cout << "Connected to the server at " << SERVER_ADDRESS << ":" << PORT << std::endl;

    // 发送数据
    std::string message = "Hello, Echo Server!";
    send(sock, message.c_str(), message.size(), 0);
    std::cout << "Message sent: " << message << std::endl;

    // 接收服务器回显的数据
    int bytes_received = read(sock, buffer, sizeof(buffer) - 1);
    if (bytes_received > 0) {
        buffer[bytes_received] = '\0';
        std::cout << "Echo received: " << buffer << std::endl;
    }

    // 关闭连接
    close(sock);
    return 0;
}
