/********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2017-2019 Intel Corporation. All Rights Reserved.

*********************************************************************************/

#pragma once

#include "mfx_defs.h"
#include "mfx_c2_defs.h"
#include "mfx_c2_component.h"
#include "mfx_c2_param_reflector.h"

#include <C2Component.h>

#include <map>
#include <functional>

// function declaration to make possible use this function statically too
extern "C" CreateMfxC2ComponentFunc MfxCreateC2Component;

class MfxC2ComponentsRegistry
{
private:
    MfxC2ComponentsRegistry();

    MFX_CLASS_NO_COPY(MfxC2ComponentsRegistry);

public:
    static MfxC2ComponentsRegistry& getInstance();

    c2_status_t CreateMfxC2Component(const char* name, const MfxC2Component::CreateConfig& config,
        std::shared_ptr<MfxC2ParamReflector> reflector, MfxC2Component** component);

    void RegisterMfxC2Component(const std::string& name, CreateMfxC2ComponentFunc* createFunc);

private:
    std::map<std::string, CreateMfxC2ComponentFunc*> registry_;
};
