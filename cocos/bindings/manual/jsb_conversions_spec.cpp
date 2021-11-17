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

#include <sstream>
#include "jsb_conversions.h"

#include "cocos/base/Map.h"
#include "cocos/base/Vector.h"
#include "cocos/core/TypedArray.h"
#include "cocos/math/Geometry.h"
#include "cocos/math/Quaternion.h"
#include "cocos/math/Vec2.h"
#include "cocos/math/Vec3.h"
#include "core/ArrayBuffer.h"
#include "core/geometry/AABB.h"
#include "extensions/cocos-ext.h"
#include "network/Downloader.h"

#include "cocos/core/geometry/Geometry.h"
#include "scene/Fog.h"
#include "scene/Shadow.h"
#include "scene/Skybox.h"

///////////////////////// utils /////////////////////////

template <class... Ts>
struct overloaded : Ts... { using Ts::operator()...; }; //NOLINT
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

template <typename A, typename T, typename F>
bool set_member_field(se::Object *obj, T *to, const std::string_view &property, F f, se::Value &tmp) { //NOLINT
    bool ok = obj->getProperty(property.data(), &tmp, true);
    SE_PRECONDITION2(ok, false, "Property '%s' is not set", property.data());
    if constexpr (std::is_member_function_pointer<F>::value) {
        A m;
        ok = sevalue_to_native(tmp, &m, obj);
        SE_PRECONDITION2(ok, false, "Convert property '%s' failed", property.data());
        (to->*f)(m);
        return true;
    }
    if constexpr (std::is_member_object_pointer<F>::value) {
        ok = sevalue_to_native(tmp, &(to->*f), obj);
        SE_PRECONDITION2(ok, false, "Convert property '%s' failed", property.data());
        return true;
    }
    static_assert(std::is_member_pointer_v<F>, "only member pointer allowed!");

    return false;
}

static bool isNumberString(const std::string &str) {
    for (const auto &c : str) {
        if (!isdigit(c)) {
            return false;
        }
    }
    return true;
}

namespace {

template <typename T>
bool std_vector_T_to_seval(const std::vector<T> &v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createArrayObject(v.size()));
    bool             ok = true;

    uint32_t i = 0;
    for (const auto &value : v) {
        if (!obj->setArrayElement(i, se::Value(value))) {
            ok = false;
            ret->setUndefined();
            break;
        }
        ++i;
    }

    if (ok) {
        ret->setObject(obj);
    }

    return ok;
}

} // namespace

namespace {
enum class DataType {
    INT,
    FLOAT
};
bool uintptr_t_to_seval(uintptr_t v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    ret->setDouble(v);
    return true;
}

bool size_to_seval(size_t v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    ret->setSize(v);
    return true;
}

bool Vec2_to_seval(const cc::Vec2 &v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.x));
    obj->setProperty("y", se::Value(v.y));
    ret->setObject(obj);

    return true;
}

bool Vec3_to_seval(const cc::Vec3 &v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.x));
    obj->setProperty("y", se::Value(v.y));
    obj->setProperty("z", se::Value(v.z));
    ret->setObject(obj);

    return true;
}

bool Vec4_to_seval(const cc::Vec4 &v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.x));
    obj->setProperty("y", se::Value(v.y));
    obj->setProperty("z", se::Value(v.z));
    obj->setProperty("w", se::Value(v.w));
    ret->setObject(obj);

    return true;
}

bool Quaternion_to_seval(const cc::Quaternion &v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.x));
    obj->setProperty("y", se::Value(v.y));
    obj->setProperty("z", se::Value(v.z));
    obj->setProperty("w", se::Value(v.w));
    ret->setObject(obj);

    return true;
}

bool Mat4_to_seval(const cc::Mat4 &v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createArrayObject(16));

    for (uint8_t i = 0; i < 16; ++i) {
        obj->setArrayElement(i, se::Value(v.m[i]));
    }

    ret->setObject(obj);
    return true;
}

bool Size_to_seval(const cc::Size &v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("width", se::Value(v.width));
    obj->setProperty("height", se::Value(v.height));
    ret->setObject(obj);
    return true;
}

bool Rect_to_seval(const cc::Rect &v, se::Value *ret) { // NOLINT(readability-identifier-naming)
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("x", se::Value(v.origin.x));
    obj->setProperty("y", se::Value(v.origin.y));
    obj->setProperty("width", se::Value(v.size.width));
    obj->setProperty("height", se::Value(v.size.height));
    ret->setObject(obj);

    return true;
}
void toVec2(void *data, DataType type, se::Value *ret) {
    auto *   intptr   = static_cast<int32_t *>(data);
    auto *   floatptr = static_cast<float *>(data);
    cc::Vec2 vec2;
    if (DataType::INT == type) {
        vec2.x = static_cast<float>(intptr[0]);
        vec2.y = static_cast<float>(intptr[1]);
    } else {
        vec2.x = *floatptr;
        vec2.y = *(floatptr + 1);
    }

    Vec2_to_seval(vec2, ret);
}

void toVec3(void *data, DataType type, se::Value *ret) {
    auto *   intptr   = static_cast<int32_t *>(data);
    auto *   floatptr = static_cast<float *>(data);
    cc::Vec3 vec3;
    if (DataType::INT == type) {
        vec3.x = static_cast<float>(intptr[0]);
        vec3.y = static_cast<float>(intptr[1]);
        vec3.z = static_cast<float>(intptr[2]);
    } else {
        vec3.x = floatptr[0];
        vec3.y = floatptr[1];
        vec3.z = floatptr[2];
    }

    Vec3_to_seval(vec3, ret);
}

