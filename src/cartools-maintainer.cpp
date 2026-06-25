#include <filesystem>
#include <iostream>
#include <fstream>
#include <ostream>

/* The cartools-maintainer command sets up the email and name for a package maintainer. */

int main() {
  std::cout << "name: ";
  std::string name;
  std::cin >> name;

  std::cout << "email: ";
  std::string email;
  std::cin >> email;

  std::string formatted = name + " <" + email + ">";
  std::cout << "formatted: " << formatted << std::endl;
  std::cout << "writing to ~/.config/redrose-linux-maintainer" << std::endl;

  try {
    std::string home = std::string(getenv("HOME"));
    std::ofstream file(home + "/.config/redrose-linux-maintainer");
    file << formatted;
    file.close();
  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "\e[91mERROR\e[0m: failed to write file:" << e.what() << std::endl;
    return 1;
  }

  return 0;
}
