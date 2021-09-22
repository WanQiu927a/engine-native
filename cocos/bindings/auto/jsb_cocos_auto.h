#pragma once
#include <type_traits>
#include "base/Config.h"
#include "cocos/bindings/jswrapper/SeApi.h"
#include "cocos/bindings/manual/jsb_conversions.h"
#include "cocos/platform/CanvasRenderingContext2D.h"
#include "cocos/platform/Device.h"
#include "cocos/platform/FileUtils.h"
#include "cocos/platform/SAXParser.h"

extern se::Object *__jsb_cc_FileUtils_proto; // NOLINT
extern se::Class * __jsb_cc_FileUtils_class; // NOLINT

bool js_register_cc_FileUtils(se::Object *obj); // NOLINT
bool register_all_engine(se::Object *obj);      // NOLINT

JSB_REGISTER_OBJECT_TYPE(cc::FileUtils);
SE_DECLARE_FUNC(js_engine_FileUtils_addSearchPath);
SE_DECLARE_FUNC(js_engine_FileUtils_createDirectory);
SE_DECLARE_FUNC(js_engine_FileUtils_fullPathForFilename);
SE_DECLARE_FUNC(js_engine_FileUtils_fullPathFromRelativeFile);
SE_DECLARE_FUNC(js_engine_FileUtils_getDataFromFile);
SE_DECLARE_FUNC(js_engine_FileUtils_getDefaultResourceRootPath);
SE_DECLARE_FUNC(js_engine_FileUtils_getFileExtension);
SE_DECLARE_FUNC(js_engine_FileUtils_getFileSize);
SE_DECLARE_FUNC(js_engine_FileUtils_getOriginalSearchPaths);
SE_DECLARE_FUNC(js_engine_FileUtils_getSearchPaths);
SE_DECLARE_FUNC(js_engine_FileUtils_getStringFromFile);
SE_DECLARE_FUNC(js_engine_FileUtils_getSuitableFOpen);
SE_DECLARE_FUNC(js_engine_FileUtils_getValueMapFromData);
SE_DECLARE_FUNC(js_engine_FileUtils_getValueMapFromFile);
SE_DECLARE_FUNC(js_engine_FileUtils_getValueVectorFromFile);
SE_DECLARE_FUNC(js_engine_FileUtils_getWritablePath);
SE_DECLARE_FUNC(js_engine_FileUtils_isAbsolutePath);
SE_DECLARE_FUNC(js_engine_FileUtils_isDirectoryExist);
SE_DECLARE_FUNC(js_engine_FileUtils_isFileExist);
SE_DECLARE_FUNC(js_engine_FileUtils_listFiles);
SE_DECLARE_FUNC(js_engine_FileUtils_purgeCachedEntries);
SE_DECLARE_FUNC(js_engine_FileUtils_removeDirectory);
SE_DECLARE_FUNC(js_engine_FileUtils_removeFile);
SE_DECLARE_FUNC(js_engine_FileUtils_renameFile);
SE_DECLARE_FUNC(js_engine_FileUtils_setDefaultResourceRootPath);
SE_DECLARE_FUNC(js_engine_FileUtils_setSearchPaths);
SE_DECLARE_FUNC(js_engine_FileUtils_setWritablePath);
SE_DECLARE_FUNC(js_engine_FileUtils_writeDataToFile);
SE_DECLARE_FUNC(js_engine_FileUtils_writeStringToFile);
SE_DECLARE_FUNC(js_engine_FileUtils_writeToFile);
SE_DECLARE_FUNC(js_engine_FileUtils_writeValueMapToFile);
SE_DECLARE_FUNC(js_engine_FileUtils_writeValueVectorToFile);
SE_DECLARE_FUNC(js_engine_FileUtils_normalizePath);
SE_DECLARE_FUNC(js_engine_FileUtils_setDelegate);
SE_DECLARE_FUNC(js_engine_FileUtils_getInstance);
SE_DECLARE_FUNC(js_engine_FileUtils_getFileDir);

extern se::Object *__jsb_cc_CanvasGradient_proto; // NOLINT
extern se::Class * __jsb_cc_CanvasGradient_class; // NOLINT

bool js_register_cc_CanvasGradient(se::Object *obj); // NOLINT
bool register_all_engine(se::Object *obj);           // NOLINT

JSB_REGISTER_OBJECT_TYPE(cc::CanvasGradient);
SE_DECLARE_FUNC(js_engine_CanvasGradient_addColorStop);
SE_DECLARE_FUNC(js_engine_CanvasGradient_CanvasGradient);

extern se::Object *__jsb_cc_CanvasRenderingContext2D_proto; // NOLINT
extern se::Class * __jsb_cc_CanvasRenderingContext2D_class; // NOLINT

bool js_register_cc_CanvasRenderingContext2D(se::Object *obj); // NOLINT
bool register_all_engine(se::Object *obj);                     // NOLINT

JSB_REGISTER_OBJECT_TYPE(cc::CanvasRenderingContext2D);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_beginPath);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_clearRect);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_closePath);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_createLinearGradient);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_fill);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_fillImageData);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_lineTo);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_moveTo);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_rect);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_restore);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_rotate);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_save);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_scale);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_setTransform);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_stroke);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_transform);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_translate);
SE_DECLARE_FUNC(js_engine_CanvasRenderingContext2D_CanvasRenderingContext2D);

extern se::Object *__jsb_cc_Device_proto; // NOLINT
extern se::Class * __jsb_cc_Device_class; // NOLINT

bool js_register_cc_Device(se::Object *obj); // NOLINT
bool register_all_engine(se::Object *obj);   // NOLINT

JSB_REGISTER_OBJECT_TYPE(cc::Device);
SE_DECLARE_FUNC(js_engine_Device_getDevicePixelRatio);
SE_DECLARE_FUNC(js_engine_Device_setAccelerometerEnabled);
SE_DECLARE_FUNC(js_engine_Device_setAccelerometerInterval);
SE_DECLARE_FUNC(js_engine_Device_vibrate);
SE_DECLARE_FUNC(js_engine_Device_setKeepScreenOn);
SE_DECLARE_FUNC(js_engine_Device_getNetworkType);
SE_DECLARE_FUNC(js_engine_Device_getBatteryLevel);
SE_DECLARE_FUNC(js_engine_Device_getDeviceOrientation);
SE_DECLARE_FUNC(js_engine_Device_getDPI);
SE_DECLARE_FUNC(js_engine_Device_getSafeAreaEdge);
SE_DECLARE_FUNC(js_engine_Device_getDeviceModel);

extern se::Object *__jsb_cc_SAXParser_proto; // NOLINT
extern se::Class * __jsb_cc_SAXParser_class; // NOLINT

bool js_register_cc_SAXParser(se::Object *obj); // NOLINT
bool register_all_engine(se::Object *obj);      // NOLINT

JSB_REGISTER_OBJECT_TYPE(cc::SAXParser);
SE_DECLARE_FUNC(js_engine_SAXParser_init);
