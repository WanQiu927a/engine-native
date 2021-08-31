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

#include "scene/Model.h"
namespace cc {
namespace scene {
class MorphModel : public Model {
public:
    MorphModel()                   = default;
    MorphModel(const MorphModel &) = delete;
    MorphModel(MorphModel &&)      = delete;
    ~MorphModel() override         = default;
    MorphModel &operator=(const MorphModel &) = delete;
    MorphModel &operator=(MorphModel &&) = delete;

    std::vector<IMacroPatch> getMacroPatches(index_t subModelIndex) const override;
    void                     initSubModel(index_t idx, RenderingSubMesh *subMeshData, Material mat) override;
    void                     destroy() override;
    void                     setSubModelMaterial(int idx, Material &mat) override;
    // void setMorphRendering(MorphRenderingInstance* morphRendering);

protected:
    void updateLocalDescriptors(index_t subModelIndex, gfx::DescriptorSet *descriptorSet) const override;

private:
    Material launderMaterial(const Material &material) const;
    // MorphRenderingInstance *_morphRenderingInstance;
};
} // namespace scene
} // namespace cc