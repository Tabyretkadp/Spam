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

namespace detail {
template <class... Fs> struct overload;

template <class F> struct overload<F> : public F {
  explicit overload(F f) : F(f) {}
};
template <class F, class... Fs>
struct overload<F, Fs...> : public overload<F>, public overload<Fs...> {
  overload(F f, Fs... fs) : overload<F>(f), overload<Fs...>(fs...) {}
  using overload<F>::operator();
  using overload<Fs...>::operator();
};
} // namespace detail

template <class... F> auto overloaded(F... f) {
  return detail::overload<F...>(f...);
}

namespace td_api = td::td_api;

static bool updates_thread_started = false;
static bool updates_thread_started_down = false;

class TdApp {
public:
  TdApp();

  void banner();
  void banner_all_command();
  void updates_thread();
  void send_msg(auto chat_id, std::string outText);
  void cls();
  bool is_valid_username(const std::string &username);
  void updates();
  void loop();

private:
  using Object = td_api::object_ptr<td_api::Object>;
  std::unique_ptr<td::ClientManager> client_manager_;
  std::int32_t client_id_{0};

  td_api::object_ptr<td_api::AuthorizationState> authorization_state_;
  bool are_authorized_{false};
  bool need_restart_{false};
  std::uint64_t current_query_id_{0};
  std::uint64_t authentication_query_id_{0};
  std::map<std::uint64_t, std::function<void(Object)>> handlers_;
  std::map<std::int64_t, td_api::object_ptr<td_api::user>> users_;
  std::map<std::int64_t, std::string> chat_title_;

  void restart();
  void send_query(td_api::object_ptr<td_api::Function> f,
                  std::function<void(Object)> handler);
  void process_response(td::ClientManager::Response response);
  std::string get_user_name(std::int64_t user_id);
  std::string get_chat_title(std::int64_t chat_id);
  void process_update(td_api::object_ptr<td_api::Object> update);
  auto create_authentication_query_handler();
  void on_authorization_state_update();
  void check_authentication_error(Object object);
  std::uint64_t next_query_id();
};
