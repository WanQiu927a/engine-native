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

#include "base/Ptr.h"
#include "core/assets/Asset.h"
#include "core/scene-graph/Scene.h"

namespace cc {

class Scene;

class SceneAsset final : public Asset {
public:
    using Super            = Asset;
    SceneAsset()           = default;
    ~SceneAsset() override = default;

    void initDefault(const cc::optional<std::string> &uuid) override;

    bool validate() const override {
        return _scene.get() != nullptr;
    }

    inline Scene *getScene() const { return _scene.get(); }
    inline void   setScene(Scene *scene) { _scene = scene; };

private:
    /**
     * @en The scene node
     * @zh 场景节点。

    @editable
    @serializable*/
    SharedPtr<Scene> _scene;

    CC_DISALLOW_COPY_MOVE_ASSIGN(SceneAsset);
};

} // namespace cc
