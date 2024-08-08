#include <arpa/inet.h>
#include <iostream>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <chrono>

void handle_first_connection(const char* server_ip, int server_port) {
  int sock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    ::perror("Socket creation failed");
    return;
  }

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  if (::inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
    ::perror("Invalid address/ Address not supported");
    ::close(sock);
    return;
  }

  if (::connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    ::perror("Connection failed");
    ::close(sock);
    return;
  }

  std::cout << "First connection established. Waiting for 10 seconds..." << std::endl;
  std::this_thread::sleep_for(std::chrono::seconds(10));
  ::shutdown(sock, SHUT_WR);
  std::cout << "First connection write end shutdown." << std::endl;

  ::close(sock);
}

void handle_second_connection(const char* server_ip, int server_port) {
  int sock = ::socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    ::perror("Socket creation failed");
    return;
  }

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  if (::inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0) {
    ::perror("Invalid address/ Address not supported");
    ::close(sock);
    return;
  }

  if (::connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
    ::perror("Connection failed");
    ::close(sock);
    return;
  }

  const char* message = "Hello, server!";
  ::send(sock, message, strlen(message), 0);
  std::cout << "Second connection: sent message to server." << std::endl;

  char buffer[1024] = {0};
  int len = ::recv(sock, buffer, sizeof(buffer), 0);
  if (len > 0) {
    std::cout << "Second connection: received message from server: " << std::string(buffer, len) << std::endl;
  } else {
    ::perror("recv");
  }

  ::close(sock);
}

int main() {
  const char* server_ip = "127.0.0.1";
  int server_port = 7997;

  std::thread first_conn_thread(handle_first_connection, server_ip, server_port);
  std::this_thread::sleep_for(std::chrono::seconds(1)); // 确保第一个连接先成功
  std::thread second_conn_thread(handle_second_connection, server_ip, server_port);

  first_conn_thread.join();
  second_conn_thread.join();

  return 0;
}
