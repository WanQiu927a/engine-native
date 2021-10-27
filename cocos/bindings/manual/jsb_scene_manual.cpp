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

#include "jsb_scene_manual.h"
#include "bindings/auto/jsb_scene_auto.h"
#include "core/event/EventTypesToJS.h"
#include "core/scene-graph/Node.h"
#include "core/scene-graph/NodeEvent.h"
#include "scene/Model.h"

#ifndef JSB_ALLOC
    #define JSB_ALLOC(kls, ...) new (std::nothrow) kls(__VA_ARGS__)
#endif

#ifndef JSB_FREE
    #define JSB_FREE(ptr) delete ptr
#endif

static bool js_scene_RenderScene_updateBatches(se::State &s) // NOLINT(readability-identifier-naming)
{
    auto *cobj = SE_THIS_OBJECT<cc::scene::RenderScene>(s);
    SE_PRECONDITION2(cobj, false, "js_scene_RenderScene_updateBatches : Invalid Native Object");
    const auto &   args = s.args();
    size_t         argc = args.size();
    CC_UNUSED bool ok   = true;
    if (argc == 1) {
        HolderType<std::vector<cc::scene::DrawBatch2D *>, false> arg0 = {};
        ok &= sevalue_to_native(args[0], &arg0, s.thisObject());
        SE_PRECONDITION2(ok, false, "js_scene_RenderScene_updateBatches : Error processing arguments");
        cobj->updateBatches(std::move(arg0.value()));
        return true;
    }
    SE_REPORT_ERROR("wrong number of arguments: %d, was expecting %d", (int)argc, 1);
    return false;
}
SE_BIND_FUNC(js_scene_RenderScene_updateBatches) // NOLINT(readability-identifier-naming)

