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

#include "core/event/EventTypesToJS.h"

namespace cc {

// Internal NodeEventType : 0~99
// Internal EventTypesToJS : 100~199
// Internal Game event : 200~299
// Internal Director Event Type: 300~399
const CallbacksInvoker::KeyType EventTypesToJS::ROOT_BATCH2D_INIT{100};           //{"ROOT_BATCH2D_INIT"};
const CallbacksInvoker::KeyType EventTypesToJS::ROOT_BATCH2D_UPDATE{101};         //{"ROOT_BATCH2D_UPDATE"};
const CallbacksInvoker::KeyType EventTypesToJS::ROOT_BATCH2D_UPLOAD_BUFFERS{102}; //{"ROOT_BATCH2D_UPLOAD_BUFFERS"};
const CallbacksInvoker::KeyType EventTypesToJS::ROOT_BATCH2D_RESET{103};          //{"ROOT_BATCH2D_RESET"};

const CallbacksInvoker::KeyType EventTypesToJS::NODE_REATTACH{104};                 //{"NODE_REATTACH"};
const CallbacksInvoker::KeyType EventTypesToJS::NODE_REMOVE_PERSIST_ROOT_NODE{105}; //{"NODE_REMOVE_PERSIST_ROOT_NODE"};
const CallbacksInvoker::KeyType EventTypesToJS::NODE_DESTROY_COMPONENTS{106};       //{"NODE_DESTROY_COMPONENTS"};
const CallbacksInvoker::KeyType EventTypesToJS::NODE_UI_TRANSFORM_DIRTY{107};       //{"NODE_UI_TRANSFORM_DIRTY"};
const CallbacksInvoker::KeyType EventTypesToJS::NODE_ACTIVE_NODE{108};              //{"NODE_ACTIVE_NODE"};
const CallbacksInvoker::KeyType EventTypesToJS::NODE_ON_BATCH_CREATED{109};         //{"NODE_ON_BATCH_CREATED"};

const CallbacksInvoker::KeyType EventTypesToJS::DIRECTOR_BEFORE_COMMIT{110}; //{"MODEL_GET_MACRO_PATCHES"};

} // namespace cc
