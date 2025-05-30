#ifndef EVENTBUS_H
#define EVENTBUS_H

#include <functional>
#include <memory>
#include <vector>


// shared_ptr по логике нужен, потому что мы не знаем о lifetime 
// нашего std::function. Если там какая-нибудь лямбда локальная, ещё что-то
// handler должен жить, пока он используется

template<typename... ARGS>
class EventBus
{
public:
  void FireEvent(ARGS... args)
  {
    for(auto& handler: handlers) { handler(args...); }
  }

  void Register(std::shared_ptr<std::function<bool(ARGS...)>> fn)
  {
    handlers.push_back(fn);
  }

  void Unregister(std::shared_ptr<std::function<bool(ARGS...)>> fn)
  {
    auto it = std::find(handlers.begin(), handlers.end(), fn);
    if(it == handlers.end()) handlers.erase(it);
  }

private:
  std::vector<std::shared_ptr<std::function<bool(ARGS...)>>> handlers;
};

#endif// EVENTBUS_H
