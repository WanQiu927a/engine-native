/****************************************************************************
 Copyright (c) 2020-2021 Xiamen Yaji Software Co., Ltd.

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

#include <array>
#include <vector>

#include "Define.h"
#include "RenderPipeline.h"
#include "SceneCulling.h"
#include "core/geometry/AABB.h"
#include "core/geometry/Sphere.h"
#include "core/scene-graph/Node.h"
#include "gfx-base/GFXBuffer.h"
#include "gfx-base/GFXDescriptorSet.h"
#include "math/Quaternion.h"
#include "platform/Application.h"
#include "scene/DirectionalLight.h"
#include "scene/Light.h"
#include "scene/RenderScene.h"
#include "scene/SpotLight.h"

namespace cc {
namespace pipeline {
bool           castBoundsInitialized = false;
geometry::AABB castWorldBounds;

RenderObject genRenderObject(const scene::Model *model, const scene::Camera *camera) {
    float depth = 0;
    if (model->getNode()) {
        auto *const node = model->getTransform();
        cc::Vec3    position;
        cc::Vec3::subtract(node->getWorldPosition(), camera->getPosition(), &position);
        depth = position.dot(camera->getForward());
    }

    return {depth, model};
}

void getShadowWorldMatrix(const geometry::Sphere *sphere, const cc::Quaternion &rotation, const cc::Vec3 &dir, cc::Mat4 *shadowWorldMat, cc::Vec3 *out) {
    Vec3 translation(dir);
    translation.negate();
    const auto distance = sphere->getRadius() * COEFFICIENT_OF_EXPANSION;
    translation.scale(distance);
    translation.add(sphere->getCenter());
    out->set(translation);

    Mat4::fromRT(rotation, translation, shadowWorldMat);
}

void updateDirLight(scene::Shadow *shadow, const scene::Light *light, std::array<float, UBOShadow::COUNT> *shadowUBO) {
    auto *const node     = light->getNode();
    const auto &rotation = node->getWorldRotation();
    Quaternion  qt(rotation.x, rotation.y, rotation.z, rotation.w);
    Vec3        forward(0, 0, -1.0F);
    forward.transformQuat(qt);
    const auto &normal   = shadow->getNormal();
    const auto  distance = shadow->getDistance() + 0.001F; // avoid z-fighting
    const auto  ndL      = normal.dot(forward);
    const auto  scale    = 1.0F / ndL;
    const auto  lx       = forward.x * scale;
    const auto  ly       = forward.y * scale;
    const auto  lz       = forward.z * scale;
    const auto  nx       = normal.x;
    const auto  ny       = normal.y;
    const auto  nz       = normal.z;
    //TODO: how to avoid create Mat4 every time?
    auto matLight  = shadow->getMatLight();
    matLight.m[0]  = 1 - nx * lx;
    matLight.m[1]  = -nx * ly;
    matLight.m[2]  = -nx * lz;
    matLight.m[3]  = 0;
    matLight.m[4]  = -ny * lx;
    matLight.m[5]  = 1 - ny * ly;
    matLight.m[6]  = -ny * lz;
    matLight.m[7]  = 0;
    matLight.m[8]  = -nz * lx;
    matLight.m[9]  = -nz * ly;
    matLight.m[10] = 1 - nz * lz;
    matLight.m[11] = 0;
    matLight.m[12] = lx * distance;
    matLight.m[13] = ly * distance;
    matLight.m[14] = lz * distance;
    matLight.m[15] = 1;

    memcpy(shadowUBO->data() + UBOShadow::MAT_LIGHT_PLANE_PROJ_OFFSET, matLight.m, sizeof(matLight));
    memcpy(shadowUBO->data() + UBOShadow::SHADOW_COLOR_OFFSET, shadow->getShadowColor4f().data(), sizeof(float) * 4);
}

void lightCollecting(scene::Camera *camera, std::vector<const scene::Light *> *validLights) {
    validLights->clear();
    auto *              sphere    = CC_NEW(geometry::Sphere);
    const auto *        scene     = camera->getScene();
    const scene::Light *mainLight = scene->getMainLight();
    validLights->emplace_back(mainLight);

    for (auto &spotLight : scene->getSpotLights()) {
        sphere->setCenter(spotLight->getPosition());
        sphere->setRadius(spotLight->getRange());
        if (sphere->interset(camera->getFrustum())) {
            validLights->emplace_back(static_cast<scene::Light *>(spotLight));
        }
    }

    CC_SAFE_DELETE(sphere);
}

void sceneCulling(RenderPipeline *pipeline, scene::Camera *camera) {
    auto *const       sceneData = pipeline->getPipelineSceneData();
    auto *const       shadow    = sceneData->getShadow();
    auto *const       skyBox    = sceneData->getSkybox();
    const auto *const scene     = camera->getScene();

    castBoundsInitialized = false;
    RenderObjectList shadowObjects;
    bool             isShadowMap = false;
    if (shadow != nullptr && shadow->isEnabled() && shadow->getType() == scene::ShadowType::SHADOW_MAP) {
        isShadowMap = true;
    }

    RenderObjectList renderObjects;

    if (skyBox != nullptr && skyBox->isEnabled() && skyBox->getModel() && (static_cast<uint32_t>(camera->getClearFlag()) & skyboxFlag)) {
        renderObjects.emplace_back(genRenderObject(skyBox->getModel(), camera));
    }

    for (const auto &model : scene->getModels()) {
        // filter model by view visibility
        if (model->isEnabled()) {
            const auto        visibility = camera->getVisibility();
            const auto *const node       = model->getNode();
            if ((model->getNode() && ((visibility & node->getLayer()) == node->getLayer())) ||
                (visibility & static_cast<uint>(model->getVisFlags()))) {
                // shadow render Object
                const auto *modelWorldBounds = model->getWorldBounds();
                if (isShadowMap && model->isCastShadow() && modelWorldBounds) {
                    if (!castBoundsInitialized) {
                        castWorldBounds.set(modelWorldBounds->getCenter(), modelWorldBounds->getHalfExtents());
                        castBoundsInitialized = true;
                    }
                    castWorldBounds.merge(*modelWorldBounds);
                    shadowObjects.emplace_back(genRenderObject(model, camera));
                }
                // frustum culling
                if (modelWorldBounds && !modelWorldBounds->aabbFrustum(camera->getFrustum())) {
                    continue;
                }

                renderObjects.emplace_back(genRenderObject(model, camera));
            }
        }
    }

    if (isShadowMap) {
        sceneData->getSphere()->define(castWorldBounds);
        sceneData->setShadowObjects(std::move(shadowObjects));
    }

    sceneData->setRenderObjects(std::move(renderObjects));
}

} // namespace pipeline
} // namespace cc
