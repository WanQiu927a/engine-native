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

#include <memory>
#include <variant>
#include "core/ArrayBuffer.h"
#include "base/TypeDef.h"

namespace cc {

template <typename T>
class TypedArrayTemp {
public:
    TypedArrayTemp() {
    }

    explicit TypedArrayTemp(uint32_t length) {
        _buffer     = std::make_shared<ArrayBuffer>(length);
        _byteEndPos = length;
    }

    explicit TypedArrayTemp(ArrayBuffer::Ptr buffer)
    : TypedArrayTemp(buffer, 0, buffer->byteLength()) {}

    TypedArrayTemp(ArrayBuffer::Ptr buffer, uint32_t byteOffset)
    : TypedArrayTemp(buffer, byteOffset, buffer->byteLength()) {}

    TypedArrayTemp(ArrayBuffer::Ptr buffer, uint32_t byteOffset, uint32_t length)
    : _buffer(buffer),
      _byteOffset(byteOffset),
      _byteLength(length),
      _byteEndPos(byteOffset + length) {
        CC_ASSERT(_byteEndPos <= _buffer->byteLength());
    }

    ~TypedArrayTemp() {
        _buffer.reset();
    }

    T &operator[](index_t idx) {
        CC_ASSERT(idx < length());
        return *((reinterpret_cast<T *>(_buffer->_data + _byteOffset)) + idx);
    }

    const T &operator[](index_t idx) const {
        CC_ASSERT(idx < length());
        return *((reinterpret_cast<T *>(_buffer->_data + _byteOffset)) + idx);
    }

    TypedArrayTemp<T> subarray(uint32_t begin) {
        return subArray(begin, _byteLength - begin + 1);
    }

    TypedArrayTemp<T> subarray(uint32_t begin, uint32_t end) {
        return TypedArrayTemp<T>(_buffer, begin, end - begin + 1);
    }

    void set(const ArrayBuffer::Ptr &buffer) {
        set(buffer, 0);
    }

    void set(const ArrayBuffer::Ptr &buffer, uint32_t offset) {
        CC_ASSERT(buffer->byteLength() + offset <= _byteEndPos);
        CC_ASSERT(_buffer);
        memcpy(_buffer->_data + offset, buffer->_data, buffer->byteLength());
    }

    void set(const TypedArrayTemp<T> &array) {
        set(array._buffer);
    }

    void set(const TypedArrayTemp<T> &array, uint32_t offset) {
        set(array._buffer, offset);
    }

    inline const ArrayBuffer::Ptr &buffer() const { return _buffer; }
    inline uint32_t                byteLength() const { return _byteLength; }
    inline uint32_t                length() const { return _byteLength / sizeof(T); }
    inline uint32_t                byteOffset() const { return _byteOffset; }
    inline uint32_t                bytesPerElement() const { return sizeof(T); }
    inline bool                    empty() const { return _byteLength == 0; }

private:
    ArrayBuffer::Ptr _buffer{nullptr};
    uint32_t         _byteOffset{0};
    uint32_t         _byteLength{0};
    uint32_t         _byteEndPos{0};
};

using Int8Array             = TypedArrayTemp<int8_t>;
using Int16Array            = TypedArrayTemp<int16_t>;
using Int32Array            = TypedArrayTemp<int32_t>;
using Uint8Array            = TypedArrayTemp<uint8_t>;
using Uint16Array           = TypedArrayTemp<uint16_t>;
using Uint32Array           = TypedArrayTemp<uint32_t>;
using Float32Array          = TypedArrayTemp<float>;
using Float64Array          = TypedArrayTemp<double>;
using TypedArray            = std::variant<Int8Array, Int16Array, Int32Array, Uint8Array, Uint16Array, Uint32Array, Float32Array, Float64Array>;
using TypedArrayElementType = std::variant<int8_t, int16_t, int32_t, uint8_t, uint16_t, uint32_t, float, double>;

uint32_t getTypedArrayLength(const TypedArray &arr);
uint32_t getTypedArrayBytesPerElement(const TypedArray &arr);

template <typename T>
T getTypedArrayValue(const TypedArray &arr, index_t idx) {
#define TYPEDARRAY_GET_VALUE(type)                         \
    if (auto *p = std::get_if<type>(&arr); p != nullptr) { \
        return static_cast<T>((*p)[idx]);                  \
    }

    TYPEDARRAY_GET_VALUE(Float32Array)
    TYPEDARRAY_GET_VALUE(Uint32Array)
    TYPEDARRAY_GET_VALUE(Uint16Array)
    TYPEDARRAY_GET_VALUE(Uint8Array)
    TYPEDARRAY_GET_VALUE(Int32Array)
    TYPEDARRAY_GET_VALUE(Int16Array)
    TYPEDARRAY_GET_VALUE(Int8Array)
    TYPEDARRAY_GET_VALUE(Float64Array)
#undef TYPEDARRAY_GET_VALUE

    return 0;
}

template <typename T>
void setTypedArrayValue(TypedArray &arr, index_t idx, T value) {
#define TYPEDARRAY_SET_VALUE(type)                         \
    if (auto *p = std::get_if<type>(&arr); p != nullptr) { \
        (*p)[idx] = value;                                 \
    }

    TYPEDARRAY_SET_VALUE(Float32Array)
    TYPEDARRAY_SET_VALUE(Uint32Array)
    TYPEDARRAY_SET_VALUE(Uint16Array)
    TYPEDARRAY_SET_VALUE(Uint8Array)
    TYPEDARRAY_SET_VALUE(Int32Array)
    TYPEDARRAY_SET_VALUE(Int16Array)
    TYPEDARRAY_SET_VALUE(Int8Array)
    TYPEDARRAY_SET_VALUE(Float64Array)
#undef TYPEDARRAY_SET_VALUE
}

template <typename T>
T &getTypedArrayValueRef(const TypedArray &arr, index_t idx) {
#define TYPEDARRAY_GET_VALUE_REF(type)                     \
    if (auto *p = std::get_if<type>(&arr); p != nullptr) { \
        return (*p)[idx];                                  \
    }

    TYPEDARRAY_GET_VALUE_REF(Float32Array)
    TYPEDARRAY_GET_VALUE_REF(Uint32Array)
    TYPEDARRAY_GET_VALUE_REF(Uint16Array)
    TYPEDARRAY_GET_VALUE_REF(Uint8Array)
    TYPEDARRAY_GET_VALUE_REF(Int32Array)
    TYPEDARRAY_GET_VALUE_REF(Int16Array)
    TYPEDARRAY_GET_VALUE_REF(Int8Array)
    TYPEDARRAY_GET_VALUE_REF(Float64Array)
#undef TYPEDARRAY_GET_VALUE_REF
}

template <typename T>
T getTypedArrayElementValue(const TypedArrayElementType &element) {
#define CAST_TO_T(type)                                        \
    if (auto *p = std::get_if<type>(&element); p != nullptr) { \
        return static_cast<T>(*p);                             \
    }

    CAST_TO_T(float)
    CAST_TO_T(uint32_t)
    CAST_TO_T(uint16_t)
    CAST_TO_T(uint8_t)
    CAST_TO_T(int32_t)
    CAST_TO_T(int16_t)
    CAST_TO_T(int8_t)
    CAST_TO_T(double)
#undef CAST_TO_T

    return 0;
}

} // namespace cc