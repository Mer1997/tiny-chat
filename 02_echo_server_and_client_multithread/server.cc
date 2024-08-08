#include <arpa/inet.h>
#include <unistd.h>

#include <algorithm>
#include <cassert>
#include <csignal>
#include <iostream>
#include <mutex>
#include <thread>

struct StatReport {
  int client_count = 0;
  int client_processing = 0;
  std::chrono::milliseconds total_processing_time;
  std::chrono::milliseconds max_processing_time;
};

std::mutex stats_mtx;
StatReport stats;

void signal_handler(int signum) {
  if (signum == SIGUSR1) {
    std::scoped_lock lck(stats_mtx);
    std::cout << "================= Statistics =================" << std::endl;
    std::cout << "Clients Served: " << stats.client_count << std::endl;
    std::cout << "Clients Processing: " << stats.client_processing << std::endl;
    std::cout << "Total Processing Time: "
              << stats.total_processing_time.count() << " ms" << std::endl;
    std::cout << "Average Processing Time: "
              << (stats.total_processing_time / stats.client_count).count()
              << " ms" << std::endl;
    std::cout << "Long-tail Latency: " << stats.max_processing_time.count()
              << " ms" << std::endl;
    std::cout << "==============================================" << std::endl;
  }
}

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

    const int enable = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
      ::perror("setsockopt(SO_REUSEADDR) failed");
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
  // 设置信号处理程序
  struct sigaction action;
  action.sa_handler = signal_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = SA_RESTART; // 使用 SA_RESTART 标志
  sigaction(SIGUSR1, &action, nullptr);

  auto server_ip = "127.0.0.1";
  auto server_port = 7997;

  auto server_sock = ServerSocket();
  bind(server_sock, server_ip, server_port);
  listen(server_sock);

  std::cout << "server[" + std::string(server_ip) + ":" +
                   std::to_string(server_port) + "]: waiting for connect"
            << std::endl;

  while (true) {
    auto client_sock = ClientSocket();
    auto sa_len = socklen_t(sizeof(client_sock.addr));

    client_sock.fd =
        ::accept(server_sock.fd, (sockaddr *)&client_sock.addr, &sa_len);
    if (client_sock.fd <= 0) {
      ::perror("Bad file descriptor");
    }

    auto process_client = [&](ClientSocket &&client) {
      {
        std::scoped_lock lck(stats_mtx);
        stats.client_processing++;
        // std::cout << "process_client: " << stats.client_processing <<
        // std::endl;
      }

      auto start = std::chrono::high_resolution_clock::now();

      char client_ip[INET_ADDRSTRLEN] = {};
      inet_ntop(AF_INET, &client.addr.sin_addr, client_ip, INET_ADDRSTRLEN);
      auto client_port = ntohs(client.addr.sin_port);

      // std::cout << "server[" + std::string(server_ip) + ":" +
      //                  std::to_string(server_port) +
      //                  "]: connection established with client[" +
      //                  std::string(client_ip) + ":" +
      //                  std::to_string(client_port) + "]("
      //           << client.fd << ")" << std::endl;

      while (true) {
        char buffer[BUFSIZ] = {};
        if (auto len = ::read(client.fd, buffer, BUFSIZ); len < 0) {
          ::perror("Error reading");
          break;
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
        };
      }

      auto end = std::chrono::high_resolution_clock::now();
      auto spent =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
      // std::cout << "server[" + std::string(server_ip) + ":" +
      //                  std::to_string(server_port) + "]: spent "
      //           << spent.count()
      //           << " ms for client[" + std::string(client_ip) + ":" +
      //                  std::to_string(client_port) + "]"
      //           << std::endl;

      {
        std::scoped_lock lck(stats_mtx);
        stats.client_count++;
        stats.client_processing--;
        stats.total_processing_time += spent;
        stats.max_processing_time = std::max(stats.max_processing_time, spent);
      }

      // simulate processing
      std::this_thread::sleep_for(std::chrono::seconds(3));

      ::close(client.fd);
    };

    std::thread(process_client, std::move(client_sock)).detach();
  }

  ::close(server_sock.fd);
  return 0;
}
