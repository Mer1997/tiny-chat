#include <arpa/inet.h>
#include <unistd.h>

#include <cassert>
#include <iostream>
#include <thread>
#include <vector>

struct Socket {
  using entity_addr_t = sockaddr_in;
  int fd = -1;
  entity_addr_t addr;

protected:
  Socket() = default;
};

struct ServerSocket : public Socket {
  ServerSocket() {
    fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
      ::perror("Socket failed");
      ::exit(EXIT_FAILURE);
    }
  }
};

struct ClientSocket : public Socket {
  ClientSocket() = default;
};

void bind(ServerSocket sock, std::string_view ip, int port) {
  sock.addr.sin_family = AF_INET;
  sock.addr.sin_port = htons(port);
  ::inet_pton(AF_INET, ip.data(), &sock.addr.sin_addr);

  static_assert(INET_ADDRSTRLEN == sizeof(sock.addr));

  if (auto ret = ::bind(sock.fd, (sockaddr *)&sock.addr, INET_ADDRSTRLEN);
      ret != 0) {
    ::perror("Bind failed");
    ::exit(EXIT_FAILURE);
  }
}

void listen(ServerSocket sock) {
  if (auto ret = ::listen(sock.fd, 10); ret != 0) {
    ::perror("Listen failed");
    ::exit(EXIT_FAILURE);
  }
}

int main(int argc, char *argv[]) {

  auto server_ip = "127.0.0.1";
  auto server_port = 7997;

  auto server_sock = ServerSocket();
  bind(server_sock, server_ip, server_port);
  listen(server_sock);

  std::cout << "server[" + std::string(server_ip) + ":" +
                   std::to_string(server_port) + "]: waiting for connect"
            << std::endl;

  auto threads = std::vector<std::thread>{};

  while (true) {

    auto client_sock = ClientSocket();
    auto sa_len = socklen_t(sizeof(client_sock.addr));

    client_sock.fd =
        ::accept(server_sock.fd, (sockaddr *)&client_sock.addr, &sa_len);
    if (client_sock.fd <= 0) {
      ::perror("Bad file descriptor");
    }

    auto process_client = [&](ClientSocket &&client) {
      char client_ip[INET_ADDRSTRLEN] = {};
      inet_ntop(AF_INET, &client.addr.sin_addr, client_ip, INET_ADDRSTRLEN);
      auto client_port = ntohs(client.addr.sin_port);

      std::cout << "server[" + std::string(server_ip) + ":" +
                       std::to_string(server_port) +
                       "]: connection established with client[" +
                       std::string(client_ip) + ":" +
                       std::to_string(client_port) + "]("
                << client.fd << ")" << std::endl;

      while (true) {
        char buffer[BUFSIZ] = {};
        if (auto len = ::read(client.fd, buffer, BUFSIZ); len < 0) {
          ::perror("Error reading");
          ::exit(EXIT_FAILURE);
        } else if (len == 0) {
          std::cout << "server[" + std::string(server_ip) + ":" +
                           std::to_string(server_port) + "]: client[" +
                           std::string(client_ip) + ":" +
                           std::to_string(client_port) +
                           "] closed the connection"
                    << std::endl;
          break;
        } else {
          std::cout << "server[" + std::string(server_ip) + ":" +
                           std::to_string(server_port) +
                           "]: received from client[" + std::string(client_ip) +
                           ":" + std::to_string(client_port) + "]: "
                    << std::string(buffer) << std::endl;

          ::send(client.fd, buffer, len, 0);

          // simulate processing
          std::this_thread::sleep_for(std::chrono::seconds(5));
        };
      }

      ::close(client.fd);
    };

    threads.emplace_back(process_client, std::move(client_sock));
  }

  // maybe join here

  ::close(server_sock.fd);
  return 0;
}
