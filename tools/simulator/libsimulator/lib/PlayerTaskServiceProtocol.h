/****************************************************************************
 Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

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

#ifndef __PLAYER_TASK_SERVICE_PROTOCOL_H
#define __PLAYER_TASK_SERVICE_PROTOCOL_H

#include <string>

#include "PlayerMacros.h"
#include "PlayerServiceProtocol.h"
#include "cocos/base/RefCounted.h"

PLAYER_NS_BEGIN

class PlayerTask : public cc::RefCounted {
public:
    static const int STATE_IDLE = 0;
    static const int STATE_RUNNING = 1;
    static const int STATE_COMPLETED = 2;

    virtual ~PlayerTask(){};

    std::string getName() const;
    std::string getExecutePath() const;
    std::string getCommandLineArguments() const;
    std::string getOutput() const;
    int getState() const;
    bool isIdle() const;
    bool isRunning() const;
    bool isCompleted() const;
    float getLifetime() const;
    int getResultCode() const;

    virtual bool run() = 0;
    virtual void stop() = 0;
    virtual void runInTerminal() = 0;

protected:
    PlayerTask(const std::string &name,
               const std::string &executePath,
               const std::string &commandLineArguments);

    std::string _name;
    std::string _executePath;
    std::string _commandLineArguments;
    std::string _output;
    float _lifetime;
    int _state;
    int _resultCode;
};

class PlayerTaskServiceProtocol : public PlayerServiceProtocol {
public:
    virtual PlayerTask *createTask(const std::string &name,
                                   const std::string &executePath,
                                   const std::string &commandLineArguments) = 0;
    virtual PlayerTask *getTask(const std::string &name) = 0;
    virtual void removeTask(const std::string &name) = 0;
};

PLAYER_NS_END

#endif // __PLAYER_TASK_SERVICE_PROTOCOL_H
