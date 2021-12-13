/****************************************************************************
 Copyright (c) 2019-2021 Xiamen Yaji Software Co., Ltd.

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

#include "base/CoreStd.h"

#include "GFXContext.h"
#include "GFXDevice.h"
#include "GFXObject.h"

namespace cc {
namespace gfx {

Device *Device::instance = nullptr;

Device *Device::getInstance() {
    return Device::instance;
}

Device::Device() {
    Device::instance = this;
    _features.fill(false);
}

Device::~Device() {
    Device::instance = nullptr;
    CC_SAFE_RELEASE(_cmdBuff);
    CC_SAFE_RELEASE(_queue);
    CC_SAFE_RELEASE(_context);
}

Format Device::getColorFormat() const {
    return _context->getColorFormat();
}

Format Device::getDepthStencilFormat() const {
    return _context->getDepthStencilFormat();
}

bool Device::initialize(const DeviceInfo &info) {
    _width        = info.width;
    _height       = info.height;
    _pixelRatio   = info.pixelRatio;
    _windowHandle = info.windowHandle;

    _bindingMappingInfo = info.bindingMappingInfo;
    if (_bindingMappingInfo.bufferOffsets.empty()) {
        _bindingMappingInfo.bufferOffsets.push_back(0);
    }
    if (_bindingMappingInfo.samplerOffsets.empty()) {
        _bindingMappingInfo.samplerOffsets.push_back(0);
    }

    bool result = doInit(info);
    _cmdBuff->addRef();
    _queue->addRef();
    _context->addRef();

    return result;
}

void Device::destroy() {
    doDestroy();

    _bindingMappingInfo.bufferOffsets.clear();
    _bindingMappingInfo.samplerOffsets.clear();
    _width = _height = _windowHandle = 0U;
    _pixelRatio                      = 1.0F;
}

} // namespace gfx
} // namespace cc
