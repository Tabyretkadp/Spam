#include <td/telegram/Client.h>
#include <td/telegram/td_api.h>
#include <td/telegram/td_api.hpp>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>

#include "./tdapp.h"

TdApp::TdApp() {
  td::ClientManager::execute(
      td_api::make_object<td_api::setLogVerbosityLevel>(1));
  client_manager_ = std::make_unique<td::ClientManager>();
  client_id_ = client_manager_->create_client_id();
  send_query(td_api::make_object<td_api::getOption>("version"), {});
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
