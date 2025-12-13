#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

#include "./core/func/func_main.h"

int main() {
#ifdef _WIN32
  SetConsoleOutputCP(65001);
  SetConsoleCP(65001);

  std::setlocale(LC_ALL, "ru_RU.UTF-8");
  std::locale::global(std::locale("ru_RU.UTF-8"));

  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD mode = 0;
  GetConsoleMode(hConsole, &mode);
  SetConsoleMode(hConsole, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
  Func app;
  app.cls();
  app.loop();

  return 0;
}
