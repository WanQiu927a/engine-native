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

#include "bindings/auto/jsb_assets_auto.h"
#include "jsb_scene_manual.h"

#ifndef JSB_ALLOC
    #define JSB_ALLOC(kls, ...) new (std::nothrow) kls(__VA_ARGS__)
#endif

#ifndef JSB_FREE
    #define JSB_FREE(ptr) delete ptr
#endif

static bool js_assets_Asset_getNativeDep(se::State &s) // NOLINT(readability-identifier-naming)
{
    auto *cobj = SE_THIS_OBJECT<cc::Asset>(s);
    SE_PRECONDITION2(cobj, false, "js_assets_Asset_getNativeDep : Invalid Native Object");
    const auto &   args = s.args();
    size_t         argc = args.size();
    CC_UNUSED bool ok   = true;
    if (argc == 0) {
        ok = nativevalue_to_se(cobj->getNativeDep(), s.rval());
        SE_PRECONDITION2(ok, false, "js_assets_Asset_getNativeDep : Error processing arguments");
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_PROP_GET(js_assets_Asset_getNativeDep) // NOLINT(readability-identifier-naming)

static bool js_assets_ImageAsset_getData(se::State &s) // NOLINT(readability-identifier-naming)
{
    auto *cobj = SE_THIS_OBJECT<cc::ImageAsset>(s);
    SE_PRECONDITION2(cobj, false, "js_assets_ImageAsset_getData : Invalid Native Object");
    
    s.rval().setUint64(reinterpret_cast<intptr_t>(cobj->getData()));
    return false;
}
SE_BIND_PROP_GET(js_assets_ImageAsset_getData) // NOLINT(readability-identifier-naming)

static bool js_assets_ImageAsset_setData(se::State &s) // NOLINT(readability-identifier-naming)
{
    auto *cobj = SE_THIS_OBJECT<cc::ImageAsset>(s);
    SE_PRECONDITION2(cobj, false, "js_assets_Asset_setData : Invalid Native Object");
    const auto &   args = s.args();
    size_t         argc = args.size();
    if (argc == 1) {
        uint8_t *data{nullptr};
        if (args[0].isObject()) {
            args[0].toObject()->getTypedArrayData(&data, nullptr);
        } else {
            data = reinterpret_cast<uint8_t *>(args[0].asPtr());
        }
        cobj->setData(data);
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 0);
    return false;
}
SE_BIND_FUNC(js_assets_ImageAsset_setData) // NOLINT(readability-identifier-naming)

bool register_all_assets_manual(se::Object *obj) // NOLINT(readability-identifier-naming)
{
    // Get the ns
    se::Value nsVal;
    if (!obj->getProperty("jsb", &nsVal)) {
        se::HandleObject jsobj(se::Object::createPlainObject());
        nsVal.setObject(jsobj);
        obj->setProperty("jsb", nsVal);
    }

    __jsb_cc_Asset_proto->defineProperty("_nativeDep", _SE(js_assets_Asset_getNativeDep), nullptr);
    __jsb_cc_ImageAsset_proto->defineFunction("setData", _SE(js_assets_ImageAsset_setData));


    return true;
}