void toVec4(void *data, DataType type, se::Value *ret) {
    auto *   intptr   = static_cast<int32_t *>(data);
    auto *   floatptr = static_cast<float *>(data);
    cc::Vec4 vec4;
    if (DataType::INT == type) {
        vec4.x = static_cast<float>(intptr[0]);
        vec4.y = static_cast<float>(intptr[1]);
        vec4.z = static_cast<float>(intptr[2]);
        vec4.w = static_cast<float>(intptr[3]);
    } else {
        vec4.x = *floatptr;
        vec4.y = *(floatptr + 1);
        vec4.z = *(floatptr + 2);
        vec4.w = *(floatptr + 3);
    }

    Vec4_to_seval(vec4, ret);
}

void toMat(const float *data, int num, se::Value *ret) {
    se::HandleObject obj(se::Object::createPlainObject());

    char propName[4] = {0};
    for (int i = 0; i < num; ++i) {
        if (i < 10) {
            snprintf(propName, 3, "m0%d", i);
        }

        else {
            snprintf(propName, 3, "m%d", i);
        }

        obj->setProperty(propName, se::Value(*(data + i)));
    }
    ret->setObject(obj);
}
} // namespace

////////////////////////////////////////////////////////////////////////////
/////////////////sevalue to native//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

// NOLINTNEXTLINE(readability-identifier-naming)
bool seval_to_ccvalue(const se::Value &v, cc::Value *ret) {
    assert(ret != nullptr);
    bool ok = true;
    if (v.isObject()) {
        se::Object *jsobj = v.toObject();
        if (!jsobj->isArray()) {
            // It's a normal js object.
            cc::ValueMap dictVal;
            ok = seval_to_ccvaluemap(v, &dictVal);
            SE_PRECONDITION3(ok, false, *ret = cc::Value::VALUE_NULL);
            *ret = cc::Value(dictVal);
        } else {
            // It's a js array object.
            cc::ValueVector arrVal;
            ok = seval_to_ccvaluevector(v, &arrVal);
            SE_PRECONDITION3(ok, false, *ret = cc::Value::VALUE_NULL);
            *ret = cc::Value(arrVal);
        }
    } else if (v.isString()) {
        *ret = v.toString();
    } else if (v.isNumber()) {
        *ret = v.toDouble();
    } else if (v.isBoolean()) {
        *ret = v.toBoolean();
    } else if (v.isNullOrUndefined()) {
        *ret = cc::Value::VALUE_NULL;
    } else {
        SE_PRECONDITION2(false, false, "type not supported!");
    }

    return ok;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool seval_to_ccvaluemap(const se::Value &v, cc::ValueMap *ret) {
    assert(ret != nullptr);

    if (v.isNullOrUndefined()) {
        ret->clear();
        return true;
    }

    SE_PRECONDITION3(v.isObject(), false, ret->clear());
    SE_PRECONDITION3(!v.isNullOrUndefined(), false, ret->clear());

    se::Object *obj = v.toObject();

    cc::ValueMap &dict = *ret;

    std::vector<std::string> allKeys;
    SE_PRECONDITION3(obj->getAllKeys(&allKeys), false, ret->clear());

    bool      ok = false;
    se::Value value;
    cc::Value ccvalue;
    for (const auto &key : allKeys) {
        SE_PRECONDITION3(obj->getProperty(key.c_str(), &value), false, ret->clear());
        ok = seval_to_ccvalue(value, &ccvalue);
        SE_PRECONDITION3(ok, false, ret->clear());
        dict.emplace(key, ccvalue);
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool seval_to_ccvaluemapintkey(const se::Value &v, cc::ValueMapIntKey *ret) {
    assert(ret != nullptr);
    if (v.isNullOrUndefined()) {
        ret->clear();
        return true;
    }

    SE_PRECONDITION3(v.isObject(), false, ret->clear());
    SE_PRECONDITION3(!v.isNullOrUndefined(), false, ret->clear());

    se::Object *obj = v.toObject();

    cc::ValueMapIntKey &dict = *ret;

    std::vector<std::string> allKeys;
    SE_PRECONDITION3(obj->getAllKeys(&allKeys), false, ret->clear());

    bool      ok = false;
    se::Value value;
    cc::Value ccvalue;
    for (const auto &key : allKeys) {
        SE_PRECONDITION3(obj->getProperty(key.c_str(), &value), false, ret->clear());

        if (!isNumberString(key)) {
            SE_LOGD("seval_to_ccvaluemapintkey, found not numeric key: %s", key.c_str());
            continue;
        }

        int intKey = atoi(key.c_str());

        ok = seval_to_ccvalue(value, &ccvalue);
        SE_PRECONDITION3(ok, false, ret->clear());
        dict.emplace(intKey, ccvalue);
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool seval_to_ccvaluevector(const se::Value &v, cc::ValueVector *ret) {
    assert(ret != nullptr);

    SE_PRECONDITION3(v.isObject(), false, ret->clear());

    se::Object *obj = v.toObject();
    SE_PRECONDITION3(obj->isArray(), false, ret->clear());

    uint32_t len = 0;
    obj->getArrayLength(&len);

    bool      ok = false;
    se::Value value;
    cc::Value ccvalue;
    for (uint32_t i = 0; i < len; ++i) {
        if (obj->getArrayElement(i, &value)) {
            ok = seval_to_ccvalue(value, &ccvalue);
            SE_PRECONDITION3(ok, false, ret->clear());
            ret->push_back(ccvalue);
        }
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevals_variadic_to_ccvaluevector(const se::ValueArray &args, cc::ValueVector *ret) {
    bool      ok = false;
    cc::Value ccvalue;

    for (const auto &arg : args) {
        ok = seval_to_ccvalue(arg, &ccvalue);
        SE_PRECONDITION3(ok, false, ret->clear());
        ret->push_back(ccvalue);
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool seval_to_Data(const se::Value &v, cc::Data *ret) {
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject() && v.toObject()->isTypedArray(), false, "Convert parameter to Data failed!");
    uint8_t *ptr    = nullptr;
    size_t   length = 0;
    bool     ok     = v.toObject()->getTypedArrayData(&ptr, &length);
    if (ok) {
        ret->copy(ptr, length);
    } else {
        ret->clear();
    }

    return ok;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool seval_to_DownloaderHints(const se::Value &v, cc::network::DownloaderHints *ret) {
    const static cc::network::DownloaderHints ZERO{0, 0, ""};
    assert(ret != nullptr);
    SE_PRECONDITION2(v.isObject(), false, "Convert parameter to DownloaderHints failed!");
    se::Value   tmp;
    se::Object *obj = v.toObject();
    bool        ok  = false;

    ok = obj->getProperty("countOfMaxProcessingTasks", &tmp);
    SE_PRECONDITION3(ok && tmp.isNumber(), false, *ret = ZERO);
    ret->countOfMaxProcessingTasks = tmp.toUint32();

    ok = obj->getProperty("timeoutInSeconds", &tmp);
    SE_PRECONDITION3(ok && tmp.isNumber(), false, *ret = ZERO);
    ret->timeoutInSeconds = tmp.toUint32();

    ok = obj->getProperty("tempFileNameSuffix", &tmp);
    SE_PRECONDITION3(ok && tmp.isString(), false, *ret = ZERO);
    ret->tempFileNameSuffix = tmp.toString();

    return ok;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::Vec4 *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Vec4 failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<float>(obj, to, "x", &cc::Vec4::x, tmp);
    set_member_field<float>(obj, to, "y", &cc::Vec4::y, tmp);
    set_member_field<float>(obj, to, "z", &cc::Vec4::z, tmp);
    set_member_field<float>(obj, to, "w", &cc::Vec4::w, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::Mat3 *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Matrix3 failed!");
    se::Object *obj = from.toObject();

    if (obj->isTypedArray()) {
        // typed array
        SE_PRECONDITION2(obj->isTypedArray(), false, "Convert parameter to Matrix3 failed!");
        size_t   length = 0;
        uint8_t *ptr    = nullptr;
        obj->getTypedArrayData(&ptr, &length);

        memcpy(to->m, ptr, length);
    } else {
        bool        ok = false;
        se::Value   tmp;
        std::string prefix = "m";
        for (uint32_t i = 0; i < 9; ++i) {
            std::string name;
            if (i < 10) {
                name = prefix + "0" + std::to_string(i);
            } else {
                name = prefix + std::to_string(i);
            }
            ok = obj->getProperty(name.c_str(), &tmp, true);
            SE_PRECONDITION3(ok, false, *to = cc::Mat3::IDENTITY);

            if (tmp.isNumber()) {
                to->m[i] = tmp.toFloat();
            } else {
                SE_REPORT_ERROR("%u, not supported type in matrix", i);
                *to = cc::Mat3::IDENTITY;
                return false;
            }

            tmp.setUndefined();
        }
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::Mat4 *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Matrix4 failed!");
    se::Object *obj = from.toObject();

    if (obj->isTypedArray()) {
        // typed array
        SE_PRECONDITION2(obj->isTypedArray(), false, "Convert parameter to Matrix4 failed!");

        size_t   length = 0;
        uint8_t *ptr    = nullptr;
        obj->getTypedArrayData(&ptr, &length);

        memcpy(to->m, ptr, length);
    } else {
        bool        ok = false;
        se::Value   tmp;
        std::string prefix = "m";
        for (uint32_t i = 0; i < 16; ++i) {
            std::string name;
            if (i < 10) {
                name = prefix + "0" + std::to_string(i);
            } else {
                name = prefix + std::to_string(i);
            }
            ok = obj->getProperty(name.c_str(), &tmp, true);
            SE_PRECONDITION3(ok, false, *to = cc::Mat4::IDENTITY);

            if (tmp.isNumber()) {
                to->m[i] = tmp.toFloat();
            } else {
                SE_REPORT_ERROR("%u, not supported type in matrix", i);
                *to = cc::Mat4::IDENTITY;
                return false;
            }

            tmp.setUndefined();
        }
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::Vec3 *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Vec3 failed!");

    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<float>(obj, to, "x", &cc::Vec3::x, tmp);
    set_member_field<float>(obj, to, "y", &cc::Vec3::y, tmp);
    set_member_field<float>(obj, to, "z", &cc::Vec3::z, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::Color *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Color failed!");

    se::Object *obj = from.toObject();
    se::Value   t;
    set_member_field<uint8_t>(obj, to, "r", &cc::Color::r, t);
    set_member_field<uint8_t>(obj, to, "g", &cc::Color::g, t);
    set_member_field<uint8_t>(obj, to, "b", &cc::Color::b, t);
    set_member_field<uint8_t>(obj, to, "a", &cc::Color::a, t);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::Vec2 *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Vec2 failed!");

    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<float>(obj, to, "x", &cc::Vec2::x, tmp);
    set_member_field<float>(obj, to, "y", &cc::Vec2::y, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::Size *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Size failed!");

    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<float>(obj, to, "width", &cc::Size::width, tmp);
    set_member_field<float>(obj, to, "height", &cc::Size::height, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::Quaternion *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Quaternion failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<float>(obj, to, "x", &cc::Quaternion::x, tmp);
    set_member_field<float>(obj, to, "y", &cc::Quaternion::y, tmp);
    set_member_field<float>(obj, to, "z", &cc::Quaternion::z, tmp);
    set_member_field<float>(obj, to, "w", &cc::Quaternion::w, tmp);
    return true;
}

//////////////////// geometry

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::geometry::AABB *to, se::Object * /*ctx*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to AABB failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<cc::Vec3>(obj, to, "halfExtents", &cc::geometry::AABB::halfExtents, tmp);
    set_member_field<cc::Vec3>(obj, to, "center", &cc::geometry::AABB::center, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::geometry::Capsule *to, se::Object * /*ctx*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Capsule failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<float>(obj, to, "radius", &cc::geometry::Capsule::radius, tmp);
    set_member_field<float>(obj, to, "halfHeight", &cc::geometry::Capsule::halfHeight, tmp);
    set_member_field<cc::geometry::Capsule::CenterEnum>(obj, to, "axis", &cc::geometry::Capsule::axis, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::geometry::Line *to, se::Object * /*ctx*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Line failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<cc::Vec3>(obj, to, "s", &cc::geometry::Line::s, tmp);
    set_member_field<cc::Vec3>(obj, to, "e", &cc::geometry::Line::e, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::geometry::Ray *to, se::Object * /*ctx*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Sphere failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<cc::Vec3>(obj, to, "o", &cc::geometry::Ray::o, tmp);
    set_member_field<cc::Vec3>(obj, to, "d", &cc::geometry::Ray::d, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::geometry::Sphere *to, se::Object * /*ctx*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Sphere failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    bool        ok;
    set_member_field<float>(obj, to, "radius", &cc::geometry::Sphere::setRadius, tmp);
    set_member_field<cc::Vec3>(obj, to, "center", &cc::geometry::Sphere::setCenter, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::geometry::Triangle *to, se::Object * /*ctx*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Plane failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<cc::Vec3>(obj, to, "a", &cc::geometry::Triangle::a, tmp);
    set_member_field<cc::Vec3>(obj, to, "b", &cc::geometry::Triangle::b, tmp);
    set_member_field<cc::Vec3>(obj, to, "c", &cc::geometry::Triangle::c, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::geometry::Plane *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Plane failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<cc::Vec3>(obj, to, "n", &cc::geometry::Plane::n, tmp);
    set_member_field<float>(obj, to, "d", &cc::geometry::Plane::d, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::geometry::Frustum *to, se::Object * /*unused*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to Frustum failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<std::array<cc::geometry::Plane, 6>>(obj, to, "planes", &cc::geometry::Frustum::planes, tmp);
    set_member_field<std::array<cc::Vec3, 8>>(obj, to, "vertices", &cc::geometry::Frustum::vertices, tmp);
    return true;
}

////////////////////////// scene info

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::scene::FogInfo *to, se::Object * /*ctx*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to FogInfo failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<cc::scene::FogType>(obj, to, "type", &cc::scene::FogInfo::setType, tmp);
    set_member_field<cc::Color>(obj, to, "fogColor", &cc::scene::FogInfo::setFogColor, tmp);
    set_member_field<bool>(obj, to, "enabled", &cc::scene::FogInfo::setEnabled, tmp);
    set_member_field<float>(obj, to, "fogDensity", &cc::scene::FogInfo::setFogDensity, tmp);
    set_member_field<float>(obj, to, "fogStart", &cc::scene::FogInfo::setFogStart, tmp);
    set_member_field<float>(obj, to, "fogEnd", &cc::scene::FogInfo::setFogEnd, tmp);
    set_member_field<float>(obj, to, "fogAtten", &cc::scene::FogInfo::setFogAtten, tmp);
    set_member_field<float>(obj, to, "fogTop", &cc::scene::FogInfo::setFogTop, tmp);
    set_member_field<float>(obj, to, "fogRange", &cc::scene::FogInfo::setFogRange, tmp);
    //TODO(PatriceJiang): covnert resource ??
    // set_member_field<cc::scene::Fog>(obj, to, "resource", &cc::scene::FogInfo::setResource, tmp);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::scene::ShadowsInfo *to, se::Object * /*ctx*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to ShadowInfo failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    set_member_field<cc::scene::ShadowType>(obj, to, "type", &cc::scene::ShadowsInfo::setType, tmp);
    set_member_field<bool>(obj, to, "enabled", &cc::scene::ShadowsInfo::setEnabled, tmp);
    set_member_field<cc::Vec3>(obj, to, "normal", &cc::scene::ShadowsInfo::setNormal, tmp);
    set_member_field<float>(obj, to, "distance", &cc::scene::ShadowsInfo::setDistance, tmp);
    set_member_field<cc::Color>(obj, to, "shadowColor", &cc::scene::ShadowsInfo::setShadowColor, tmp);
    set_member_field<bool>(obj, to, "autoAdapt", &cc::scene::ShadowsInfo::setAutoAdapt, tmp);
    set_member_field<cc::scene::PCFType>(obj, to, "pcf", &cc::scene::ShadowsInfo::setPcf, tmp);
    set_member_field<float>(obj, to, "bias", &cc::scene::ShadowsInfo::setBias, tmp);
    set_member_field<float>(obj, to, "normalBias", &cc::scene::ShadowsInfo::setNormalBias, tmp);
    set_member_field<float>(obj, to, "near", &cc::scene::ShadowsInfo::setNear, tmp);
    set_member_field<float>(obj, to, "far", &cc::scene::ShadowsInfo::setFar, tmp);
    set_member_field<float>(obj, to, "orthoSize", &cc::scene::ShadowsInfo::setOrthoSize, tmp);
    set_member_field<float>(obj, to, "maxReceived", &cc::scene::ShadowsInfo::setMaxReceived, tmp);
    set_member_field<float>(obj, to, "size", &cc::scene::ShadowsInfo::setShadowMapSize, tmp);
    set_member_field<float>(obj, to, "saturation", &cc::scene::ShadowsInfo::setSaturation, tmp);
    //TODO(PatriceJiang): covnert resource ??
    // set_member_field<cc::scene::Shadow>(obj, to, "resource", &cc::scene::ShadowInfo::setResource, tmp);

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::scene::SkyboxInfo *to, se::Object * /*ctx*/) {
    SE_PRECONDITION2(from.isObject(), false, "Convert parameter to ShadowInfo failed!");
    se::Object *obj = from.toObject();
    se::Value   tmp;
    //TODO(PatriceJiang): export TextureCube
    // set_member_field<cc::TextureCube*>(obj, to, "envmap", &cc::scene::SkyboxInfo::setEnvmap, tmp);
    set_member_field<bool>(obj, to, "isRGBE", &cc::scene::SkyboxInfo::setRGBE, tmp);
    set_member_field<bool>(obj, to, "enabled", &cc::scene::SkyboxInfo::setEnabled, tmp);
    set_member_field<bool>(obj, to, "useIBL", &cc::scene::SkyboxInfo::setUseIBL, tmp);
    return true;
}

// std::variant<int32_t, float, bool, std::string>;
// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::MacroValue *to, se::Object * /*ctx*/) {
    if (from.isBoolean()) {
        *to = from.toBoolean();
    } else if (from.isNumber()) {
        //cjh TODO: how to check it's a int32_t or float?
        *to = from.toFloat();
    } else if (from.isString()) {
        *to = from.toString();
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, std::vector<cc::MacroRecord> *to, se::Object * /*ctx*/) {
    if (from.isNullOrUndefined()) {
        to->clear();
        return true;
    }

    SE_PRECONDITION2(from.isObject(), false, "sevalue_to_native(std::vector<cc::MacroRecord>), not an object");
    auto *fromObj = from.toObject();
    CC_ASSERT(fromObj->isArray());
    uint32_t len = 0;
    bool     ok  = fromObj->getArrayLength(&len);
    if (ok) {
        to->resize(len);
        se::Value arrElement;
        for (uint32_t i = 0; i < len; ++i) {
            ok = fromObj->getArrayElement(i, &arrElement);
            if (!ok || !arrElement.isObject()) {
                continue;
            }
            cc::MacroRecord          macroRecord;
            std::vector<std::string> keys;
            ok = arrElement.toObject()->getAllKeys(&keys);
            if (ok) {
                se::Value seMacroVal;
                for (const auto &key : keys) {
                    ok = arrElement.toObject()->getProperty(key, &seMacroVal);
                    cc::MacroValue macroVal;
                    sevalue_to_native(seMacroVal, &macroVal, nullptr);
                    macroRecord.emplace(key, std::move(macroVal));
                }
            }
            (*to)[i] = std::move(macroRecord);
        }
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::MaterialProperty *to, se::Object *ctx) {
    if (from.isNullOrUndefined()) {
        *to = std::monostate();
        return true;
    }

    //TODO(PatriceJiang): float/int32_t from js number
    if (from.isNumber()) {
        double v = from.toDouble();
        if (std::trunc(v) != v) {
            *to = static_cast<float>(v);
        } else {
            *to = static_cast<int32_t>(v);
        }
        return true;
    }

    if (from.isObject()) {
        auto *obj = const_cast<se::Object *>(from.toObject());
        bool  hasX;
        bool  hasY;
        bool  hasZ;
        bool  hasW;
        bool  hasEuler;
        bool  hasM01;
        bool  hasM08;
        bool  hasM15;
        bool  hasAssetID;
        bool  hasColorVal;

        se::Value tmp0;
        se::Value tmp1;
        se::Value tmp2;
        se::Value tmp3;
        se::Value tmp4;

        hasColorVal = obj->getProperty("_val", &tmp0, true);
        if (hasColorVal) {
            *to = cc::Color{tmp0.toUint32()};
            return true;
        }

        hasX     = obj->getProperty("x", &tmp0, true);
        hasY     = hasX && obj->getProperty("y", &tmp1, true);
        hasZ     = hasY && obj->getProperty("z", &tmp2, true);
        hasW     = hasZ && obj->getProperty("w", &tmp3, true);
        hasEuler = hasW && obj->getProperty("getEulerAngles", &tmp4, true);

        if (hasW) {
            if (hasEuler) {
                *to = cc::Quaternion(tmp0.toFloat(), tmp1.toFloat(), tmp2.toFloat(), tmp3.toFloat());
            } else {
                *to = cc::Vec4(tmp0.toFloat(), tmp1.toFloat(), tmp2.toFloat(), tmp3.toFloat());
            }
            return true;
        }

        if (hasZ) {
            *to = cc::Vec3(tmp0.toFloat(), tmp1.toFloat(), tmp2.toFloat());
            return true;
        }

        if (hasY) {
            *to = cc::Vec2(tmp0.toFloat(), tmp1.toFloat());
            return true;
        }

        hasM01 = obj->getProperty("m00", &tmp0, true);
        hasM08 = hasM01 && obj->getProperty("m08", &tmp1, true);
        hasM15 = hasM08 && obj->getProperty("m15", &tmp2, true);

        if (hasM15) {
            cc::Mat4 m4;
            sevalue_to_native(from, &m4, ctx);
            *to = m4;
            return true;
        }

        if (hasM08) {
            cc::Mat3 m3;
            sevalue_to_native(from, &m3, ctx);
            *to = m3;
            return true;
        }

        hasAssetID = obj->getProperty("_id", &tmp3, true);
        if (hasAssetID) {
            *to = reinterpret_cast<cc::TextureBase *>(obj->getPrivateData());
            return true;
        }

        // TODO: optimize the the performance?
        if (obj->_getClass() != nullptr) {
            if (0 == strcmp(obj->_getClass()->getName(), "Texture2D")) {
                *to = reinterpret_cast<cc::Texture2D *>(obj->getPrivateData());
                return true;
            }

            if (0 == strcmp(obj->_getClass()->getName(), "TextureCube")) {
                *to = reinterpret_cast<cc::TextureCube *>(obj->getPrivateData());
                return true;
            }
        }

        // gfx::Texture?
        *to = reinterpret_cast<cc::gfx::Texture *>(obj->getPrivateData());
        return true;
    }

    return false;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::IPreCompileInfoValueType *to, se::Object *ctx) {
    se::Object *obj = from.toObject();
    SE_PRECONDITION2(obj->isArray(), false, "faild to convert to IPreCompileInfoValueType");

    uint32_t len;
    obj->getArrayLength(&len);
    if (len == 0) {
        //TODO(PatriceJiang): judge type of empty array?
        *to = std::vector<bool>{};
        return false;
    }

    se::Value firstEle;
    obj->getArrayElement(0, &firstEle);
    if (firstEle.isBoolean()) {
        std::vector<bool> result;
        sevalue_to_native(from, &result, ctx);
        return true;
    }
    if (firstEle.isNumber()) {
        std::vector<float> result;
        sevalue_to_native(from, &result, ctx);
        return true;
    }
    if (firstEle.isString()) {
        std::vector<std::string> result;
        sevalue_to_native(from, &result, ctx);
        return true;
    }

    return false;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, std::variant<std::vector<float>, std::string> *to, se::Object * /*ctx*/) {
    if (from.isObject() && from.toObject()->isArray()) {
        uint32_t           len = 0;
        bool               ok  = from.toObject()->getArrayLength(&len);
        std::vector<float> arr;
        arr.resize(len);
        for (uint32_t i = 0; i < len; ++i) {
            se::Value e;
            ok = from.toObject()->getArrayElement(i, &e);
            if (ok) {
                if (e.isNumber()) {
                    arr[i] = e.toFloat();
                }
            }
        }
        *to = std::move(arr);
    } else if (from.isString()) {
        *to = from.toString();
    } else {
        CC_ASSERT(false);
    }
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, std::variant<std::monostate, cc::MaterialProperty, cc::MaterialPropertyList> *to, se::Object *ctx) {
    bool ok = false;
    if (from.isObject() && from.toObject()->isArray()) {
        cc::MaterialPropertyList propertyList{};
        ok = sevalue_to_native(from, &propertyList, ctx);
        if (ok) {
            *to = std::move(propertyList);
        }
    } else {
        cc::MaterialProperty property;
        ok = sevalue_to_native(from, &property, ctx);
        if (ok) {
            *to = std::move(property);
        }
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::ArrayBuffer *to, se::Object * /*ctx*/) {
    assert(from.isObject());
    to->setJSArrayBuffer(from.toObject());
    return true;
}
// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, std::shared_ptr<cc::ArrayBuffer> *out, se::Object *ctx) {
    if (!*out) {
        *out = std::make_shared<cc::ArrayBuffer>();
    }
    sevalue_to_native(from, out->get(), ctx);
    //TODO(PatriceJiang): should not mix smart pointers with raw pointers.
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, std::vector<bool> *to, se::Object * /*ctx*/) {
    if (from.isNullOrUndefined()) {
        to->clear();
        return true;
    }

    se::Object *arr = from.toObject();
    uint32_t    size;
    se::Value   tmp;
    arr->getArrayLength(&size);
    to->resize(size);
    for (uint32_t i = 0; i < size; i++) {
        arr->getArrayElement(i, &tmp);
        (*to)[i] = tmp.toBoolean();
    }
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, std::vector<unsigned char> *to, se::Object * /*ctx*/) {
    if (from.isNullOrUndefined()) {
        to->clear();
        return true;
    }

    assert(from.isObject());
    se::Object *in = from.toObject();

    if (in->isTypedArray()) {
        uint8_t *data    = nullptr;
        size_t   dataLen = 0;
        in->getTypedArrayData(&data, &dataLen);
        to->resize(dataLen);
        to->assign(data, data + dataLen);
        return true;
    }

    if (in->isArrayBuffer()) {
        uint8_t *data    = nullptr;
        size_t   dataLen = 0;
        in->getArrayBufferData(&data, &dataLen);
        to->resize(dataLen);
        to->assign(data, data + dataLen);
        return true;
    }

    if (in->isArray()) {
        uint32_t len = 0;
        in->getArrayLength(&len);
        to->resize(len);
        se::Value ele;
        for (uint32_t i = 0; i < len; i++) {
            in->getArrayElement(i, &ele);
            (*to)[i] = ele.toUint8();
        }
        return true;
    }

    SE_LOGE("type error, ArrayBuffer/TypedArray/Array expected!");
    return false;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::TypedArray *to, se::Object * /*ctx*/) {
    std::visit(overloaded{[&](auto &typedArray) {
                              typedArray.setJSTypedArray(from.toObject());
                          },
                          [](std::monostate /*unused*/) {}},
               *to);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &from, cc::IBArray *to, se::Object * /*ctx*/) {
    std::visit([&](auto &typedArray) {
        typedArray.setJSTypedArray(from.toObject());
    },
               *to);

    return true;
}

#if USE_SPINE

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &val, spine::String *obj, se::Object * /*unused*/) {
    *obj = val.toString().data();
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool sevalue_to_native(const se::Value &v, spine::Vector<spine::String> *ret, se::Object * /*unused*/) {
    assert(v.isObject());
    se::Object *obj = v.toObject();
    assert(obj->isArray());

    bool     ok  = true;
    uint32_t len = 0;
    ok           = obj->getArrayLength(&len);
    if (!ok) {
        ret->clear();
        return false;
    }

    se::Value tmp;
    for (uint32_t i = 0; i < len; ++i) {
        ok = obj->getArrayElement(i, &tmp);
        if (!ok || !tmp.isObject()) {
            ret->clear();
            return false;
        }

        const char *str = tmp.toString().c_str();
        ret->add(str);
    }

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool seval_to_Map_string_key(const se::Value &v, cc::Map<std::string, cc::middleware::Texture2D *> *ret) {
    assert(ret != nullptr);
    assert(v.isObject());
    se::Object *obj = v.toObject();

    std::vector<std::string> allKeys;
    bool                     ok = obj->getAllKeys(&allKeys);
    if (!ok) {
        ret->clear();
        return false;
    }

    se::Value tmp;
    for (const auto &key : allKeys) {
        auto pngPos = key.find(".png");
        if (pngPos == std::string::npos) {
            continue;
        }

        ok = obj->getProperty(key.c_str(), &tmp);
        if (!ok || !tmp.isObject()) {
            ret->clear();
            return false;
        }

        auto *nativeObj = static_cast<cc::middleware::Texture2D *>(tmp.toObject()->getPrivateData());
        ret->insert(key, nativeObj);
    }

    return true;
}

////////////////////////////////////////////////////////////////////////////
/////////////////nativevalue_to_se//////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////

// native to seval

// NOLINTNEXTLINE(readability-identifier-naming)
bool ccvalue_to_seval(const cc::Value &v, se::Value *ret) {
    assert(ret != nullptr);
    bool ok = true;
    switch (v.getType()) {
        case cc::Value::Type::NONE:
            ret->setNull();
            break;
        case cc::Value::Type::UNSIGNED:
            ret->setUint32(v.asUnsignedInt());
            break;
        case cc::Value::Type::BOOLEAN:
            ret->setBoolean(v.asBool());
            break;
        case cc::Value::Type::FLOAT:
        case cc::Value::Type::DOUBLE:
            ret->setDouble(v.asDouble());
            break;
        case cc::Value::Type::INTEGER:
            ret->setInt32(v.asInt());
            break;
        case cc::Value::Type::STRING:
            ret->setString(v.asString());
            break;
        case cc::Value::Type::VECTOR:
            ok = ccvaluevector_to_seval(v.asValueVector(), ret);
            break;
        case cc::Value::Type::MAP:
            ok = ccvaluemap_to_seval(v.asValueMap(), ret);
            break;
        case cc::Value::Type::INT_KEY_MAP:
            ok = ccvaluemapintkey_to_seval(v.asIntKeyMap(), ret);
            break;
        default:
            SE_LOGE("Could not the way to convert cc::Value::Type (%d) type!", (int)v.getType());
            ok = false;
            break;
    }

    return ok;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool ccvaluemap_to_seval(const cc::ValueMap &v, se::Value *ret) {
    assert(ret != nullptr);

    se::HandleObject obj(se::Object::createPlainObject());
    bool             ok = true;
    for (const auto &e : v) {
        const std::string &key   = e.first;
        const cc::Value &  value = e.second;

        if (key.empty()) {
            continue;
        }

        se::Value tmp;
        if (!ccvalue_to_seval(value, &tmp)) {
            ok = false;
            ret->setUndefined();
            break;
        }

        obj->setProperty(key.c_str(), tmp);
    }
    if (ok) {
        ret->setObject(obj);
    }

    return ok;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool ccvaluemapintkey_to_seval(const cc::ValueMapIntKey &v, se::Value *ret) {
    assert(ret != nullptr);

    se::HandleObject obj(se::Object::createPlainObject());
    bool             ok = true;
    for (const auto &e : v) {
        std::stringstream keyss;
        keyss << e.first;
        std::string      key   = keyss.str();
        const cc::Value &value = e.second;

        if (key.empty()) {
            continue;
        }

        se::Value tmp;
        if (!ccvalue_to_seval(value, &tmp)) {
            ok = false;
            ret->setUndefined();
            break;
        }

        obj->setProperty(key.c_str(), tmp);
    }
    if (ok) {
        ret->setObject(obj);
    }

    return ok;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool ccvaluevector_to_seval(const cc::ValueVector &v, se::Value *ret) {
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createArrayObject(v.size()));
    bool             ok = true;

    uint32_t i = 0;
    for (const auto &value : v) {
        se::Value tmp;
        if (!ccvalue_to_seval(value, &tmp)) {
            ok = false;
            ret->setUndefined();
            break;
        }

        obj->setArrayElement(i, tmp);
        ++i;
    }
    if (ok) {
        ret->setObject(obj);
    }

    return ok;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool ManifestAsset_to_seval(const cc::extension::ManifestAsset &v, se::Value *ret) {
    assert(ret != nullptr);
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("md5", se::Value(v.md5));
    obj->setProperty("path", se::Value(v.path));
    obj->setProperty("compressed", se::Value(v.compressed));
    obj->setProperty("size", se::Value(v.size));
    obj->setProperty("downloadState", se::Value(v.downloadState));
    ret->setObject(obj);

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool Data_to_seval(const cc::Data &v, se::Value *ret) {
    assert(ret != nullptr);
    if (v.isNull()) {
        ret->setNull();
    } else {
        se::HandleObject obj(se::Object::createTypedArray(se::Object::TypedArrayType::UINT8, v.getBytes(), v.getSize()));
        ret->setObject(obj, true);
    }
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool DownloadTask_to_seval(const cc::network::DownloadTask &v, se::Value *ret) {
    assert(ret != nullptr);

    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("identifier", se::Value(v.identifier));
    obj->setProperty("requestURL", se::Value(v.requestURL));
    obj->setProperty("storagePath", se::Value(v.storagePath));
    ret->setObject(obj);

    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const spine::String &obj, se::Value &val, se::Object * /*unused*/) {
    val.setString(obj.buffer());
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const spine::Vector<spine::String> &v, se::Value &ret, se::Object * /*unused*/) {
    se::HandleObject obj(se::Object::createArrayObject(v.size()));
    bool             ok = true;

    spine::Vector<spine::String> tmpv = v;
    for (uint32_t i = 0, count = static_cast<uint32_t>(tmpv.size()); i < count; i++) {
        if (!obj->setArrayElement(i, se::Value(tmpv[i].buffer()))) {
            ok = false;
            ret.setUndefined();
            break;
        }
    }

    if (ok) {
        ret.setObject(obj);
    }

    return ok;
}
#endif

////////////////// custom types

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::Data &from, se::Value &to, se::Object * /*unused*/) {
    se::Object *buffer = se::Object::createArrayBufferObject(from.getBytes(), from.getSize());
    to.setObject(buffer);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::Value &from, se::Value &to, se::Object * /*unused*/) {
    return ccvalue_to_seval(from, &to);
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const std::unordered_map<std::string, cc::Value> &from, se::Value &to, se::Object * /*unused*/) {
    return ccvaluemap_to_seval(from, &to);
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::Vec4 &from, se::Value &to, se::Object * /*unused*/) {
    return Vec4_to_seval(from, &to);
}

// bool nativevalue_to_se(const cc::Vec2 &from, se::Value &to, se::Object * /*unused*/) {
//     return Vec2_to_seval(from, &to);
// }

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::Vec3 &from, se::Value &to, se::Object * /*unused*/) {
    return Vec3_to_seval(from, &to);
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::Size &from, se::Value &to, se::Object * /*unused*/) {
    return Size_to_seval(from, &to);
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::extension::ManifestAsset &from, se::Value &to, se::Object * /*unused*/) {
    return ManifestAsset_to_seval(from, &to);
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::Quaternion &from, se::Value &to, se::Object * /*ctx*/) {
    return Quaternion_to_seval(from, &to);
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::Rect &from, se::Value &to, se::Object * /*unused*/) {
    return Rect_to_seval(from, &to);
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::ArrayBuffer &arrayBuffer, se::Value &to, se::Object * /*ctx*/) {
    to.setObject(arrayBuffer.getJSArrayBuffer());
    return true;
}

//// NOLINTNEXTLINE(readability-identifier-naming)
//bool nativevalue_to_se(const cc::TypedArray &from, se::Value &to, se::Object * /*ctx*/) {
//    std::visit([&](auto &typedArray) {
//        to.setObject(typedArray.getJSTypedArray());
//    },
//               from);
//    return true;
//}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::NativeDep &from, se::Value &to, se::Object * /*ctx*/) {
    se::HandleObject obj(se::Object::createPlainObject());
    obj->setProperty("uuid", se::Value(from.uuid));
    obj->setProperty("ext", se::Value(from.ext));
    obj->setProperty("__isNative__", se::Value(from.__isNative__));
    to.setObject(obj);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::Mat3 &from, se::Value &to, se::Object * /*ctx*/) {
    se::HandleObject obj(se::Object::createPlainObject());
    char             keybuf[8] = {0};
    for (auto i = 0; i < 9; i++) {
        snprintf(keybuf, sizeof(keybuf), "m%02d", i);
        obj->setProperty(keybuf, se::Value(from.m[i]));
    }
    to.setObject(obj);
    return true;
}

// NOLINTNEXTLINE(readability-identifier-naming)
bool nativevalue_to_se(const cc::Mat4 &from, se::Value &to, se::Object * /*ctx*/) {
    se::HandleObject obj(se::Object::createPlainObject());
    char             keybuf[8] = {0};
    for (auto i = 0; i < 16; i++) {
        snprintf(keybuf, sizeof(keybuf), "m%02d", i);
        obj->setProperty(keybuf, se::Value(from.m[i]));
    }
    to.setObject(obj);
    return true;
}