#include <arpa/inet.h>
#include <unistd.h>

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <thread>

int main(int argc, char *argv[]) {
  auto listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
  assert(listen_fd != -1);

  auto server_ip = "127.0.0.1";
  auto server_port = 7997;

  auto server_addr = sockaddr_in();
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(server_port);
  ::inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

  static_assert(INET_ADDRSTRLEN == sizeof(server_addr));

  if (auto ret = ::bind(listen_fd, (sockaddr *)&server_addr, INET_ADDRSTRLEN);
      ret != 0) {
    auto s = "bind ip" + std::string(server_ip) + "failed!";
    std::cerr << s << std::endl;
    ::exit(EXIT_FAILURE);
  }

  if (auto ret = ::listen(listen_fd, 10); ret != 0) {
    auto s = "listen ip" + std::string(server_ip) + "failed!";
    std::cerr << s << std::endl;
    ::exit(EXIT_FAILURE);
  }

  std::cout << "server[" + std::string(server_ip) + ":" +
                   std::to_string(server_port) + "]: waiting for connect"
            << std::endl;

  while (true) {
    auto client_addr = sockaddr_in();
    auto client_addr_len = socklen_t(sizeof(client_addr));
    // auto client_fd = ::accept(listen_fd, nullptr, nullptr);
    auto client_fd =
        ::accept(listen_fd, (sockaddr *)&client_addr, &client_addr_len);

    char client_ip[INET_ADDRSTRLEN] = {};
    inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    auto client_port = ntohs(client_addr.sin_port);

    std::cout << "server[" + std::string(server_ip) + ":" +
                     std::to_string(server_port) +
                     "]: connection established with client[" +
                     std::string(client_ip) + ":" +
                     std::to_string(client_port) + "]"
              << std::endl;

    char buffer[BUFSIZ] = {};
    if (auto len = ::read(client_fd, buffer, BUFSIZ); len <= 0) {
      std::cerr << "read() failed" << std::endl;

      ::exit(EXIT_FAILURE);
    } else {
      std::cout << "server[" + std::string(server_ip) + ":" +
                       std::to_string(server_port) +
                       "]: received from client[" + std::string(client_ip) +
                       ":" + std::to_string(client_port) + "]: "
                << std::string(buffer) << std::endl;

      ::send(client_fd, buffer, len, 0);

      // simulate processing
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
    ::close(client_fd);
  }
  ::close(listen_fd);
  return EXIT_SUCCESS;
}
