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

#include "core/assets/ImageAsset.h"

#include "platform/Image.h"

namespace cc {

ImageAsset::~ImageAsset() {
    if (_nativeData != nullptr) {
        _nativeData->release();
    }
}

void ImageAsset::setNativeAsset(const std::any &obj) {
    if (obj.has_value()) {
        if (auto *pData = std::any_cast<Image *>(&obj); pData != nullptr) {
            _nativeData = *pData;
            _nativeData->retain();
        }
    }
}

const uint8_t *ImageAsset::getData() const {
    if (_nativeData != nullptr) {
        return _nativeData->getData();
    }
    return nullptr;
}

uint32_t ImageAsset::getWidth() const {
    if (_nativeData != nullptr) {
        return _nativeData->getWidth();
    }
    return 0;
}

uint32_t ImageAsset::getHeight() const {
    if (_nativeData != nullptr) {
        return _nativeData->getHeight();
    }
    return 0;
}

PixelFormat ImageAsset::getFormat() const {
    if (_nativeData != nullptr) {
        return static_cast<PixelFormat>(_nativeData->getRenderFormat());
    }
    return PixelFormat::RGBA8888; //cjh TODO: use RGBA8888 as default value?
}

bool ImageAsset::isCompressed() const {
    if (_nativeData != nullptr) {
        return _nativeData->isCompressed();
    }
    return false;
}

std::string ImageAsset::getUrl() const {
    if (_nativeData != nullptr) {
        return _nativeData->getFilePath();
    }
    return "";
}

} // namespace cc