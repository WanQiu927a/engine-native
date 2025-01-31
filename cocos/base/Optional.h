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

#ifdef USE_CXX_17

    #include <optional>

namespace cc {

template <class T>
using optional = std::optional<T>;

using nullopt_t = std::nullopt_t;
inline constexpr nullopt_t nullopt{nullopt_t::__secret_tag{}, nullopt_t::__secret_tag{}};

}; // namespace cc
#else
    #include "boost/none.hpp"
    #include "boost/optional.hpp"

namespace cc {

template <typename T>
class optional : public boost::optional<T> {
public:
    using boost::optional<T>::optional;

    template <typename U, typename = std::enable_if_t<std::is_assignable<T&, U>::value> >
    optional(U&& val) {
        this->emplace_assign(val);
    }

    template <typename U, typename = std::enable_if_t<std::is_assignable<T&, U>::value> >
    optional(const optional<U>& val) {
        this->emplace_assign(val);
    }
};

using nullopt_t = boost::none_t;

inline const nullopt_t nullopt((boost::none_t::init_tag()));

}; // namespace cc

#endif
