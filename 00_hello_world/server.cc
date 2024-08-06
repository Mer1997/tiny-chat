#include <format>
#include <iostream>

auto main(void) -> int {
  auto whoami = std::string("server");
  std::cout << std::format("[{}]: Hello world!", whoami) << std::endl;
  return 0;
}
