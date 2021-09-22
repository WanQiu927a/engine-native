/****************************************************************************
// References: https://github.com/pcgpcgpcg/testbeastclion/blob/b2b441b4642e68302166d98f42fc42c988132e90/utils/EventEmitter.hpp
// Modified by wanxiang.xie@cocos.com
****************************************************************************/
#include "core/event/EventEmitter.h"
#include "base/Log.h"

template <typename Callback>
void EventEmitter::on(const std::string &name, Callback cb) {
    auto it = _events.find(name);
    if (it != _events.end()) {
        //        throw *new std::runtime_error("duplicate listener");
        CC_LOG_ERROR("duplicate listener on event (%s)", name.c_str());
    }

    if (++this->_listeners >= this->maxListeners) {
        std::cout
            << "warning: possible EventEmitter memory leak detected. "
            << this->_listeners
            << " listeners added. "
            << std::endl;
    };

    auto f        = toFunction(cb);
    auto fn       = new decltype(f)(toFunction(cb));
    _events[name] = static_cast<void *>(fn);
}

template <typename... Args>
void EventEmitter::emit(const std::string &name, Args... args) {
    auto it = _events.find(name);
    if (it != _events.end()) {
        auto *cb = _events.at(name);
        auto  fp = static_cast<std::function<void(Args...)> *>(cb);
        (*fp)(args...);
    }

    auto once = _events_once.find(name);
    if (once != _events_once.end()) {
        this->off(name);
    }
}

void EventEmitter::off() {
    _events.clear();
    _events_once.clear();
    this->_listeners = 0;
}

void EventEmitter::off(const std::string &name) {
    auto it = _events.find(name);

    if (it != _events.end()) {
        _events.erase(it);
        this->_listeners--;

        auto once = _events_once.find(name);
        if (once != _events_once.end()) {
            _events_once.erase(once);
        }
    }
}