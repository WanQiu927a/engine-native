#include "core/builtin/BuiltinResMgr.h"
#include "core/assets/EffectAsset.h"
#include "core/assets/ImageAsset.h"
#include "core/assets/Material.h"
#include "core/assets/Texture2D.h"
#include "core/assets/TextureCube.h"
#include "core/builtin/Effects.h"
#include "core/builtin/ShaderSourceAssembly.h"
#include "core/data/deserializer/AssetDeserializerFactory.h"
#include "math/Color.h"
#include "platform/Image.h"
#include "rapidjson/document.h"
#include "renderer/core/ProgramLib.h"
#include "scene/Pass.h"

namespace cc {

namespace {
BuiltinResMgr *instance = nullptr;

constexpr uint8_t BLACK_IMAGE_RGBA_DATA_2X2[2 * 2 * 4] = {
    // r, g, b, a
    0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF,
    0x00, 0x00, 0x00, 0xFF};

constexpr uint8_t GREY_IMAGE_RGBA_DATA_2X2[2 * 2 * 4] = {
    0x07, 0x07, 0x07, 0xFF,
    0x07, 0x07, 0x07, 0xFF,
    0x07, 0x07, 0x07, 0xFF,
    0x07, 0x07, 0x07, 0xFF};

constexpr uint8_t WHITE_IMAGE_RGBA_DATA_2X2[2 * 2 * 4] = {
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF};

constexpr uint8_t NORMAL_IMAGE_RGBA_DATA_2X2[2 * 2 * 4] = {
    0x7F, 0xFF, 0x7F, 0xFF,
    0x7F, 0xFF, 0x7F, 0xFF,
    0x7F, 0xFF, 0x7F, 0xFF,
    0x7F, 0xFF, 0x7F, 0xFF};

constexpr uint8_t EMPTY_IMAGE_RGBA_DATA_2X2[2 * 2 * 4] = {
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00};

const uint8_t DEFAULT_IMAGE_RGBA_DATA_16X16[16 * 16 * 4] = {
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF,
    0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF, 0x55, 0x55, 0x55, 0xFF};

} // namespace

/* static */
BuiltinResMgr *BuiltinResMgr::getInstance() {
    if (instance == nullptr) {
        instance = new BuiltinResMgr(); //cjh how to release it?
    }
    return instance;
}

bool BuiltinResMgr::initBuiltinRes(gfx::Device *device) {
    // black texture
    initTexture2DWithUuid("black-texture", BLACK_IMAGE_RGBA_DATA_2X2, sizeof(BLACK_IMAGE_RGBA_DATA_2X2), 2, 2, 4);

    // empty texture
    initTexture2DWithUuid("empty-texture", EMPTY_IMAGE_RGBA_DATA_2X2, sizeof(EMPTY_IMAGE_RGBA_DATA_2X2), 2, 2, 4);

    // grey texture
    initTexture2DWithUuid("grey-texture", GREY_IMAGE_RGBA_DATA_2X2, sizeof(GREY_IMAGE_RGBA_DATA_2X2), 2, 2, 4);

    // white texture
    initTexture2DWithUuid("white-texture", WHITE_IMAGE_RGBA_DATA_2X2, sizeof(WHITE_IMAGE_RGBA_DATA_2X2), 2, 2, 4);

    // normal texture
    initTexture2DWithUuid("normal-texture", NORMAL_IMAGE_RGBA_DATA_2X2, sizeof(NORMAL_IMAGE_RGBA_DATA_2X2), 2, 2, 4);

    // default texture
    initTexture2DWithUuid("default-texture", DEFAULT_IMAGE_RGBA_DATA_16X16, sizeof(DEFAULT_IMAGE_RGBA_DATA_16X16), 16, 16, 4);

    // white cube texture
    initTextureCubeWithUuid("white-cube-texture", WHITE_IMAGE_RGBA_DATA_2X2, sizeof(WHITE_IMAGE_RGBA_DATA_2X2), 2, 2, 4);

    // default cube texture
    initTextureCubeWithUuid("default-cube-texture", DEFAULT_IMAGE_RGBA_DATA_16X16, sizeof(DEFAULT_IMAGE_RGBA_DATA_16X16), 16, 16, 4);

    //cjh TODO:    if (SpriteFrame) {
    //        const spriteFrame = new SpriteFrame() as SpriteFrame;
    //        const image = imgAsset;
    //        const texture = new Texture2D();
    //        texture.image = image;
    //        spriteFrame.texture = texture;
    //        spriteFrame._uuid = 'default-spriteframe";
    //        resources[spriteFrame._uuid] = spriteFrame;
    //    }
    //
    const char *shaderVersionKey = getDeviceShaderVersion(device);
    if (nullptr == shaderVersionKey || 0 == strlen(shaderVersionKey)) {
        CC_LOG_ERROR("Failed to initialize builtin shaders: unknown device.");
        return false;
    }
    //
    const ShaderSource *shaderSources = nullptr;
    if (const auto iter = ShaderSourceAssembly::get().find(shaderVersionKey); iter != ShaderSourceAssembly::get().cend()) {
        shaderSources = iter->second;
    }

    if (nullptr == shaderSources) {
        CC_LOG_ERROR("Current device is requiring builtin shaders of version %s, but shaders of that version are not assembled in this build.", shaderVersionKey);
        return false;
    }
    //
    //    return Promise.resolve().then(() => {

    rapidjson::Document doc;
    doc.Parse(builtinEffects.value().c_str());

    index_t         effectIndex       = 0;
    rapidjson::Type type              = doc.GetType();
    auto            assetDeserializer = AssetDeserializerFactory::createAssetDeserializer(DeserializeAssetType::EFFECT);
    for (const auto &e : doc.GetArray()) {
        auto *effect = new EffectAsset();
        assetDeserializer->deserialize(e, effect);

        index_t shaderIndex = 0;
        for (auto &shaderInfo : effect->shaders) {
            const auto &shaderSource = (*shaderSources)[effectIndex][shaderIndex];
            if (!shaderSource.empty()) {
                if (0 == strcmp(shaderVersionKey, "glsl1")) {
                    shaderInfo.glsl1.vert = shaderSource.at("vert").value();
                    shaderInfo.glsl1.frag = shaderSource.at("frag").value();
                } else if (0 == strcmp(shaderVersionKey, "glsl3")) {
                    shaderInfo.glsl3.vert = shaderSource.at("vert").value();
                    shaderInfo.glsl3.frag = shaderSource.at("frag").value();
                } else if (0 == strcmp(shaderVersionKey, "glsl4")) {
                    shaderInfo.glsl4.vert = shaderSource.at("vert").value();
                    shaderInfo.glsl4.frag = shaderSource.at("frag").value();
                }
            }
            ++shaderIndex;
        }

        effect->hideInEditor = true;
        effect->onLoaded();

        ++effectIndex;
    }

    initMaterials();

    return true;
}

void BuiltinResMgr::initMaterials() {
    auto &resources = _resources;

    // standard material
    auto *standardMtl = new Material(); //cjh TODO: how to release?
    standardMtl->setUuid("standard-material");
    standardMtl->initialize({
        .effectName = "standard",
    });
    resources[standardMtl->getUuid()] = standardMtl;
    _materialsToBeCompiled.emplace_back(standardMtl);

    // material indicating missing effect (yellow)
    auto *missingEfxMtl = new Material();
    missingEfxMtl->setUuid("missing-effect-material");
    missingEfxMtl->initialize({.effectName = "unlit",
                               .defines    = MacroRecord{{"USE_COLOR", true}}});
    missingEfxMtl->setProperty("mainColor", Color{255, 255, 0, 255}); // #ffff00;
    resources[missingEfxMtl->getUuid()] = missingEfxMtl;
    _materialsToBeCompiled.emplace_back(missingEfxMtl);

    // material indicating missing material (purple)
    auto *missingMtl = new Material();
    missingMtl->setUuid("missing-material");
    missingMtl->initialize({.effectName = "unlit",
                            .defines    = MacroRecord{{"USE_COLOR", true}}});
    missingMtl->setProperty("mainColor", Color{255, 0, 255, 255}); // #ff00ff
    resources[missingMtl->getUuid()] = missingMtl;
    _materialsToBeCompiled.emplace_back(missingMtl);

    auto *clearStencilMtl = new Material();
    clearStencilMtl->setUuid("default-clear-stencil");
    clearStencilMtl->initialize({.effectName = "clear-stencil",
                                 .defines    = MacroRecord{{"USE_TEXTURE", false}}});
    resources[clearStencilMtl->getUuid()] = clearStencilMtl;
    _materialsToBeCompiled.emplace_back(clearStencilMtl);

    // sprite material
    auto *spriteMtl = new Material();
    spriteMtl->setUuid("ui-base-material");
    spriteMtl->initialize({.effectName = "sprite",
                           .defines    = MacroRecord{{"USE_TEXTURE", false}}});
    resources[spriteMtl->getUuid()] = spriteMtl;
    _materialsToBeCompiled.emplace_back(spriteMtl);

    // sprite material
    auto *spriteColorMtl = new Material();
    spriteColorMtl->setUuid("ui-sprite-material");
    spriteColorMtl->initialize({.effectName = "sprite",
                                .defines    = MacroRecord{{"USE_TEXTURE", true}, {"CC_USE_EMBEDDED_ALPHA", false}, {"IS_GRAY", false}}});
    resources[spriteColorMtl->getUuid()] = spriteColorMtl;
    _materialsToBeCompiled.emplace_back(spriteColorMtl);

    // sprite alpha test material
    auto *alphaTestMaskMtl = new Material();
    alphaTestMaskMtl->setUuid("ui-alpha-test-material");
    alphaTestMaskMtl->initialize({.effectName = "sprite",
                                  .defines    = MacroRecord{{"USE_TEXTURE", true}, {"USE_ALPHA_TEST", true}, {"CC_USE_EMBEDDED_ALPHA", false}, {"IS_GRAY", false}}});
    resources[alphaTestMaskMtl->getUuid()] = alphaTestMaskMtl;
    _materialsToBeCompiled.emplace_back(alphaTestMaskMtl);

    // sprite gray material
    auto *spriteGrayMtl = new Material();
    spriteGrayMtl->setUuid("ui-sprite-gray-material");
    spriteGrayMtl->initialize({.effectName = "sprite",
                               .defines    = MacroRecord{{"USE_TEXTURE", true}, {"CC_USE_EMBEDDED_ALPHA", false}, {"IS_GRAY", true}}});
    resources[spriteGrayMtl->getUuid()] = spriteGrayMtl;
    _materialsToBeCompiled.emplace_back(spriteGrayMtl);

    // sprite alpha material
    auto *spriteAlphaMtl = new Material();
    spriteAlphaMtl->setUuid("ui-sprite-alpha-sep-material");
    spriteAlphaMtl->initialize({.effectName = "sprite",
                                .defines    = MacroRecord{{"USE_TEXTURE", true}, {"CC_USE_EMBEDDED_ALPHA", true}, {"IS_GRAY", false}}});
    resources[spriteAlphaMtl->getUuid()] = spriteAlphaMtl;
    _materialsToBeCompiled.emplace_back(spriteAlphaMtl);

    // sprite alpha & gray material
    auto *spriteAlphaGrayMtl = new Material();
    spriteAlphaGrayMtl->setUuid("ui-sprite-gray-alpha-sep-material");
    spriteAlphaGrayMtl->initialize({.effectName = "sprite",
                                    .defines    = MacroRecord{{"USE_TEXTURE", true}, {"CC_USE_EMBEDDED_ALPHA", true}, {"IS_GRAY", true}}});
    resources[spriteAlphaGrayMtl->getUuid()] = spriteAlphaGrayMtl;
    _materialsToBeCompiled.emplace_back(spriteAlphaGrayMtl);

    // ui graphics material
    auto *defaultGraphicsMtl = new Material();
    defaultGraphicsMtl->setUuid("ui-graphics-material");
    defaultGraphicsMtl->initialize({.effectName = "graphics"});
    resources[defaultGraphicsMtl->getUuid()] = defaultGraphicsMtl;
    _materialsToBeCompiled.emplace_back(defaultGraphicsMtl);

    // default particle material
    auto *defaultParticleMtl = new Material();
    defaultParticleMtl->setUuid("default-particle-material");
    defaultParticleMtl->initialize({.effectName = "particle"});
    resources[defaultParticleMtl->getUuid()] = defaultParticleMtl;
    _materialsToBeCompiled.emplace_back(defaultParticleMtl);

    // default particle gpu material
    auto *defaultParticleGPUMtl = new Material();
    defaultParticleGPUMtl->setUuid("default-particle-gpu-material");
    defaultParticleGPUMtl->initialize({.effectName = "particle-gpu"});
    resources[defaultParticleGPUMtl->getUuid()] = defaultParticleGPUMtl;
    _materialsToBeCompiled.emplace_back(defaultParticleGPUMtl);

    // default particle material
    auto *defaultTrailMtl = new Material();
    defaultTrailMtl->setUuid("default-trail-material");
    defaultTrailMtl->initialize({.effectName = "particle-trail"});
    resources[defaultTrailMtl->getUuid()] = defaultTrailMtl;
    _materialsToBeCompiled.emplace_back(defaultTrailMtl);

    // default particle material
    auto *defaultBillboardMtl = new Material();
    defaultBillboardMtl->setUuid("default-billboard-material");
    defaultBillboardMtl->initialize({.effectName = "billboard"});
    resources[defaultBillboardMtl->getUuid()] = defaultBillboardMtl;
    _materialsToBeCompiled.emplace_back(defaultBillboardMtl);

    // ui spine two color material
    auto *spineTwoColorMtl = new Material();
    spineTwoColorMtl->setUuid("default-spine-material");
    spineTwoColorMtl->initialize({.effectName = "spine",
                                  .defines    = MacroRecord{
                                      {"USE_TEXTURE", true},
                                      {"CC_USE_EMBEDDED_ALPHA", false},
                                      {"IS_GRAY", false},
                                  }});
    resources[spineTwoColorMtl->getUuid()] = spineTwoColorMtl;
    _materialsToBeCompiled.emplace_back(spineTwoColorMtl);
    //
    //cjh TODO:    game.on(Game.EVENT_GAME_INITED, () => {
    // tryCompileAllPasses();
    //    });
}

void BuiltinResMgr::tryCompileAllPasses() {
    for (auto &mat : _materialsToBeCompiled) {
        for (auto &pass : mat->getPasses()) {
            pass->tryCompile();
        }
    }
}

void BuiltinResMgr::initTexture2DWithUuid(const std::string &uuid, const uint8_t *data, size_t dataBytes, uint32_t width, uint32_t height, uint32_t bytesPerPixel) {
    auto *image = new (std::nothrow) Image();
    if (image != nullptr) {
        image->initWithRawData(data, dataBytes, width, height, bytesPerPixel);
        auto *texture = new (std::nothrow) Texture2D();
        texture->setUuid(uuid);

        auto *imgAsset = new (std::nothrow) ImageAsset();
        imgAsset->setNativeAsset(image); //cjh HOW TO RELEASE?
        texture->setImage(imgAsset);

        texture->initialize();
        _resources.emplace(texture->getUuid(), texture);

        image->release();
    }
}

void BuiltinResMgr::initTextureCubeWithUuid(const std::string &uuid, const uint8_t *data, size_t dataBytes, uint32_t width, uint32_t height, uint32_t bytesPerPixel) {
    auto *image = new (std::nothrow) Image();
    if (image != nullptr) {
        image->initWithRawData(data, dataBytes, width, height, bytesPerPixel);
        auto *texture = new (std::nothrow) TextureCube();
        texture->setUuid(uuid);
        texture->setMipFilter(TextureCube::Filter::NEAREST);

        ITextureCubeMipmap mipmap;
        mipmap.front = new ImageAsset(); //cjh HOW TO RELEASE?
        mipmap.front->setNativeAsset(image);
        mipmap.back = new ImageAsset();
        mipmap.back->setNativeAsset(image);
        mipmap.left = new ImageAsset();
        mipmap.left->setNativeAsset(image);
        mipmap.right = new ImageAsset();
        mipmap.right->setNativeAsset(image);
        mipmap.top = new ImageAsset();
        mipmap.top->setNativeAsset(image);
        mipmap.bottom = new ImageAsset();
        mipmap.bottom->setNativeAsset(image);

        texture->setImage(mipmap);

        texture->initialize();
        _resources.emplace(texture->getUuid(), texture);

        image->release();
    }
}

} // namespace cc
