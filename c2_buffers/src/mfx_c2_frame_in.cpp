/********************************************************************************

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2017-2021 Intel Corporation. All Rights Reserved.

*********************************************************************************/

#include "mfx_c2_frame_in.h"
#include "mfx_debug.h"
#include "mfx_c2_defs.h"
#include "mfx_c2_utils.h"
#include "mfx_c2_debug.h"

#include <C2AllocatorGralloc.h>

using namespace android;

#undef MFX_DEBUG_MODULE_NAME
#define MFX_DEBUG_MODULE_NAME "mfx_c2_frame_in"

MfxC2FrameIn::~MfxC2FrameIn()
{
    MFX_DEBUG_TRACE_FUNC;

    if (frame_converter_ && mfx_frame_ && mfx_frame_->Data.MemId) {
        frame_converter_->FreeGrallocToVaMapping(mfx_frame_->Data.MemId);
    }
}

c2_status_t MfxC2FrameIn::Create(std::shared_ptr<MfxFrameConverter> frame_converter,  std::unique_ptr<const C2GraphicView> c_graph_view,
        C2FrameData& buf_pack, mfxFrameSurface1 *mfx_frame, MfxC2FrameIn* wrapper)
{
    wrapper->c2_graphic_view_ = std::move(c_graph_view);
    wrapper->frame_converter_ = frame_converter;
    wrapper->mfx_frame_ = mfx_frame;
    wrapper->c2_buffer_ = std::move(buf_pack.buffers.front());

    return C2_OK;
}

c2_status_t MfxC2FrameIn::Create(std::shared_ptr<MfxFrameConverter> frame_converter,
    C2FrameData& buf_pack, const mfxFrameInfo& info, c2_nsecs_t timeout, MfxC2FrameIn* wrapper)
{
    MFX_DEBUG_TRACE_FUNC;

    c2_status_t res = C2_OK;

    do {
        if (nullptr == wrapper) {
            res = C2_BAD_VALUE;
            break;
        }

        std::unique_ptr<C2ConstGraphicBlock> c_graph_block;
        res = GetC2ConstGraphicBlock(buf_pack, &c_graph_block);
        if(C2_OK != res) break;

        if ( (info.Width && info.Width > c_graph_block->width()) ||
             (info.Height && info.Height > c_graph_block->height()) ) {
            res = C2_BAD_VALUE;
            break;
        }

        mfxFrameSurface1 *mfx_frame = new mfxFrameSurface1;

        if (nullptr != frame_converter) {

            mfxMemId mem_id = nullptr;
            bool decode_target = false;
            native_handle_t *grallocHandle = android::UnwrapNativeCodec2GrallocHandle(c_graph_block->handle());

            mfxStatus mfx_sts = frame_converter->ConvertGrallocToVa(grallocHandle, decode_target, &mem_id);
            if (MFX_ERR_NONE != mfx_sts) {
                res = MfxStatusToC2(mfx_sts);
                break;
            }

            InitMfxFrameHW(buf_pack.ordinal.timestamp.peeku(), buf_pack.ordinal.frameIndex.peeku(),
                mem_id, c_graph_block->width(), c_graph_block->height(), MFX_FOURCC_NV12, info,
                mfx_frame);
        } else {
            res = MapConstGraphicBlock(*c_graph_block, timeout, &wrapper->c2_graphic_view_);
            if(C2_OK != res) break;

            const uint32_t stride = wrapper->c2_graphic_view_->layout().planes[C2PlanarLayout::PLANE_Y].rowInc;
            InitMfxNV12FrameSW(buf_pack.ordinal.timestamp.peeku(), buf_pack.ordinal.frameIndex.peeku(),
                wrapper->c2_graphic_view_->data(), c_graph_block->width(), c_graph_block->height(), stride, MFX_FOURCC_NV12, info,
                mfx_frame);
        }

        wrapper->frame_converter_ = frame_converter;
        wrapper->mfx_frame_ = mfx_frame;
        wrapper->c2_buffer_ = std::move(buf_pack.buffers.front());

    } while(false);

    MFX_DEBUG_TRACE__android_c2_status_t(res);
    return res;
}
