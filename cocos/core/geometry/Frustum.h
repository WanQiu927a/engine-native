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

#include <array>
#include "core/geometry/Enums.h"
#include "core/geometry/Plane.h"
#include "math/Mat4.h"
#include "math/Vec3.h"

namespace cc {
namespace geometry {

class Frustum final : public ShapeBase {
public:
    /**
     * @en
     * Create a ortho frustum.
     * @zh
     * 创建一个正交视锥体。
     * @param out 正交视锥体。
     * @param width 正交视锥体的宽度。
     * @param height 正交视锥体的高度。
     * @param near 正交视锥体的近平面值。
     * @param far 正交视锥体的远平面值。
     * @param transform 正交视锥体的变换矩阵。
     * @return {Frustum} frustum.
     */
    static void createOrtho(Frustum *out, float width,
                            float       height,
                            float       near,
                            float       far,
                            const Mat4 &transform);

    /**
     * @en
     * create a new frustum.
     * @zh
     * 创建一个新的截锥体。
     * @return {Frustum} frustum.
     */
    static Frustum *create() {
        return new Frustum();
    }

    /**
     * @en
     * Clone a frustum.
     * @zh
     * 克隆一个截锥体。
     */
    static Frustum *clone(const Frustum &f) {
        return Frustum::copy(new Frustum(), f);
    }

    /**
     * @en
     * Copy the values from one frustum to another.
     * @zh
     * 从一个截锥体拷贝到另一个截锥体。
     */
    static Frustum *copy(Frustum *out, const Frustum &f) {
        out->setType(f.getType());
        out->planes   = f.planes;
        out->vertices = f.vertices;
        return out;
    }

    /**
     * @en
     * Set whether to use accurate intersection testing function on this frustum.
     * @zh
     * 设置是否在此截锥体上使用精确的相交测试函数。
     */
    void setAccurate(bool accurate) {
        setType(accurate ? ShapeEnum::SHAPE_FRUSTUM_ACCURATE : ShapeEnum::SHAPE_FRUSTUM);
    }

    Frustum() {
        setType(ShapeEnum::SHAPE_FRUSTUM);
    }

    /**
     * @en
     * Transform this frustum.
     * @zh
     * 变换此截锥体。
     * @param mat 变换矩阵。
     */
    void transform(const Mat4 &);

    std::array<Vec3, 8>  vertices;
    std::array<Plane, 6> planes;
    void                 update(const Mat4 &m, const Mat4 &inv);
};

} // namespace geometry
} // namespace cc