static bool js_root_registerListeners(se::State &s) // NOLINT(readability-identifier-naming)
{
    auto *cobj = SE_THIS_OBJECT<cc::Root>(s);
    SE_PRECONDITION2(cobj, false, "js_root_registerListeners : Invalid Native Object");

#define ROOT_DISPATCH_EVENT_TO_JS(eventType, jsFuncName)                                         \
    cobj->getEventProcessor()->on(eventType, [](cc::Root *rootObj) {                             \
        se::AutoHandleScope hs;                                                                  \
        se::Value           rootVal;                                                             \
        bool                ok = nativevalue_to_se(rootObj, rootVal);                            \
        SE_PRECONDITION2_VOID(ok, "js_root_registerListeners : Error processing arguments");     \
        if (rootVal.isObject()) {                                                                \
            se::Value funcVal;                                                                   \
            ok = rootVal.toObject()->getProperty(#jsFuncName, &funcVal) && funcVal.isObject();   \
            SE_PRECONDITION2_VOID(ok, "js_root_registerListeners : Error processing arguments"); \
            funcVal.toObject()->call(se::EmptyValueArray, rootVal.toObject());                   \
        }                                                                                        \
    })

    ROOT_DISPATCH_EVENT_TO_JS(cc::EventTypesToJS::ROOT_BATCH2D_INIT, _onBatch2DInit);
    ROOT_DISPATCH_EVENT_TO_JS(cc::EventTypesToJS::ROOT_BATCH2D_UPDATE, _onBatch2DUpdate);
    ROOT_DISPATCH_EVENT_TO_JS(cc::EventTypesToJS::ROOT_BATCH2D_UPLOAD_BUFFERS, _onBatch2DUploadBuffers);
    ROOT_DISPATCH_EVENT_TO_JS(cc::EventTypesToJS::ROOT_BATCH2D_RESET, _onBatch2DReset);

    return true;
}
SE_BIND_FUNC(js_root_registerListeners) // NOLINT(readability-identifier-naming)

static void registerOnTransformChanged(cc::Node *node, se::Object *jsObject) {
    node->on(
        cc::NodeEventType::TRANSFORM_CHANGED,
        [jsObject](cc::TransformBit transformBit) {
            se::AutoHandleScope hs;
            se::Value           funcVal;
            jsObject->getProperty("_onTransformChanged", &funcVal);
            SE_PRECONDITION2_VOID(funcVal.isObject() && funcVal.toObject()->isFunction(), "Not function named _onTransformChanged.");

            se::ValueArray args;
            se::Value      arg0;
            nativevalue_to_se(transformBit, arg0);
            args.push_back(arg0);
            funcVal.toObject()->call(args, jsObject);
        });
}

static void registerOnParentChanged(cc::Node *node, se::Object *jsObject) {
    node->on(
        cc::NodeEventType::PARENT_CHANGED,
        [jsObject](cc::Node *oldParent) {
            se::AutoHandleScope hs;
            se::Value           funcVal;
            jsObject->getProperty("_onParentChanged", &funcVal);
            SE_PRECONDITION2_VOID(funcVal.isObject() && funcVal.toObject()->isFunction(), "Not function named _onParentChanged.");

            se::ValueArray args;
            se::Value      arg0;
            nativevalue_to_se(oldParent, arg0);
            args.push_back(arg0);
            funcVal.toObject()->call(args, jsObject);
        });
}

static void registerOnLayerChanged(cc::Node *node, se::Object *jsObject) {
    node->on(
        cc::NodeEventType::PARENT_CHANGED,
        [jsObject](uint32_t layer) {
            se::AutoHandleScope hs;
            se::Value           funcVal;
            jsObject->getProperty("_onLayerChanged", &funcVal);
            SE_PRECONDITION2_VOID(funcVal.isObject() && funcVal.toObject()->isFunction(), "Not function named _onLayerChanged.");

            se::ValueArray args;
            se::Value      arg0;
            nativevalue_to_se(layer, arg0);
            args.push_back(arg0);
            funcVal.toObject()->call(args, jsObject);
        });
}

static void registerOnChildRemoved(cc::Node *node, se::Object *jsObject) {
    node->on(
        cc::NodeEventType::PARENT_CHANGED,
        [jsObject](cc::Node *child) {
            se::AutoHandleScope hs;
            se::Value           funcVal;
            jsObject->getProperty("_onChildRemoved", &funcVal);
            SE_PRECONDITION2_VOID(funcVal.isObject() && funcVal.toObject()->isFunction(), "Not function named _onChildRemoved.");

            se::ValueArray args;
            se::Value      arg0;
            nativevalue_to_se(child, arg0);
            args.push_back(arg0);
            funcVal.toObject()->call(args, jsObject);
        });
}

static void registerOnChildAdded(cc::Node *node, se::Object *jsObject) {
    cc::CallbackInfoBase::ID skip;
    node->on(
        cc::NodeEventType::PARENT_CHANGED,
        [jsObject](cc::Node *child) {
            se::AutoHandleScope hs;
            se::Value           funcVal;
            jsObject->getProperty("_onChildAdded", &funcVal);
            SE_PRECONDITION2_VOID(funcVal.isObject() && funcVal.toObject()->isFunction(), "Not function named _onChildAdded.");

            se::ValueArray args;
            se::Value      arg0;
            nativevalue_to_se(child, arg0);
            args.push_back(arg0);
            funcVal.toObject()->call(args, jsObject);
        },
        skip);
}

static void registerOnActiveNode(cc::Node *node, se::Object *jsObject) {
    cc::CallbackInfoBase::ID skip;
    node->on(
        cc::NodeEventType::PARENT_CHANGED,
        [jsObject](bool shouldActiveNow) {
            se::AutoHandleScope hs;
            se::Value           funcVal;
            jsObject->getProperty("_onActiveNode", &funcVal);
            SE_PRECONDITION2_VOID(funcVal.isObject() && funcVal.toObject()->isFunction(), "Not function named _onActiveNode.");

            se::ValueArray args;
            se::Value      arg0;
            nativevalue_to_se(shouldActiveNow, arg0);
            args.push_back(arg0);
            funcVal.toObject()->call(args, jsObject);
        },
        skip);
}

static bool js_scene_Node_registerListeners(se::State &s) // NOLINT(readability-identifier-naming)
{
    auto *cobj = SE_THIS_OBJECT<cc::Node>(s);
    SE_PRECONDITION2(cobj, false, "js_scene_Node_registerListeners : Invalid Native Object");

    auto *jsObject = s.thisObject();

#define NODE_DISPATCH_EVENT_TO_JS(eventType, jsFuncName)                                                 \
    cobj->on(                                                                                            \
        eventType, [jsObject]() {                                                                        \
            se::AutoHandleScope hs;                                                                      \
            se::Value           funcVal;                                                                 \
            bool                ok = jsObject->getProperty(#jsFuncName, &funcVal) && funcVal.isObject(); \
            SE_PRECONDITION2_VOID(ok, "js_scene_Node_registerListeners : Error processing arguments");   \
            funcVal.toObject()->call(se::EmptyValueArray, jsObject);                                     \
        });

    registerOnTransformChanged(cobj, jsObject);
    registerOnParentChanged(cobj, jsObject);
    registerOnLayerChanged(cobj, jsObject);
    registerOnChildRemoved(cobj, jsObject);
    registerOnChildAdded(cobj, jsObject);
    registerOnActiveNode(cobj, jsObject);

    NODE_DISPATCH_EVENT_TO_JS(cc::EventTypesToJS::NODE_REATTACH, _onReAttach);
    NODE_DISPATCH_EVENT_TO_JS(cc::EventTypesToJS::NODE_REMOVE_PERSIST_ROOT_NODE, _onRemovePersistRootNode);
    NODE_DISPATCH_EVENT_TO_JS(cc::EventTypesToJS::NODE_DESTROY_COMPONENTS, _onDestroyComponents);
    NODE_DISPATCH_EVENT_TO_JS(cc::EventTypesToJS::NODE_UI_TRANSFORM_DIRTY, _onUiTransformDirty);
    NODE_DISPATCH_EVENT_TO_JS(cc::NodeEventType::NODE_DESTROYED, _onNodeDestroyed);
    NODE_DISPATCH_EVENT_TO_JS(cc::NodeEventType::SIBLING_ORDER_CHANGED, _onSiblingOrderChanged);

    return true;
}
SE_BIND_FUNC(js_scene_Node_registerListeners) // NOLINT(readability-identifier-naming)

bool register_all_scene_manual(se::Object *obj) // NOLINT(readability-identifier-naming)
{
    // Get the ns
    se::Value nsVal;
    if (!obj->getProperty("ns", &nsVal)) {
        se::HandleObject jsobj(se::Object::createPlainObject());
        nsVal.setObject(jsobj);
        obj->setProperty("ns", nsVal);
    }

    __jsb_cc_scene_RenderScene_proto->defineFunction("updateBatches", _SE(js_scene_RenderScene_updateBatches));
    __jsb_cc_Root_proto->defineFunction("_registerListeners", _SE(js_root_registerListeners));

    // Node TS wrapper will invoke this function to let native object listen some events.
    __jsb_cc_Node_proto->defineFunction("_registerListeners", _SE(js_scene_Node_registerListeners));
    return true;
}
