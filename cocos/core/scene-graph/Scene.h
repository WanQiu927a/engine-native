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
#include "core/scene-graph/Node.h"

namespace cc {

namespace scene {
class RenderScene;
}

class SceneGlobals;

class Scene final : public Node {
public:
    using Super = Node;
    explicit Scene(const std::string &name);
    ~Scene() override;

    inline scene::RenderScene *getRenderScene() const { return _renderScene; }
    inline SceneGlobals *      getSceneGlobals() const { return _globals; };

    void load();
    void activate(bool active = true);

    void onBatchCreated(bool dontChildPrefab) override;
    bool destroy() override;

protected:
    /**
     * @en Per-scene level rendering info
     * @zh 场景级别的渲染信息
     */
    //    @serializable
    SceneGlobals *_globals{nullptr};

    scene::RenderScene *_renderScene{nullptr};
    bool                _inited{false};

    CC_DISALLOW_COPY_MOVE_ASSIGN(Scene);
};

} // namespace cc
