#include <iostream>

void banner() {
  std::cout << "\033[38;5;196m|\033[0m \033[38;5;255mEnter action:"
            << std::endl;
  std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> [q] quit" << std::endl;
  std::cout
      << "\033[38;5;196m|\033[0m \033[38;5;255m> [c/cg] show chat/show gruop"
      << std::endl;
  std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> [m] send message"
            << std::endl;
  // std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> [b] msg bomber" <<
  // std::endl;
  std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> [cls] clear console"
            << std::endl;
  std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> [l] logout\033[0m"
            << std::endl;
  std::cout
      << "\033[38;5;196m|\033[0m \033[38;5;255m> [h] show all command\033[0m"
      << std::endl;
}

void banner_all_command() {
  std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> [0] break\033[0m"
            << std::endl;
  std::cout
      << "\033[38;5;196m|\033[0m \033[38;5;255m> [1][beta] subscribe + like"
      << std::endl;
  std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> [...] ...\033[0m"
            << std::endl;
}
