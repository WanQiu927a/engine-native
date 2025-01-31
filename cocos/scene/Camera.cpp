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

#include "scene/Camera.h"
#include <vector>
#include "core/Root.h"
#include "math/MathUtil.h"
#include "renderer/gfx-base/GFXDevice.h"
#include "renderer/pipeline/Define.h"

namespace cc {
namespace scene {

namespace {
std::array<Mat4, 4> correctionMatrices;

void assignMat4(Mat4 &mat4, float m0, float m1, float m2, float m3, float m4, float m5) {
    mat4.m[0] = m0;
    mat4.m[1] = m1;
    mat4.m[2] = m2;
    mat4.m[3] = m3;
    mat4.m[4] = m4;
    mat4.m[5] = m5;
}

constexpr std::array<std::array<float, 4>, 4> PRE_TRANSFORMS = {{
    {{1, 0, 0, 1}},   // SurfaceTransform.IDENTITY
    {{0, 1, -1, 0}},  // SurfaceTransform.ROTATE_90
    {{-1, 0, 0, -1}}, // SurfaceTransform.ROTATE_180
    {{0, -1, 1, 0}}   // SurfaceTransform.ROTATE_270
}};

} // namespace

const std::vector<float> Camera::FSTOPS{1.8F, 2.0F, 2.2F, 2.5F, 2.8F, 3.2F, 3.5F, 4.0F, 4.5F, 5.0F, 5.6F, 6.3F, 7.1F, 8.0F, 9.0F, 10.0F, 11.0F, 13.0F, 14.0F, 16.0F, 18.0F, 20.0F, 22.0F};
const std::vector<float> Camera::SHUTTERS{1.0F, 1.0F / 2.0F, 1.0F / 4.0F, 1.0F / 8.0F, 1.0F / 15.0F, 1.0F / 30.0F, 1.0F / 60.0F, 1.0F / 125.0F,
                                          1.0F / 250.0F, 1.0F / 500.0F, 1.0F / 1000.0F, 1.0F / 2000.0F, 1.0F / 4000.0F};
const std::vector<float> Camera::ISOS{100.0F, 200.0F, 400.0F, 800.0F};

Camera::Camera(gfx::Device *device)
: _device(device) {
    _apertureValue = Camera::FSTOPS.at(static_cast<int>(_aperture));
    _shutterValue  = Camera::SHUTTERS.at(static_cast<int>(_shutter));
    _isoValue      = Camera::ISOS[static_cast<int>(_iso)];

    _aspect = _screenScale = 1.F;

    if (correctionMatrices.empty()) {
        float ySign = _device->getCapabilities().clipSpaceSignY;
        assignMat4(correctionMatrices[static_cast<int>(gfx::SurfaceTransform::IDENTITY)], 1.F, 0, 0, 0, 0, ySign);
        assignMat4(correctionMatrices[static_cast<int>(gfx::SurfaceTransform::ROTATE_90)], 0, 1.F, 0, 0, -ySign, 0);
        assignMat4(correctionMatrices[static_cast<int>(gfx::SurfaceTransform::ROTATE_180)], -1, 0, 0, 0, 0, -ySign);
        assignMat4(correctionMatrices[static_cast<int>(gfx::SurfaceTransform::ROTATE_270)], 0, -1, 0, 0, ySign, 0);
    }
}

bool Camera::initialize(const ICameraInfo &info) {
    _node       = info.node;
    _width      = 1.F;
    _height     = 1.F;
    _clearFlag  = gfx::ClearFlagBit::NONE;
    _clearDepth = 1.0F;
    _visibility = pipeline::CAMERA_DEFAULT_MASK;
    _name       = info.name;
    _proj       = info.projection;
    _priority   = info.priority;
    _aspect = _screenScale = 1.F;
    updateExposure();
    changeTargetWindow(info.window);
    return true;
}

void Camera::destroy() {
    if (_window) {
        _window->detachCamera(this);
        _window = nullptr;
    }
    _name.clear();
}

void Camera::attachToScene(RenderScene *scene) {
    _enabled = true;
    _scene   = scene;
}

void Camera::detachFromScene() {
    _enabled = false;
    _scene   = nullptr;
}

void Camera::resize(uint32_t width, uint32_t height) {
    if (!_window) {
        return;
    }

    _width       = width;
    _height      = height;
    _aspect      = (static_cast<float>(width) * _viewport.z) / (static_cast<float>(height) * _viewport.w);
    _isProjDirty = true;
}

void Camera::setFixedSize(uint32_t width, uint32_t height) {
    _width        = width;
    _height       = height;
    _aspect       = (static_cast<float>(width) * _viewport.z) / (static_cast<float>(height) * _viewport.w);
    _isWindowSize = false;
}

void Camera::update(bool forceUpdate /*false*/) {
    if (!_node) {
        return;
    }

    bool viewProjDirty = false;
    // view matrix
    if (_node->getChangedFlags() || forceUpdate) {
        _matView = _node->getWorldMatrix().getInversed();
        _forward.set(-_matView.m[2], -_matView.m[6], -_matView.m[10]);

        _position.set(_node->getWorldPosition());
        viewProjDirty = true;
    }

    // projection matrix
    gfx::SurfaceTransform orientation = _device->getSurfaceTransform();
    if (_isProjDirty || _curTransform != orientation) {
        _curTransform               = orientation;
        const float projectionSignY = _device->getCapabilities().clipSpaceSignY;
        // Only for rendertexture processing
        if (_window && _window->hasOffScreenAttachments()) {
            orientation = gfx::SurfaceTransform::IDENTITY;
        }
        if (_proj == CameraProjection::PERSPECTIVE) {
            Mat4::createPerspective(_fov, _aspect, _nearClip, _farClip,
                                    _fovAxis == CameraFOVAxis::VERTICAL, _device->getCapabilities().clipSpaceMinZ, projectionSignY, static_cast<int>(orientation), &_matProj);
        } else {
            const float x = _orthoHeight * _aspect; // aspect is already oriented
            const float y = _orthoHeight;
            Mat4::createOrthographicOffCenter(-x, x, -y, y, _nearClip, _farClip,
                                              _device->getCapabilities().clipSpaceMinZ, projectionSignY,
                                              static_cast<int>(orientation), &_matProj);
        }
        _matProjInv   = _matProj.getInversed();
        viewProjDirty = true;
        _isProjDirty  = false;
    }

    // view-projection
    if (viewProjDirty) {
        Mat4::multiply(_matProj, _matView, &_matViewProj);
        _matViewProjInv = _matViewProj.getInversed();
        _frustum.update(_matViewProj, _matViewProjInv);
    }
}
void Camera::changeTargetWindow(RenderWindow *window) {
    if (_window) {
        _window->detachCamera(this);
    }
    RenderWindow *win = window ? window : Root::getInstance()->getMainWindow();
    if (win) {
        win->attachCamera(this);
        _window = win;
        resize(win->getWidth(), win->getHeight());
    }
}

geometry::Ray *Camera::screenPointToRay(geometry::Ray *out, float x, float y) {
    if (!_node) {
        return nullptr;
    }

    const float                 cx           = _viewport.x * static_cast<float>(_width);
    const float                 cy           = _viewport.y * static_cast<float>(_height);
    const float                 cw           = _viewport.z * static_cast<float>(_width);
    const float                 ch           = _viewport.w * static_cast<float>(_height);
    const bool                  isProj       = _proj == CameraProjection::PERSPECTIVE;
    const float                 ySign        = _device->getCapabilities().clipSpaceSignY;
    const std::array<float, 4> &preTransform = PRE_TRANSFORMS[static_cast<int>(_curTransform)];

    Vec3 tmpVec3{
        (x - cx) / cw * 2 - 1.F,
        (y - cy) / ch * 2 - 1.F,
        isProj ? 1.F : -1.F};
    tmpVec3.x = tmpVec3.x * preTransform[0] + tmpVec3.y * preTransform[2] * ySign;
    tmpVec3.y = tmpVec3.x * preTransform[1] + tmpVec3.y * preTransform[3] * ySign;

    if (isProj) {
        tmpVec3.transformMat4(tmpVec3, _matViewProjInv);
    } else {
        out->o.transformMat4(tmpVec3, _matViewProjInv);
    }

    if (isProj) {
        // camera origin
        geometry::Ray::fromPoints(out, _node->getWorldPosition(), tmpVec3);
    } else {
        out->d.set(0, 0, -1.F);
        out->d.transformQuat(_node->getRotation());
    }

    return out;
}

const Vec3 &Camera::screenToWorld(Vec3 &out, const Vec3 &screenPos) {
    const float                 cx           = _viewport.x * static_cast<float>(_width);
    const float                 cy           = _viewport.y * static_cast<float>(_height);
    const float                 cw           = _viewport.z * static_cast<float>(_width);
    const float                 ch           = _viewport.w * static_cast<float>(_height);
    const float                 ySign        = _device->getCapabilities().clipSpaceSignY;
    const std::array<float, 4> &preTransform = PRE_TRANSFORMS[static_cast<int>(_curTransform)];

    if (_proj == CameraProjection::PERSPECTIVE) {
        // calculate screen pos in far clip plane
        out.set(
            (screenPos.x - cx) / cw * 2 - 1,
            (screenPos.y - cy) / ch * 2 - 1,
            1.0F);

        // transform to world
        out.x = out.x * preTransform[0] + out.y * preTransform[2] * ySign;
        out.y = out.x * preTransform[1] + out.y * preTransform[3] * ySign;
        _matViewProjInv.transformPoint(&out);

        // lerp to depth z
        Vec3 tmpVec3;
        if (_node) {
            tmpVec3.set(_node->getWorldPosition());
        }

        out = out.lerp(tmpVec3, MathUtil::lerp(_nearClip / _farClip, 1, screenPos.z));
    } else {
        out.set(
            (screenPos.x - cx) / cw * 2 - 1,
            (screenPos.y - cy) / ch * 2 - 1,
            screenPos.z * 2 - 1);

        // transform to world
        out.x = out.x * preTransform[0] + out.y * preTransform[2] * ySign;
        out.y = out.x * preTransform[1] + out.y * preTransform[3] * ySign;
        _matViewProjInv.transformPoint(&out);
    }

    return out;
}

const Vec3 &Camera::worldToScreen(Vec3 &out, const Vec3 &worldPos) {
    const float                 cx           = _viewport.x * static_cast<float>(_width);
    const float                 cy           = _viewport.y * static_cast<float>(_height);
    const float                 cw           = _viewport.z * static_cast<float>(_width);
    const float                 ch           = _viewport.w * static_cast<float>(_height);
    const float                 ySign        = _device->getCapabilities().clipSpaceSignY;
    const std::array<float, 4> &preTransform = PRE_TRANSFORMS[static_cast<int>(_curTransform)];

    _matViewProj.transformPoint(const_cast<Vec3 *>(&worldPos));

    out.x = out.x * preTransform[0] + out.y * preTransform[2] * ySign;
    out.y = out.x * preTransform[1] + out.y * preTransform[3] * ySign;

    out.x = cx + (out.x + 1) * 0.5F * cw;
    out.y = cy + (out.y + 1) * 0.5F * ch;
    out.z = out.z * 0.5F + 0.5F;

    return out;
}

const Mat4 &Camera::worldMatrixToScreen(Mat4 &out, const Mat4 &worldMatrix, uint32_t width, uint32_t height) {
    Mat4::multiply(_matViewProj, worldMatrix, &out);
    Mat4::multiply(correctionMatrices[static_cast<int>(_curTransform)], out, &out);

    const float halfWidth  = static_cast<float>(width) / 2;
    const float halfHeight = static_cast<float>(height) / 2;
    Mat4        tmpMat4(Mat4::IDENTITY);
    tmpMat4.translate(halfWidth, halfHeight, 0);
    tmpMat4.scale(halfWidth, halfHeight, 1);

    out.multiply(tmpMat4);

    return out;
}

void Camera::setExposure(float ev100) {
    _exposure = 0.833333F / std::pow(2.0F, ev100);
}

void Camera::updateExposure() {
    const float ev100 = std::log2((_apertureValue * _apertureValue) / _shutterValue * 100.F / _isoValue);
    setExposure(ev100);
}

void Camera::setViewport(const Vec4 &val) {
    const float x      = val.x;
    const float width  = val.z;
    const float height = val.w;

    const float y = _device->getCapabilities().clipSpaceSignY < 0 ? 1 - val.y - height : val.y;

    switch (_device->getSurfaceTransform()) {
        case gfx::SurfaceTransform::ROTATE_90:
            _viewport.x = 1 - y - height;
            _viewport.y = x;
            _viewport.z = height;
            _viewport.w = width;
            break;
        case gfx::SurfaceTransform::ROTATE_180:
            _viewport.x = 1 - x - width;
            _viewport.y = 1 - y - height;
            _viewport.z = width;
            _viewport.w = height;
            break;
        case gfx::SurfaceTransform::ROTATE_270:
            _viewport.x = y;
            _viewport.y = 1 - x - width;
            _viewport.z = height;
            _viewport.w = width;
            break;
        case gfx::SurfaceTransform::IDENTITY:
            _viewport.x = x;
            _viewport.y = y;
            _viewport.z = width;
            _viewport.w = height;
            break;
        default:
            break;
    }

    resize(_width, _height);
}

void Camera::detachCamera() {
    if (_window) {
        _window->detachCamera(this);
    }
}

} // namespace scene
} // namespace cc
