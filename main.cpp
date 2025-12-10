#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

#include <chrono>
#include <cstdint>
#include <ctime>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <thread>

#include "./main.h"

TdApp::TdApp() {
  td::ClientManager::execute(
      td_api::make_object<td_api::setLogVerbosityLevel>(1));
  client_manager_ = std::make_unique<td::ClientManager>();
  client_id_ = client_manager_->create_client_id();
  send_query(td_api::make_object<td_api::getOption>("version"), {});
}

void TdApp::banner() {
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

void TdApp::banner_all_command() {
  std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> [0] break\033[0m"
            << std::endl;
  std::cout
      << "\033[38;5;196m|\033[0m \033[38;5;255m> [1][beta] subscribe + like"
      << std::endl;
  std::cout << "\033[38;5;196m|\033[0m \033[38;5;255m> [...] ...\033[0m"
            << std::endl;
}

void TdApp::cls() {
#ifdef _WIN32
  system("cls");
#endif
  system("clear");
}

void TdApp::updates() {
  while (true) {
    auto response = client_manager_->receive(0);
    if (response.object) {
      process_response(std::move(response));
    } else {
      break;
    }
  }
}

void TdApp::updates_thread() {
  if (!updates_thread_started) {
    std::thread([this] {
      while (!updates_thread_started_down) {
        auto response = client_manager_->receive(5);
        if (response.object) {
          process_response(std::move(response));
        }
      }
    }).detach();
    updates_thread_started = true;
  }
}

void TdApp::send_msg(auto chat_id, std::string outText) {
  auto send_message = td_api::make_object<td_api::sendMessage>();
  send_message->chat_id_ = chat_id;
  auto message_content = td_api::make_object<td_api::inputMessageText>();
  message_content->text_ = td_api::make_object<td_api::formattedText>();
  message_content->text_->text_ = std::move(outText);
  send_message->input_message_content_ = std::move(message_content);

  send_query(std::move(send_message), {});
}

bool TdApp::is_valid_username(const std::string &username) {
  if (username.empty() || username.length() > 32)
    return false;

  for (char c : username) {
    if (!std::isalnum(c) && c != '_') {
      return false;
    }
  }

  return true;
}

void TdApp::loop() {
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
        send_query(td_api::make_object<td_api::getChats>(nullptr, 20),
                   [&](Object object) {
                     if (object->get_id() == td_api::error::ID) {
                       return;
                     }
                     auto chats = td::move_tl_object_as<td_api::chats>(object);
                     for (auto chat_id : chats->chat_ids_) {
                       if (chat_id < 0) {
                         std::cout << "\033[38;5;196m|\033[0m "
                                   << "\033[38;5;255m" << chat_id
                                   << "\t\033[38;5;196m->\t\033[0m"
                                   << "\033[38;5;255m" << chat_title_[chat_id]
                                   << "\033[0m" << std::endl;
                       }
                     }
                   });
        std::this_thread::sleep_for(std::chrono::milliseconds(70));
        updates();
      } else if (action == "c") {
        send_query(td_api::make_object<td_api::getChats>(nullptr, 20),
                   [&](Object object) {
                     if (object->get_id() == td_api::error::ID) {
                       return;
                     }
                     auto chats = td::move_tl_object_as<td_api::chats>(object);
                     for (auto chat_id : chats->chat_ids_) {
                       if (chat_id > 0) {
                         std::cout << "\033[38;5;196m|\033[0m "
                                   << "\033[38;5;255m" << chat_id
                                   << "\t\033[38;5;196m->\t\033[0m"
                                   << "\033[38;5;255m" << chat_title_[chat_id]
                                   << "\033[0m" << std::endl;
                       }
                     }
                   });

        std::this_thread::sleep_for(std::chrono::milliseconds(70));
        updates();
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

        updates_thread();

        while (true) {
          std::time_t now = time(0);
          std::tm *ltm = localtime(&now);
          send_query(td_api::make_object<td_api::getChats>(nullptr, 20),
                     [&, text](Object object) {
                       if (object->get_id() == td_api::error::ID) {
                         return;
                       }
                       auto chats =
                           td::move_tl_object_as<td_api::chats>(object);
                       for (auto chat_id : chats->chat_ids_) {
                         if (chat_id < 0) {
                           send_msg(chat_id, text);
                         }
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

      } else if (action == "1") {
        std::string public_chat;
        std::cout << "\033[38;5;196m|\033[0m \033[38;5;255mInput chat: \033[0m";
        std::cin >> public_chat;
        send_query(td_api::make_object<td_api::searchPublicChat>(public_chat),
                   [this](Object object) {
                     if (object->get_id() == td_api::error::ID) {
                       std::cout
                           << "\033[38;5;196m|\033[0m \033[38;5;255mCan't find "
                              "the channel :(\033[0m"
                           << std::endl;
                       return;
                     }

                     auto chat = td_api::move_object_as<td_api::chat>(object);
                     auto join = td_api::make_object<td_api::joinChat>();

                     join->chat_id_ = chat->id_;

                     send_query(std::move(join), [this](Object object) {
                       if (object->get_id() == td_api::error::ID) {
                         std::cout << "\033[38;5;196m|\033[0m "
                                      "\033[38;5;255mCould not subscribe\033[0m"
                                   << std::endl;
                         return;
                       } else {
                         std::cout << "\033[38;5;196m|\033[0m "
                                      "\033[38;5;255mSubscribed!\033[0m"
                                   << std::endl;
                       }
                     });
                   });
        std::this_thread::sleep_for(std::chrono::seconds(5));
        updates();
      }
    }
  }
}

void TdApp::restart() {
  client_manager_.reset();
  // TdApp();
}

void TdApp::send_query(td_api::object_ptr<td_api::Function> f,
                       std::function<void(Object)> handler) {
  auto query_id = next_query_id();
  if (handler) {
    handlers_.emplace(query_id, std::move(handler));
  }
  client_manager_->send(client_id_, query_id, std::move(f));
}

void TdApp::process_response(td::ClientManager::Response response) {
  if (!response.object) {
    return;
  }
  if (response.request_id == 0) {
    return process_update(std::move(response.object));
  }
  auto it = handlers_.find(response.request_id);
  if (it != handlers_.end()) {
    it->second(std::move(response.object));
    handlers_.erase(it);
  }
}

std::string TdApp::get_user_name(std::int64_t user_id) {
  auto it = users_.find(user_id);
  if (it == users_.end()) {
    return "unknown user";
  }
  return it->second->first_name_ + " " + it->second->last_name_;
}

std::string TdApp::get_chat_title(std::int64_t chat_id) {
  auto it = chat_title_.find(chat_id);
  if (it == chat_title_.end()) {
    return "unknown chat";
  }
  return it->second;
}

void TdApp::process_update(td_api::object_ptr<td_api::Object> update) {
  td_api::downcast_call(
      *update,
      overloaded(
          [&](td_api::updateAuthorizationState &update_authorization_state) {
            authorization_state_ =
                std::move(update_authorization_state.authorization_state_);
            on_authorization_state_update();
          },
          [&](td_api::updateNewChat &update_new_chat) {
            chat_title_[update_new_chat.chat_->id_] =
                update_new_chat.chat_->title_;
          },
          [&](td_api::updateChatTitle &update_chat_title) {
            chat_title_[update_chat_title.chat_id_] = update_chat_title.title_;
          },
          [&](td_api::updateUser &update_user) {
            auto user_id = update_user.user_->id_;
            users_[user_id] = std::move(update_user.user_);
          },
          [&](td_api::updateNewMessage &update_new_message) {
            if (update_new_message.message_->is_outgoing_) {
              return;
            }

            auto chat_id = update_new_message.message_->chat_id_;
            std::string sender_name;
            td_api::downcast_call(
                *update_new_message.message_->sender_id_,
                overloaded(
                    [this, &sender_name](td_api::messageSenderUser &user) {
                      sender_name = get_user_name(user.user_id_);
                    },
                    [this, &sender_name](td_api::messageSenderChat &chat) {
                      sender_name = get_chat_title(chat.chat_id_);
                    }));
            std::string text;
            if (update_new_message.message_->content_->get_id() ==
                td_api::messageText::ID) {
              text = static_cast<td_api::messageText &>(
                         *update_new_message.message_->content_)
                         .text_->text_;
            }

            std::string outText;
            if (!text.empty() && is_valid_username(text)) {
              outText = "Пытаюсь подписаться на @" + text + "...";
              send_msg(chat_id, outText);

              send_query(
                  td::td_api::make_object<td::td_api::searchPublicChat>(text),
                  [&](Object object) {
                    if (object->get_id() == td::td_api::error::ID) {
                      send_msg(chat_id, "Канал не найден :(");
                    } else {
                      auto chat =
                          td::td_api::move_object_as<td::td_api::chat>(object);

                      auto join_request =
                          td::td_api::make_object<td::td_api::joinChat>();
                      join_request->chat_id_ = chat->id_;

                      send_query(std::move(join_request), [&](Object object) {
                        if (object->get_id() == td::td_api::error::ID) {
                          send_msg(chat_id, "Не удалось подписаться :(");
                        }
                      });
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(1));
                    send_msg(chat_id, "Подписка успешна ;)");
                  });

            } else {
              std::fstream file("outtext.txt");
              if (!file.is_open()) {
                outText = "";
                send_msg(chat_id, outText);
              } else {
                getline(file, outText);
                file.close();
                size_t pos = 0;
                while ((pos = outText.find("\\n", pos)) != std::string::npos) {
                  outText.replace(pos, 2, "\n");
                  pos += 1;
                }
                send_msg(chat_id, outText);
              }
            }
          },
          [](auto &update) {}));
}

auto TdApp::create_authentication_query_handler() {
  return [&, id = authentication_query_id_](Object object) {
    if (id == authentication_query_id_) {
      check_authentication_error(std::move(object));
    }
  };
}

void TdApp::on_authorization_state_update() {
  authentication_query_id_++;
  td_api::downcast_call(
      *authorization_state_,
      overloaded(
          [&](td_api::authorizationStateReady &) {
            are_authorized_ = true;
            std::cout << "\033[38;5;196m";
            std::cout << "──────────── ✿ ─────────────\n";
            std::cout << "= ";
            std::cout << "\033[38;5;255mВъод в телеграм выполнен \033[0m";
            std::cout << "\033[38;5;196m= \n";
            std::cout << "──────────── ✿ ─────────────";
            std::cout << "\033[0m" << std::endl;
            banner();
          },
          [&](td_api::authorizationStateLoggingOut &) {
            are_authorized_ = false;
            std::cout << "Logging out" << std::endl;
          },
          [&](td_api::authorizationStateClosing &) {
            std::cout << "Closing" << std::endl;
          },
          [&](td_api::authorizationStateClosed &) {
            are_authorized_ = false;
            need_restart_ = true;
            std::cout << "Terminated" << std::endl;
          },
          [&](td_api::authorizationStateWaitPhoneNumber &) {
            std::cout << "Enter phone number: " << std::flush;
            std::string phone_number;
            std::cin >> phone_number;
            send_query(
                td_api::make_object<td_api::setAuthenticationPhoneNumber>(
                    phone_number, nullptr),
                create_authentication_query_handler());
          },
          [&](td_api::authorizationStateWaitPremiumPurchase &) {
            std::cout << "Telegram Premium subscription is required"
                      << std::endl;
          },
          [&](td_api::authorizationStateWaitEmailAddress &) {
            std::cout << "Enter email address: " << std::flush;
            std::string email_address;
            std::cin >> email_address;
            send_query(
                td_api::make_object<td_api::setAuthenticationEmailAddress>(
                    email_address),
                create_authentication_query_handler());
          },
          [&](td_api::authorizationStateWaitEmailCode &) {
            std::cout << "Enter email authentication code: " << std::flush;
            std::string code;
            std::cin >> code;
            send_query(
                td_api::make_object<td_api::checkAuthenticationEmailCode>(
                    td_api::make_object<td_api::emailAddressAuthenticationCode>(
                        code)),
                create_authentication_query_handler());
          },
          [&](td_api::authorizationStateWaitCode &) {
            std::cout << "Enter authentication code: " << std::flush;
            std::string code;
            std::cin >> code;
            send_query(
                td_api::make_object<td_api::checkAuthenticationCode>(code),
                create_authentication_query_handler());
          },
          [&](td_api::authorizationStateWaitRegistration &) {
            std::string first_name;
            std::string last_name;
            std::cout << "Enter your first name: " << std::flush;
            std::cin >> first_name;
            std::cout << "Enter your last name: " << std::flush;
            std::cin >> last_name;
            send_query(td_api::make_object<td_api::registerUser>(
                           first_name, last_name, false),
                       create_authentication_query_handler());
          },
          [&](td_api::authorizationStateWaitPassword &) {
            std::cout << "Enter authentication password: " << std::flush;
            std::string password;
            std::getline(std::cin, password);
            send_query(td_api::make_object<td_api::checkAuthenticationPassword>(
                           password),
                       create_authentication_query_handler());
          },
          [&](td_api::authorizationStateWaitOtherDeviceConfirmation &state) {
            std::cout << "Confirm this login link on another device: "
                      << state.link_ << std::endl;
          },
          [&](td_api::authorizationStateWaitTdlibParameters &) {
            auto request = td_api::make_object<td_api::setTdlibParameters>();
            request->database_directory_ = "../tdlib";
            request->use_message_database_ = true;
            request->use_secret_chats_ = true;
            request->api_id_ = 94575;
            request->api_hash_ = "a3406de8d171bb422bb6ddf3bbd800e2";
            request->system_language_code_ = "ru";
            request->device_model_ = "Desktop";
            request->application_version_ = "1.0";
            send_query(std::move(request),
                       create_authentication_query_handler());
          }));
}

void TdApp::check_authentication_error(Object object) {
  if (object->get_id() == td_api::error::ID) {
    auto error = td::move_tl_object_as<td_api::error>(object);
    std::cout << "Error: " << to_string(error) << std::flush;
    on_authorization_state_update();
  }
}

std::uint64_t TdApp::next_query_id() { return ++current_query_id_; }

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

  TdApp example;
  example.cls();
  example.loop();

  return 0;
}
