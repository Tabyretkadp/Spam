#include <condition_variable>
#include <mutex>
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <vector>

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

  void updates_thread(int arg);
  void send_msg(int64_t chat_id, std::string outText);
  void cls();
  bool is_valid_username(const std::string &username);
  void getChats(int arg, std::function<void(std::vector<int64_t>)> callback);
  bool searchPublicChat(std::string publicChat);
  void loop();

private:
  std::mutex chats_mutex_;
  std::condition_variable chats_cv_;
  bool chats_ready_;
};
