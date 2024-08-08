#include <arpa/inet.h>
#include <atomic>
#include <chrono>
#include <iostream>
#include <mutex>
#include <netinet/in.h>
#include <sys/socket.h>
#include <thread>
#include <unistd.h>
#include <vector>

struct Stats {
  std::chrono::milliseconds total_response_time;
  std::chrono::milliseconds max_response_time;
  int request_count;

  Stats() : total_response_time(0), max_response_time(0), request_count(0) {}
};

Stats stats;
std::mutex stats_mutex;

std::atomic<int> count;

void client_task(const char *server_address, int port, int id) {

  int sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in server_addr = {};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  inet_pton(AF_INET, server_address, &server_addr.sin_addr);

  auto start = std::chrono::high_resolution_clock::now();

  if (auto ret = connect(sock, (sockaddr *)&server_addr, sizeof(server_addr));
      ret != 0) {
    perror("Connect failed");
  }
  std::string message = "Hello from client " + std::to_string(id);
  send(sock, message.c_str(), message.size(), 0);

  char buffer[1024];
  auto len = read(sock, buffer, sizeof(buffer));
  if (len < 0) {
    perror("Read failed");
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto response_time =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  close(sock);

  std::lock_guard<std::mutex> lock(stats_mutex);
  stats.total_response_time += response_time;
  if (response_time > stats.max_response_time) {
    stats.max_response_time = response_time;
  }
  stats.request_count++;
}

int main() {
  const int client_count = 10000;
  const char *server_address = "127.0.0.1";
  const int port = 7997;

  std::vector<std::thread> clients;
  for (int i = 0; i < client_count; ++i) {
    clients.emplace_back(client_task, server_address, port, i);
  }

  for (auto &client : clients) {
    client.join();
  }

  auto avg_response_time = stats.request_count > 0
                               ? stats.total_response_time / stats.request_count
                               : std::chrono::milliseconds(0);

  std::cout << "================= Client Statistics ================="
            << std::endl;
  std::cout << "Total Requests: " << stats.request_count << std::endl;
  std::cout << "Total Response Time: " << stats.total_response_time.count()
            << " ms" << std::endl;
  std::cout << "Average Response Time: " << avg_response_time.count() << " ms"
            << std::endl;
  std::cout << "Max Response Time: " << stats.max_response_time.count() << " ms"
            << std::endl;
  std::cout << "====================================================="
            << std::endl;

  return 0;
}
