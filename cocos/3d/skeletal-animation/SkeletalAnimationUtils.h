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
#include <optional>
#include <unordered_map>
#include "3d/assets/Mesh.h"
#include "3d/assets/Skeleton.h"
#include "core/geometry/AABB.h"
#include "core/scene-graph/Node.h"
#include "gfx-base/GFXDef-common.h"
#include "renderer/core/TextureBufferPool.h"
#include "renderer/gfx-base/GFXBuffer.h"
#include "renderer/gfx-base/GFXDevice.h"

namespace cc {
struct IChunkContent {
    int32_t skeleton{0}; // int or uint or float?
    int32_t clip{0};     // int or uint?
};

struct ICustomJointTextureLayout {
    uint32_t                   textureLength{0};
    std::vector<IChunkContent> contents;
};

struct IInternalJointAnimInfo {
    std::optional<Mat4>              downstream{std::nullopt};         // downstream default pose, if present
    std::optional<std::vector<Mat4>> curveData{std::nullopt};          // the nearest animation curve, if present
    index_t                          bindposeIdx{0};                   // index of the actual bindpose to use
    std::optional<Mat4>              bindposeCorrection{std::nullopt}; // correction factor from the original bindpose
};

struct IJointTextureHandle {
    uint32_t                                                  pixelOffset{0};
    uint32_t                                                  refCount{0};
    uint64_t                                                  clipHash{0};
    uint64_t                                                  skeletonHash{0};
    bool                                                      readyToBeDeleted{false};
    const ITextureBufferHandle &                              handle;
    std::unordered_map<uint32_t, std::vector<geometry::AABB>> bounds;
    std::optional<std::vector<IInternalJointAnimInfo>>        animInfos{std::nullopt};
};

class JointTexturePool {
public:
    JointTexturePool() = default;
    explicit JointTexturePool(gfx::Device *device);
    ~JointTexturePool() = default;

    inline float getPixelsPerJoint() const { return _pixelsPerJoint; }

    void clear();

    void registerCustomTextureLayouts(const std::vector<ICustomJointTextureLayout> &layouts);

    /**
     * @en
     * Get joint texture for the default pose.
     * @zh
     * 获取默认姿势的骨骼贴图。
     */
    const IJointTextureHandle &getDefaultPoseTexture(Skeleton *skeleton, Mesh *mesh, scenegraph::Node *skinningRoot) const;

    /**
     * @en
     * Get joint texture for the specified animation clip.
     * @zh
     * 获取指定动画片段的骨骼贴图。
     */
    const IJointTextureHandle &getSequencePoseTexture(Skeleton *skeleton, Mesh *mesh, scenegraph::Node *skinningRoot) const;

    void releaseHandle(const IJointTextureHandle &handle);

    void releaseSkeleton(Skeleton *skeleton);

    // void releaseAnimationClip (AnimationClip* clip); // TODO(xwx): AnimationClip not define

private:
    // const IInternalJointAnimInfo &createAnimInfos(Skeleton *skeleton, AnimationClip *clip, scenegraph::Node *skinningRoot); // TODO(xwx): AnimationClip not define

    gfx::Device *                                     _device{nullptr};
    TextureBufferPool *                               _pool{nullptr};
    std::unordered_map<uint32_t, IJointTextureHandle> _textureBuffers;
    uint32_t                                          _formatSize{0};
    float                                             _pixelsPerJoint{0}; // int or float?
    TextureBufferPool *                               _customPool{nullptr};
    std::unordered_map<uint64_t, index_t>             _chunkIdxMap; // hash -> chunkIdx

    CC_DISALLOW_COPY_MOVE_ASSIGN(JointTexturePool);
};

struct IAnimInfo {
    gfx::Buffer *buffer{nullptr};
    Float32Array data;
    bool         dirty{false};
};

class JointAnimationInfo {
public:
    JointAnimationInfo();
    explicit JointAnimationInfo(gfx::Device *device);
    ~JointAnimationInfo() = default;

    const IAnimInfo &getData(std::string nodeID = "-1") const;
    void             destroy(std::string nodeID);
    const IAnimInfo &switchChip(const IAnimInfo &info /*, AnimationClip *clip */);
    void             clear();

private:
    std::unordered_map<std::string, IAnimInfo> _pool; // pre node
    gfx::Device *                              _device{nullptr};

    CC_DISALLOW_COPY_MOVE_ASSIGN(JointAnimationInfo);
};

} // namespace cc