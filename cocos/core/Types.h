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

#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include <cstdint>

#include "base/Value.h"
#include "math/Vec3.h"

namespace cc {

struct Error {
};

using HTMLElement = void *;

using Int8Array    = std::vector<int8_t>;
using Int16Array   = std::vector<int16_t>;
using Int32Array   = std::vector<int32_t>;
using Uint8Array   = std::vector<uint8_t>;
using Uint16Array  = std::vector<uint16_t>;
using Uint32Array  = std::vector<uint32_t>;
using Float32Array = std::vector<float>;
using Float64Array = std::vector<double>;

using TypedArray = std::variant<Int8Array, Int16Array, Int32Array, Uint8Array, Uint16Array, Uint32Array, Float32Array, Float64Array>;
using IndexArray = std::variant<Uint8Array, Uint16Array, Uint32Array>;

struct BoundingBox {
    Vec3 min;
    Vec3 max;
};

struct VertexIdChannel {
    uint32_t stream;
    uint32_t index;
};

struct NativeDep {
    std::string uuid;
    std::string ext;
    bool        __isNative__{false};

    explicit NativeDep() = default;

    explicit NativeDep(bool isNative_, const std::string &uuid_, const std::string &ext_)
    : uuid(uuid_), ext(ext_), __isNative__(isNative_), _isValid(true) {}

    inline bool isValid() const { return _isValid; }

private:
    bool _isValid{false};
};

} // namespace cc
