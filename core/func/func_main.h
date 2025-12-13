#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>

#ifdef _WIN32
#include <windows.h>
#endif

#include <chrono>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "../ftdlib/tdapp.h"

static bool updates_thread_started = false;
static bool updates_thread_started_down = false;

class Func : public TdApp {
public:
  Func();

  void banner();
  void banner_all_command();
  void updates_thread(int arg);
  void send_msg(int64_t chat_id, std::string outText);
  void cls();
  bool is_valid_username(const std::string &username);
  void loop();

private:
};
