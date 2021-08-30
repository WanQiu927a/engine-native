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

#include "renderer/core/ProgramLib.h"
#include <algorithm>
#include <cmath>
#include <functional>
#include <numeric>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "base/Log.h"
#include "core/Types.h"
#include "core/assets/EffectAsset.h"
#include "renderer/core/PassUtils.h"
#include "renderer/gfx-base/GFXDef-common.h"
#include "renderer/gfx-base/GFXPipelineLayout.h"
#include "renderer/pipeline/Define.h"

namespace cc {

template <typename T>
auto getBitCount(T cnt) {
    return std::ceil(std::log2(cnt > 2 ? 2 : cnt)); // std::max checks number types
}

static bool recordAsBool(const MacroRecord::mapped_type &v) {
    if (std::holds_alternative<bool>(v)) {
        return std::get<bool>(v);
    }
    if (std::holds_alternative<std::string>(v)) {
        return std::get<std::string>(v) == "true";
    }
    if (std::holds_alternative<float>(v)) {
        return std::get<float>(v) != 0;
    }
    return false;
}

static std::string recordAsString(const MacroRecord::mapped_type &v) {
    if (std::holds_alternative<bool>(v)) {
        return std::get<bool>(v) ? "1" : "0";
    }
    if (std::holds_alternative<std::string>(v)) {
        return std::get<std::string>(v);
    }
    if (std::holds_alternative<float>(v)) {
        return std::to_string(std::get<float>(v));
    }
    return "";
}

std::string mapDefine(const IDefineInfo &info, const std::optional<MacroRecord::mapped_type> &def) {
    if (info.type == "boolean") {
        return def.has_value() ? (recordAsBool(def.value()) ? "1" : "0") : "0";
    }
    if (info.type == "string") {
        return def.has_value() ? recordAsString(def.value()) : info.options.value()[0];
    }
    if (info.type == "number") {
        return def.has_value() ? std::to_string(std::get<float>(def.value())) : std::to_string(info.range.value()[0]);
    }
    CC_LOG_WARNING("unknown define type '%s'", info.type.c_str());
    return "-1"; // should neven happen
}

std::vector<IMacroInfo> prepareDefines(const MacroRecord &records, const std::vector<IDefineRecord> &defList) {
    std::vector<IMacroInfo> macros{};
    for (const auto &tmp : defList) {
        const auto &name  = tmp.name;
        auto        it    = records.find(name);
        auto        value = mapDefine(tmp, it == records.end() ? std::nullopt : std::optional(it->second));
        //TODO(PatriceJiang): v === '0' can be bool ?
        auto isDefault = it == records.end() || (std::holds_alternative<float>(it->second) && std::get<float>(it->second) == 0.0F);
        macros.emplace_back(IMacroInfo{.name = name, .value = value, .isDefault = isDefault});
    }
    return macros;
}

std::string getShaderInstanceName(const std::string &name, const std::vector<IMacroInfo> &macros) {
    std::stringstream ret;
    ret << name;
    for (const auto &cur : macros) {
        if (!cur.isDefault) {
            ret << "|" << cur.name << cur.value;
        }
    }
    return ret.str();
}

void insertBuiltinBindings(const IProgramInfo &tmpl, ITemplateInfo &tmplInfo, const pipeline::DescriptorSetLayoutInfos &source,
                           const std::string &type, std::vector<gfx::DescriptorSetLayoutBinding> *outBindings) {
    auto &                         target = type == "globals" ? tmpl.builtins.globals : tmpl.builtins.locals;
    std::vector<gfx::UniformBlock> tempBlocks{};
    // TODO(PatriceJiang): dd
    for (const auto &b : target.blocks) {
        auto infoIt = source.layouts.find(b.name);
        if (infoIt == source.layouts.end()) {
            CC_LOG_WARNING("builtin UBO '%s' not available !", b.name.c_str());
            continue;
        }
        const auto &info = infoIt->second;
        //TODO(PatriceJiang): UniformBlock|UniformSamplerTexture|UniformStorageImage should share the same super class
        auto binding = std::find_if(source.bindings.begin(), source.bindings.end(), [&](const gfx::DescriptorSetLayoutBinding &i) {
            if (std::holds_alternative<gfx::UniformBlock>(info)) {
                return i.binding == std::get<gfx::UniformBlock>(info).binding;
            }
            if (std::holds_alternative<gfx::UniformSamplerTexture>(info)) {
                return i.binding == std::get<gfx::UniformSamplerTexture>(info).binding;
            }
            if (std::holds_alternative<gfx::UniformStorageImage>(info)) {
                return i.binding == std::get<gfx::UniformStorageImage>(info).binding;
            }
            return false;
        });
        if (binding == source.bindings.end() || !(binding->descriptorType & gfx::DESCRIPTOR_BUFFER_TYPE)) {
            CC_LOG_WARNING("builtin UBO '%s' not available !", b.name.c_str());
            continue;
        }
        //TODO(PatriceJIang): insert uniform block
        auto *block = std::get_if<gfx::UniformBlock>(&info);
        if (block) {
            tempBlocks.emplace_back(*block);
        }
        //TODO(PatriceJiang): use pointer instead of structs in comparasion
        // `outBindings.includes(binding)`
        if (outBindings && std::count_if(outBindings->begin(), outBindings->end(), [=](const gfx::DescriptorSetLayoutBinding &b) { return b.binding == binding->binding; }) == 0) {
            outBindings->emplace_back(*binding);
        }
    }
    tmplInfo.gfxBlocks.insert(tmplInfo.gfxBlocks.begin(), tempBlocks.begin(), tempBlocks.end());
    std::vector<gfx::UniformSamplerTexture> tempSamplerTextures;
    for (const auto &s : target.samplerTextures) {
        auto infoIt = source.layouts.find(s.name);
        if (infoIt == source.layouts.end()) {
            CC_LOG_WARNING("builtin samplerTexture '%s' not available !", s.name.c_str());
            continue;
        }
        auto *info = std::get_if<gfx::UniformSamplerTexture>(&(infoIt->second));
        if (!info) {
            CC_LOG_WARNING("builtin samplerTexture '%s' not available !", s.name.c_str());
            continue;
        }
        const auto binding = std::find_if(source.bindings.begin(), source.bindings.end(), [&](const auto &bd) {
            //TODO(PatriceJiang) compare object
            return bd.binding == info->binding;
        });
        if (binding == source.bindings.end() || !(binding->descriptorType & gfx::DESCRIPTOR_SAMPLER_TYPE)) {
            CC_LOG_WARNING("builtin samplerTexture '%s' not available !", s.name.c_str());
            continue;
        }
        tempSamplerTextures.emplace_back(*info);
        if (outBindings && std::count_if(outBindings->begin(), outBindings->end(), [&](const auto &b) {
                //TODO(PatriceJiang) compare object
                return b.binding == binding->binding;
            })) {
            outBindings->emplace_back(*binding);
        }
    }

    tmplInfo.gfxSamplerTextures.insert(tmplInfo.gfxSamplerTextures.begin(), tempSamplerTextures.begin(), tempSamplerTextures.end());
    if (outBindings) {
        std::sort(outBindings->begin(), outBindings->end(), [](const auto &a, const auto &b) {
            return a.binding < b.binding;
        });
    }
}

int getSize(const IBlockInfo &block) {
    auto s = 0;
    for (const auto &m : block.members) {
        s += getTypeSize(m.type) * m.count;
    }
    return s;
}

auto genHandles(const IProgramInfo &tmpl) {
    Record<std::string, uint32_t> handleMap{};
    // block member handles
    for (const auto &block : tmpl.blocks) {
        const auto members = block.members;
        auto       offset  = 0;
        for (const auto &uniform : members) {
            handleMap[uniform.name] = genHandle(PropertyType::BUFFER,
                                                static_cast<uint32_t>(pipeline::SetIndex::MATERIAL),
                                                block.binding,
                                                uniform.type,
                                                offset);
            offset += (getTypeSize(uniform.type) >> 2) * uniform.count;
        }
    }
    // samplerTexture handles
    for (const auto &samplerTexture : tmpl.samplerTextures) {
        handleMap[samplerTexture.name] = genHandle(PropertyType::TEXTURE,
                                                   static_cast<uint32_t>(pipeline::SetIndex::MATERIAL),
                                                   samplerTexture.binding,
                                                   samplerTexture.type);
    }
    return handleMap;
}

bool dependencyCheck(const std::vector<std::string> &dependencies, const MacroRecord &defines) {
    for (const auto &d : dependencies) {
        if (d[0] == '!') { // negative dependency
            if (defines.find(d.substr(1)) != defines.end()) {
                return false;
            }
        } else if (defines.count(d) == 0 ? true : recordAsBool(defines.at(d))) {
            // TODO(PatriceJiang): !defines[d] : checked: undefine, false, 0
            return false;
        }
    }
    return true;
}

std::vector<gfx::Attribute> getActiveAttributes(const IProgramInfo &tmpl, const ITemplateInfo &tmplInfo, const MacroRecord &defines) {
    std::vector<gfx::Attribute> out{};
    auto                        attributes    = tmpl.attributes;
    auto                        gfxAttributes = tmplInfo.gfxAttributes;
    for (auto i = 0; i < attributes.size(); i++) {
        if (!dependencyCheck(attributes[i].defines, defines)) {
            continue;
        }
        out.emplace_back(gfxAttributes[i]);
    }
    return out;
}

const char *getDeviceShaderVersion(const gfx::Device &device) {
    switch (device.getGfxAPI()) {
        case gfx::API::GLES2:
        case gfx::API::WEBGL:
            return "glsl1";
        case gfx::API::GLES3:
        case gfx::API::WEBGL2:
            return "glsl3";
        default:
            return "glsl4";
    }
}

void ProgramLib::regist(EffectAsset &effect) {
    for (auto i = 0; i < effect._shaders.size(); i++) {
        auto tmpl        = define(effect._shaders[i]);
        tmpl->effectName = effect.getName();
    }
}

IProgramInfo *ProgramLib::define(IShaderInfo &shader) {
    auto itCurrTmpl = _templates.find(shader.name);
    if (itCurrTmpl != _templates.end() && itCurrTmpl->second.hash == shader.hash) {
        return &itCurrTmpl->second;
    }
    auto &currTmpl = itCurrTmpl->second;

    //TODO(PatriceJiang): need copy
    //```
    //const tmpl = ({ ...shader }) as IProgramInfo;
    ///```
    IProgramInfo &tmpl = _templates[shader.name];

    tmpl.effectName = shader.name;
    // tmpl.defines = shader.defines; //TODO: can not do simple copy
    tmpl.defines.resize(shader.defines.size());
    for (auto i = 0; i < shader.defines.size(); i++) {
        *reinterpret_cast<IDefineInfo *>(&tmpl.defines[i]) = shader.defines[i];
    }
    // tmpl.constantMacros = ?? //TODO(PatriceJiang)
    // tmpl.uber = ??   //TODO(PatriceJiang)

    // calculate option mask offset
    auto offset = 0;
    for (auto &def : tmpl.defines) {
        auto cnt = 1;
        if (def.type == "number") {
            //TODO(PatriceJiang): check typeof value
            auto &range = def.range.value();
            cnt         = getBitCount(range[1] - range[0] + 1); // inclusive on both ends
            def.map     = [=](std::any value) { return static_cast<int>(std::any_cast<float>(value) - range[0]); };
        } else if (def.type == "string") {
            cnt     = getBitCount(def.options.value().size());
            def.map = [=](std::any value) {
                //TODO(PatriceJiang): check typeof value
                int idx = std::find(def.options.value().begin(), def.options.value().end(), std::any_cast<std::string>(value)) - def.options.value().begin();
                return static_cast<int>(std::max(0, idx));
            };
        } else if (def.type == "boolean") {
            def.map = [=](std::any value) {
                bool *pvalue = std::any_cast<bool>(&value);
                if (pvalue) return *pvalue ? 1 : 0;
                return 0;
            };
        }
        def.offset = offset;
        offset += cnt;
    }
    if (offset > 31) {
        tmpl.uber = true;
    }
    // generate constant macros
    {
        std::stringstream ss;
        for (auto &key : tmpl.builtins.statistics) {
            ss << "#define " << key.first << " " << key.second << std::endl;
        }
        tmpl.constantMacros = ss.str();
    }
    if (_templateInfos.count(tmpl.hash > 0)) {
        ITemplateInfo tmplInfo{};
        // cache material-specific descriptor set layout
        tmplInfo.samplerStartBinding = tmpl.blocks.size();
        tmplInfo.gfxBlocks           = {};
        tmplInfo.gfxSamplerTextures  = {};
        tmplInfo.bindings            = {};
        tmplInfo.blockSizes          = {};
        for (auto &block : tmpl.blocks) {
            tmplInfo.blockSizes.emplace_back(static_cast<float>(getSize(block)));
            tmplInfo.bindings.emplace_back(gfx::DescriptorSetLayoutBinding{
                .binding        = static_cast<uint>(block.binding),
                .descriptorType = block.descriptorType.value_or(gfx::DescriptorType::UNIFORM_BUFFER),
                .count          = 1,
                .stageFlags     = block.stageFlags});
            std::vector<gfx::Uniform> uniforms;
            {
                // construct uniforms
                uniforms.resize(block.members.size());
                for (int i = 0; i < block.members.size(); i++) {
                    uniforms[i] = gfx::Uniform{
                        .name  = block.members[i].name,
                        .type  = block.members[i].type,
                        .count = block.members[i].count,
                    };
                }
            }
            tmplInfo.gfxBlocks.emplace_back(gfx::UniformBlock{
                .set     = static_cast<uint>(pipeline::SetIndex::MATERIAL),
                .binding = static_cast<uint>(block.binding),
                .name    = block.name,
                .members = uniforms,
                .count   = 1}); // effect compiler guarantees block count = 1
        }
        for (auto &samplerTexture : tmpl.samplerTextures) {
            tmplInfo.bindings.emplace_back(gfx::DescriptorSetLayoutBinding{
                .binding        = static_cast<uint>(samplerTexture.binding),
                .descriptorType = samplerTexture.descriptorType.value_or(gfx::DescriptorType::SAMPLER_TEXTURE),
                .count          = samplerTexture.count,
                .stageFlags     = samplerTexture.stageFlags});
            tmplInfo.gfxSamplerTextures.emplace_back(gfx::UniformSamplerTexture{
                .set     = static_cast<uint>(pipeline::SetIndex::MATERIAL),
                .binding = static_cast<uint>(samplerTexture.binding),
                .name    = samplerTexture.name,
                .type    = samplerTexture.type,
                .count   = samplerTexture.count});
        }
        tmplInfo.gfxAttributes = {};
        for (auto &attr : tmpl.attributes) {
            tmplInfo.gfxAttributes.emplace_back(gfx::Attribute{
                .name         = attr.name,
                .format       = attr.format,
                .isNormalized = attr.isNormalized,
                .stream       = 0,
                .isInstanced  = attr.isInstanced,
                .location     = attr.location});
        }
        insertBuiltinBindings(tmpl, tmplInfo, pipeline::localDescriptorSetLayout, "locals", nullptr);

        tmplInfo.gfxStages = {};
        tmplInfo.gfxStages.emplace_back(gfx::ShaderStage{
            .stage  = gfx::ShaderStageFlagBit::VERTEX,
            .source = ""});
        tmplInfo.gfxStages.emplace_back(gfx::ShaderStage{
            .stage  = gfx::ShaderStageFlagBit::FRAGMENT,
            .source = ""});
        tmplInfo.handleMap  = genHandles(tmpl);
        tmplInfo.setLayouts = {};

        _templateInfos[tmpl.hash] = tmplInfo;
    }
    return &tmpl;
}

/**
     * @en Gets the shader template with its name
     * @zh 通过名字获取 Shader 模板
     * @param name Target shader name
     */

IProgramInfo *ProgramLib::getTemplate(const std::string &name) {
    auto it = _templates.find(name);
    return it != _templates.end() ? &it->second : nullptr;
}

/**
     * @en Gets the shader template info with its name
     * @zh 通过名字获取 Shader 模版信息
     * @param name Target shader name
     */

ITemplateInfo *ProgramLib::getTemplateInfo(const std::string &name) {
    auto it = _templates.find(name);
    assert(it != _templates.end());
    auto hash   = it->second.hash;
    auto itInfo = _templateInfos.find(hash);
    return itInfo != _templateInfos.end() ? &itInfo->second : nullptr;
}

/**
     * @en Gets the pipeline layout of the shader template given its name
     * @zh 通过名字获取 Shader 模板相关联的管线布局
     * @param name Target shader name
     */
gfx::DescriptorSetLayout *ProgramLib::getDescriptorSetLayout(gfx::Device &device, const std::string &name, bool isLocal) {
    auto itTmpl = _templates.find(name);
    assert(itTmpl != _templates.end());
    const auto &tmpl      = itTmpl->second;
    auto        itTplInfo = _templateInfos.find(tmpl.hash);
    auto &      tmplInfo  = itTplInfo->second;
    if (tmplInfo.setLayouts.empty()) {
        gfx::DescriptorSetLayoutInfo info;
        tmplInfo.setLayouts.resize(static_cast<size_t>(pipeline::SetIndex::COUNT));
        info.bindings                                                           = tmplInfo.bindings;
        tmplInfo.setLayouts[static_cast<index_t>(pipeline::SetIndex::MATERIAL)] = device.createDescriptorSetLayout(info);
        info.bindings                                                           = pipeline::localDescriptorSetLayout.bindings;
        tmplInfo.setLayouts[static_cast<index_t>(pipeline::SetIndex::LOCAL)]    = device.createDescriptorSetLayout(info);
    }
    return tmplInfo.setLayouts[isLocal ? static_cast<index_t>(pipeline::SetIndex::LOCAL) : static_cast<index_t>(pipeline::SetIndex::MATERIAL)];
}

std::string ProgramLib::getKey(const std::string &name, const MacroRecord &defines) {
    auto itTpl = _templates.find(name);
    assert(itTpl != _templates.end());
    auto &tmpl     = itTpl->second;
    auto &tmplDefs = tmpl.defines;
    if (tmpl.uber) {
        std::stringstream key;
        for (auto &tmplDef : tmplDefs) {
            auto itDef = defines.find(tmplDef.name);
            if (itDef == defines.end() || !tmplDef.map) {
                continue;
            }
            auto &value  = itDef->second;
            auto  mapped = tmplDef.map(value);
            auto  offset = tmplDef.offset;
            key << offset << mapped << "|";
        }
        return key.str() + std::to_string(tmpl.hash);
    }
    auto              key = 0;
    std::stringstream ss;
    for (auto &tmplDef : tmplDefs) {
        auto itDef = defines.find(tmplDef.name);
        if (itDef == defines.end() || !tmplDef.map) {
            continue;
        }
        auto &value  = itDef->second;
        auto  mapped = tmplDef.map(value);
        auto  offset = tmplDef.offset;
        key |= (mapped << offset);
    }
    ss << std::hex << key;
    return ss.str() + std::to_string(tmpl.hash);
}

void ProgramLib::destroyShaderByDefines(const MacroRecord &defines) {
    if (defines.empty()) return;
    std::vector<std::string> defineValues;
    for (const auto &i : defines) {
        defineValues.emplace_back(i.first + recordAsString(i.second));
    }
    std::vector<std::string> matchedKeys;
    for (const auto &i : _cache) {
        bool matched = true;
        for (const auto &v : defineValues) {
            if (i.first.find(v) == std::string::npos) {
                matched = false;
                break;
            }
        }
        if (matched) {
            matchedKeys.emplace_back(i.first);
        }
    }
    for (const auto &key : matchedKeys) {
        CC_LOG_DEBUG("destroyed shader %s", key.c_str());
        _cache[key]->destroy(); // TODO(PatriceJiang): unref ?
        _cache.erase(key);
    }
}

gfx::Shader *ProgramLib::getGFXShader(gfx::Device &device, const std::string &name, MacroRecord &defines,
                                      const pipeline::RenderPipeline &pipeline, std::string *keyOut) {
    for (const auto &it : pipeline.getMacros()) {
        defines[it.first] = it.second;
    }

    std::string key;
    if (!keyOut) {
        key = getKey(name, defines);
    } else {
        key = *keyOut;
    }
    auto itRes = _cache.find(key);
    if (itRes != _cache.end()) {
        return itRes->second;
    }

    auto itTpl = _templates.find(name);
    assert(itTpl != _templates.end());

    const auto &tmpl      = itTpl->second;
    const auto  itTplInfo = _templateInfos.find(tmpl.hash);
    assert(itTplInfo != _templateInfos.end());
    auto tmplInfo = itTplInfo->second;

    if (!tmplInfo.pipelineLayout) {
        getDescriptorSetLayout(device, name); // ensure set layouts have been created
        insertBuiltinBindings(tmpl, tmplInfo, pipeline::globalDescriptorSetLayout, "globals", nullptr);
        tmplInfo.setLayouts[static_cast<index_t>(pipeline::SetIndex::GLOBAL)] = pipeline.getDescriptorSetLayout();
        tmplInfo.pipelineLayout                                               = device.createPipelineLayout(gfx::PipelineLayoutInfo{tmplInfo.setLayouts});
    }

    std::vector<IMacroInfo> macroArray = prepareDefines(defines, tmpl.defines);
    std::stringstream       ss;
    ss << std::endl;
    for (const auto &m : macroArray) {
        ss << "#define " << m.name << " " << m.value << std::endl;
    }
    auto prefix = pipeline.getConstantMacros() + tmpl.constantMacros + ss.str();

    const IShaderSource *src                 = &tmpl.glsl3;
    const auto *         deviceShaderVersion = getDeviceShaderVersion(device);
    if (deviceShaderVersion) {
        src = tmpl.getSource(deviceShaderVersion);
    } else {
        CC_LOG_ERROR("Invalid GFX API!");
    }
    tmplInfo.gfxStages[0].source = prefix + src->vert;
    tmplInfo.gfxStages[1].source = prefix + src->frag;

    // strip out the active attributes only, instancing depend on this
    auto attributes = getActiveAttributes(tmpl, tmplInfo, defines);

    auto instanceName          = getShaderInstanceName(name, macroArray);
    auto shaderInfo            = gfx::ShaderInfo{instanceName, tmplInfo.gfxStages, attributes, tmplInfo.gfxBlocks};
    shaderInfo.samplerTextures = tmplInfo.gfxSamplerTextures;
    auto *shader               = device.createShader(shaderInfo);
    _cache[key]                = shader;
    return shader;
}

} // namespace cc