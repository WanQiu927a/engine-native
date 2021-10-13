/****************************************************************************
 Copyright (c) 2021 Xiamen Yaji Software Co., Ltd.
 
 http://www.cocos.com
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.
 
 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
 ****************************************************************************/
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "base/Utils.h"
#include "core/data/Object.h"
#include "core/memop/Pool.h"

namespace cc {

#define CC_CALLBACK_INVOKE_0(__selector__, __target__, ...)                   std::function<void()>(std::bind(&__selector__, __target__, ##__VA_ARGS__)), __target__
#define CC_CALLBACK_INVOKE_1(__selector__, __target__, Arg0, ...)             std::function<void(Arg0)>(std::bind(&__selector__, __target__, std::placeholders::_1, ##__VA_ARGS__)), __target__
#define CC_CALLBACK_INVOKE_2(__selector__, __target__, Arg0, Arg1, ...)       std::function<void(Arg0, Arg1)>(std::bind(&__selector__, __target__, std::placeholders::_1, std::placeholders::_2, ##__VA_ARGS__)), __target__
#define CC_CALLBACK_INVOKE_3(__selector__, __target__, Arg0, Arg1, Arg2, ...) std::function<void(Arg0, Arg1, Arg2)>(std::bind(&__selector__, __target__, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, ##__VA_ARGS__)), __target__

struct CallbackInfoBase {
    using ID = uint32_t;

    CallbackInfoBase()          = default;
    virtual ~CallbackInfoBase() = default;

    virtual bool check() const = 0;
    virtual void reset()       = 0;

    ID    _id{0};
    void *_target{nullptr};
    bool  _once{false};
};

template <typename... Args>
struct CallbackInfo final : public CallbackInfoBase {
    using CallbackFn = std::function<void(Args...)>;
    CallbackFn _callback{nullptr};

    template <typename AnyFunction>
    void set(AnyFunction &&callback, void *target, bool once) {
        _callback = std::forward<AnyFunction>(callback);
        _target   = target;
        _once     = once;
    }

    void reset() override {
        _callback = nullptr;
        _target   = nullptr;
        _once     = false;
    }

    bool check() const override {
        // Validation
        auto *target = dynamic_cast<CCObject *>(reinterpret_cast<CCObject *>(_target));
        if (target != nullptr) {
            if (!isObjectValid(target, true)) {
                return false;
            }
        }
        return true;
    }
};

/**
 * @zh 事件监听器列表的简单封装。
 * @en A simple list of event callbacks
 */
class CallbackList final {
public:
    std::vector<std::shared_ptr<CallbackInfoBase>> _callbackInfos;
    bool                                           _isInvoking{false};
    bool                                           _containCanceled{false};

    /**
     * @zh 从列表中移除与指定目标相同回调函数的事件。
     * @en Remove the event listeners with the given callback from the list
     *
     * @param cbID - The callback id to be removed
     */
    void removeByCallbackID(CallbackInfoBase::ID cbID);

    /**
     * @zh 从列表中移除与指定目标相同调用者的事件。
     * @en Remove the event listeners with the given target from the list
     * @param target
     */
    void removeByTarget(void *target);

    /**
     * @zh 移除指定编号事件。
     * @en Remove the event listener at the given index
     * @param index
     */
    void cancel(index_t index);

    /**
     * @zh 注销所有事件。
     * @en Cancel all event listeners
     */
    void cancelAll();

    /**
     * @zh 立即删除所有取消的回调。（在移除过程中会更加紧凑的排列数组）
     * @en Delete all canceled callbacks and compact array
     */
    void purgeCanceled();

    /**
     * @zh 清除并重置所有数据。
     * @en Clear all data
     */
    void clear();
};

/**
 * @zh CallbacksInvoker 用来根据事件名（Key）管理事件监听器列表并调用回调方法。
 * @en CallbacksInvoker is used to manager and invoke event listeners with different event keys,
 * each key is mapped to a CallbackList.
 */
class CallbacksInvoker final {
public:
    static constexpr int MAX_SIZE{16};

    /**
     * @zh 向一个事件名注册一个新的事件监听器，包含回调函数和调用者
     * @en Register an event listener to a given event key with callback and target.
     *
     * @param key - Event type
     * @param callback - Callback function when event triggered
     * @param target - Callback callee
     * @param once - Whether invoke the callback only once (and remove it)
     */
    template <typename... Args>
    CallbackInfoBase::ID on(const std::string &key, const std::function<void(Args...)> &callback, void *target, bool once = false);

    template <typename... Args>
    CallbackInfoBase::ID on(const std::string &key, const std::function<void(Args...)> &callback, bool once = false);

