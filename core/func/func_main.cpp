#include <functional>
#include <mutex>
#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <utility>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

#include <chrono>
#include <ctime>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>

#include "../visual/banners.h"
#include "./func_main.h"

Func::Func() : TdApp() {}

void Func::cls() {
#ifdef _WIN32
  system("cls");
#endif
  system("clear");
}

void Func::updates_thread(int arg) {
  if (arg == 1 && !updates_thread_started) {
    std::thread([this] {
      while (!updates_thread_started_down) {
        auto response = client_manager_->receive(5);
        if (response.object) {
          process_response(std::move(response));
        }
      }
    }).detach();
    updates_thread_started = true;
  } else if (arg == 0) {
    while (true) {
      auto response = client_manager_->receive(0);
      if (response.object) {
        process_response(std::move(response));
      } else {
        break;
      }
    }
  }
}

bool Func::is_valid_username(const std::string &username) {
  if (username.empty() || username.length() > 32)
    return false;

  for (char c : username) {
    if (!std::isalnum(c) && c != '_') {
      return false;
    }
  }

  return true;
}

void Func::send_msg(int64_t chat_id, std::string outText) {
  auto send_message = td_api::make_object<td_api::sendMessage>();
  send_message->chat_id_ = chat_id;
  auto message_content = td_api::make_object<td_api::inputMessageText>();
  message_content->text_ = td_api::make_object<td_api::formattedText>();
  message_content->text_->text_ = std::move(outText);
  send_message->input_message_content_ = std::move(message_content);

  send_query(std::move(send_message), {});
}

void Func::getChats(int arg,
                    std::function<void(std::vector<int64_t>)> callback) {
  send_query(td_api::make_object<td_api::getChats>(nullptr, 20),
             [this, arg, callback](Object object) {
               std::vector<int64_t> result;
               if (object->get_id() == td_api::error::ID) {
                 auto error = td_api::move_object_as<td_api::error>(object);
                 std::cout << "error: " << error->message_ << std::endl;
                 return;
               }

               auto chats = td::move_tl_object_as<td_api::chats>(object);
               for (auto chat_id : chats->chat_ids_) {
                 if (chat_id < 0 && arg == 1) {
                   result.push_back(chat_id);
                 } else if (chat_id > 0 && arg == 0) {
                   result.push_back(chat_id);
                 }
               }

               callback(result);
             });
}

bool Func::searchPublicChat(std::string publicChat) {
  send_query(td_api::make_object<td_api::searchPublicChat>(publicChat),
             [this](Object object) {
               std::vector<int64_t> result;
               if (object->get_id() == td_api::error::ID) {
                 auto error = td_api::move_object_as<td_api::error>(object);
                 std::cout << "error: " << error->message_ << std::endl;
                 return false;
               }
               return true;
             });
  return false;
}

void Func::loop() {
  while (true) {
    if (need_restart_) {
      restart();
    } else if (!are_authorized_) {
      process_response(client_manager_->receive(10));
    } else {
      std::cout << "\033[38;5;196m|\033[0m > ";
      std::string line;
      std::getline(std::cin, line);
      std::istringstream ss(line);
      std::string action;
      if (!(ss >> action)) {
        continue;
      }
      if (action == "q") {
        return;
      } else if (action == "cls" || action == "clear") {
        cls();
        banner();
      } else if (action == "...") {
        std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> ...\033[0m"
                  << std::endl;
      } else if (action == "h") {
        banner_all_command();
      } else if (action == "l") {
        send_query(td_api::make_object<td_api::logOut>(), {});
      } else if (action == "cg") {
        getChats(1, [this](std::vector<int64_t> chat_ids) {
          for (const auto &chat_id : chat_ids) {
            std::cout << "\033[38;5;196m|\033[0m "
                      << "\033[38;5;255m" << chat_id
                      << "\t\033[38;5;196m->\t\033[0m"
                      << "\033[38;5;255m" << chat_title_[chat_id] << "\033[0m"
                      << std::endl;
          }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
        updates_thread(0);
      } else if (action == "c") {
        getChats(0, [this](std::vector<int64_t> chat_ids) {
          for (const auto &chat_id : chat_ids) {
            std::cout << "\033[38;5;196m|\033[0m "
                      << "\033[38;5;255m" << chat_id
                      << "\t\033[38;5;196m->\t\033[0m"
                      << "\033[38;5;255m" << chat_title_[chat_id] << "\033[0m"
                      << std::endl;
          }
        });
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
        updates_thread(0);
      } else if (action == "m") {
        int time_in;
        std::string text;

        std::cout << "\033[38;5;196m|\033[0m \033[38;5;255mInput text: \033[0m";
        std::getline(std::cin, text);
        if (text == "0") {
          continue;
        }
        size_t pos = 0;
        while ((pos = text.find("\\n", pos)) != std::string::npos) {
          text.replace(pos, 2, "\n");
          pos += 1;
        }
        std::cout << "\033[38;5;196m|\033[0m \033[38;5;255mInput time: \033[0m";
        std::cin >> time_in;
        if (time_in == 0) {
          continue;
        }
        std::cin.ignore();

        updates_thread(1);

        while (true) {
          std::time_t now = time(0);
          std::tm *ltm = localtime(&now);

          getChats(1, [this, text](std::vector<int64_t> chat_ids) {
            for (const auto &chat_id : chat_ids) {
              send_msg(chat_id, text);
            }
          });

          std::cout << "\033[38;5;196m| \033[0m" << "\033[38;5;196m[\033[0m"
                    << "\033[38;5;255m" << ltm->tm_hour
                    << "\033[38;5;196m:" << "\033[38;5;255m" << ltm->tm_min
                    << "\033[38;5;196m:" << "\033[38;5;255m" << ltm->tm_sec
                    << "\033[38;5;196m] \033[38;5;255m" << "\t->\t"
                    << "Message send!"
                    << "\033[0m" << std::endl;

          std::this_thread::sleep_for(std::chrono::seconds(time_in));
        }

        updates_thread_started = false;
        updates_thread_started_down = false;
      }
      /*
      else if (action == "1") {
        std::string public_chat;
        std::cout << "\033[38;5;196m|\033[0m \033[38;5;255mInput chat: \033[0m";
        std::cin >> public_chat;

        if(searchPublicChat(public_chat)){
            std::cout << "Канал нашелся!" << std::endl;
        } else {
            std::cout << "Канал НЕ нашелся" << std::endl;
        }
      }
      */
    }
  }
}
