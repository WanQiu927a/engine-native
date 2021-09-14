/****************************************************************************
 Copyright (c) 2018 Xiamen Yaji Software Co., Ltd.

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

#include "SimpleDemo.h"
#include "3d/framework/MeshRenderer.h"
#include "3d/misc/CreateMesh.h"
#include "base/Log.h"
#include "core/assets/AssetsModuleHeader.h"
#include "core/builtin/BuiltinResMgr.h"
#include "core/components/CameraComponent.h"
#include "core/scene-graph/Node.h"
#include "renderer/GFXDeviceManager.h"

#include "core/scene-graph/Layers.h"
#include "primitive/Primitive.h"

using namespace cc;
using namespace cc::gfx;

void SimpleDemo::setup(int width, int height, uintptr_t windowHandle) {
    CC_LOG_INFO("SimpleDemo::%s, width: %d, height: %d", __FUNCTION__, width, height);

    // Initialize Device
    BindingMappingInfo bindingMappingInfo;
    bindingMappingInfo.bufferOffsets  = std::vector<int>{0, pipeline::globalUBOCount + pipeline::localUBOCount, pipeline::globalUBOCount};
    bindingMappingInfo.samplerOffsets = std::vector<int>{-pipeline::globalUBOCount, pipeline::globalSamplerCount + pipeline::localSamplerCount, pipeline::globalSamplerCount - pipeline::localUBOCount};
    bindingMappingInfo.flexibleSet    = 1;

    DeviceInfo info;
    info.windowHandle       = windowHandle;
    info.width              = width;
    info.height             = height;
    info.pixelRatio         = 1;
    info.bindingMappingInfo = bindingMappingInfo;
    _device                 = DeviceManager::create(info);

    // Initialize Root
    _root = new Root(_device);
    _root->initialize();
    _root->resize(width, height);
    _root->setRenderPipeline(nullptr);

    BuiltinResMgr::getInstance()->initBuiltinRes(_device);

    BuiltinResMgr::getInstance()->tryCompileAllPasses();
    //

    // Scene
    _scene = new Scene("myscene");
    // add a node to scene
    auto *node = new Node("mynode");
    node->setParent(_scene);

    // create mesh asset
    auto *cube = new Primitive(PrimitiveType::BOX);

    // create mesh renderer
    _cubeMeshRenderer = node->addComponent<MeshRenderer>();
    _cubeMeshRenderer->setMesh(cube);

    // create camera
    auto *cameraComp = node->addComponent<Camera>();
    cameraComp->setProjection(Camera::ProjectionType::PERSPECTIVE);
    cameraComp->setPriority(0);
    cameraComp->setFov(45);
    cameraComp->setFovAxis(Camera::FOVAxis::VERTICAL);
    cameraComp->setOrthoHeight(10);
    cameraComp->setNear(1);
    cameraComp->setFar(1000);
    cameraComp->setClearColor(cc::Color{51, 51, 51, 255});
    cameraComp->setClearDepth(1);
    cameraComp->setClearStencil(0);
    cameraComp->setClearFlags(Camera::ClearFlag::SOLID_COLOR);
    cameraComp->setRect(cc::Rect(0, 0, 1, 1));
    cameraComp->setAperture(Camera::Aperture::F16_0);
    cameraComp->setShutter(Camera::Shutter::D125);
    cameraComp->setIso(Camera::ISO::ISO100);
    cameraComp->setScreenScale(1.0f);
    cameraComp->setVisibility(static_cast<uint32_t>(Layers::LayerList::IGNORE_RAYCAST | Layers::LayerList::UI_3D | Layers::LayerList::DEFAULT));

    // set material
    auto *material = new Material();
    material->initialize({.effectName = "unlit",
                          .defines    = MacroRecord{{"USE_COLOR", true}}});
    material->setProperty("mainColor", cc::Color{255, 0, 255, 255});
    _cubeMeshRenderer->setMaterial(material);

    // simulate logic in director.ts
    _scene->load();
    _scene->activate();

    // simulate component lifecycle
    cameraComp->onLoad();
    cameraComp->onEnable();
    _cubeMeshRenderer->onLoad();
    _cubeMeshRenderer->onEnable();
}

void SimpleDemo::step(float dt) {
    //    CC_LOG_INFO("SimpleDemo::%s, dt: %.06f", __FUNCTION__, dt);
    _cubeMeshRenderer->update(dt);

    _root->frameMove(dt);
}

void SimpleDemo::finalize() {
    CC_LOG_INFO("SimpleDemo::%s", __FUNCTION__);
    CC_SAFE_DELETE(_cubeMeshRenderer);
    CC_SAFE_DELETE(_scene);
    CC_SAFE_DELETE(_root);
    DeviceManager::destroy();
}