    template <typename LambdaType>
    CallbackInfoBase::ID on(const std::string &key, const LambdaType &callback, void *target, bool once = false);

    template <typename LambdaType>
    CallbackInfoBase::ID on(const std::string &key, const LambdaType &callback, bool once = false);

    /**
     * @zh 检查指定事件是否已注册回调。
     * @en Checks whether there is correspond event listener registered on the given event
     * @param key - Event type
     * @param cbID - Callback ID
     */
    bool hasEventListener(const std::string &key);
    bool hasEventListener(const std::string &key, CallbackInfoBase::ID cbID);

    /**
     * @zh 移除在特定事件类型中注册的所有回调或在某个目标中注册的所有回调。
     * @en Removes all callbacks registered in a certain event type or all callbacks registered with a certain target
     * @param keyOrTarget - The event type or target with which the listeners will be removed
     */
    void removeAll(const std::string &key);

    /**
     * @zh 删除以指定事件，回调函数，目标注册的回调。
     * @en Remove event listeners registered with the given event key, callback and target
     * @param key - Event type
     * @param callback - The callback function of the event listener, if absent all event listeners for the given type will be removed
     * @param target - The callback callee of the event listener
     */
    void off(const std::string &key, CallbackInfoBase::ID cbID);

    /**
     * @zh 派发一个指定事件，并传递需要的参数
     * @en Trigger an event directly with the event name and necessary arguments.
     * @param key - event type
     * @param arg0 - The first argument to be passed to the callback
     * @param arg1 - The second argument to be passed to the callback
     * @param arg2 - The third argument to be passed to the callback
     * @param arg3 - The fourth argument to be passed to the callback
     * @param arg4 - The fifth argument to be passed to the callback
     */
    template <typename... Args>
    void emit(const std::string &key, Args &&...args);

private:
    template <typename T>
    struct function_traits
    : public function_traits<decltype(&T::operator())> {};

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType (ClassType::*)(Args...) const> {
        typedef std::function<ReturnType(Args...)> f_type;
    };

    template <typename L>
    typename function_traits<L>::f_type makeFunction(L l) {
        return (typename function_traits<L>::f_type)(l);
    }

    std::unordered_map<std::string, CallbackList> _callbackTable;
    static CallbackInfoBase::ID                   cbIDCounter;
};

template <typename... Args>
CallbackInfoBase::ID CallbacksInvoker::on(const std::string &key, const std::function<void(Args...)> &callback, void *target, bool once) {
    auto &list                = _callbackTable[key];
    auto  info                = std::make_shared<CallbackInfo<Args...>>();
    info->_id                 = ++cbIDCounter;
    CallbackInfoBase::ID cbID = info->_id;
    info->set(callback, target, once);
    list._callbackInfos.emplace_back(std::move(info));
    return cbID;
}

template <typename... Args>
CallbackInfoBase::ID CallbacksInvoker::on(const std::string &key, const std::function<void(Args...)> &callback, bool once) {
    return on(key, callback, nullptr, once);
}

template <typename LambdaType>
CallbackInfoBase::ID CallbacksInvoker::on(const std::string &key, const LambdaType &callback, void *target, bool once) {
    return on(key, makeFunction(callback), target, once);
}

template <typename LambdaType>
CallbackInfoBase::ID CallbacksInvoker::on(const std::string &key, const LambdaType &callback, bool once) {
    return on(key, makeFunction(callback), once);
}

template <typename... Args>
void CallbacksInvoker::emit(const std::string &key, Args &&...args) {
    auto iter = _callbackTable.find(key);
    if (iter != _callbackTable.end()) {
        auto &list        = iter->second;
        bool  rootInvoker = !list._isInvoking;
        list._isInvoking  = true;

        auto &infos = list._callbackInfos;
        for (size_t i = 0, len = infos.size(); i < len; ++i) {
            if (infos[i] == nullptr) {
                continue;
            }

            auto info = std::dynamic_pointer_cast<CallbackInfo<Args...>>(infos[i]);
            if (info != nullptr) {
                const auto &         callback = info->_callback;
                CallbackInfoBase::ID cbID     = info->_id;
                // Pre off once callbacks to avoid influence on logic in callback
                if (info->_once) {
                    off(key, cbID);
                }
                // Lazy check validity of callback target,
                // if target is CCObject and is no longer valid, then remove the callback info directly
                if (!info->check()) {
                    off(key, cbID);
                } else {
                    callback(std::forward<Args>(args)...);
                }
            } else {
                CCASSERT(false, "EventEmitter::emit: Invalid event signature.");
            }
        }

        if (rootInvoker) {
            list._isInvoking = false;
            if (list._containCanceled) {
                list.purgeCanceled();
            }
        }
    }
}

} // namespace cc
