/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "GraphicBufferMapper"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS
//#define LOG_NDEBUG 0

#include <stdint.h>
#include <errno.h>

// We would eliminate the non-conforming zero-length array, but we can't since
// this is effectively included from the Linux kernel
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wzero-length-array"
#include <sync/sync.h>
#pragma clang diagnostic pop

#include <utils/Errors.h>
#include <utils/Log.h>
#include <utils/Trace.h>

#include <ui/Gralloc1On0Adapter.h>
#include <ui/GrallocMapper.h>
#include <ui/GraphicBufferMapper.h>
#include <ui/GraphicBuffer.h>

#include <system/graphics.h>

namespace android {
// ---------------------------------------------------------------------------

ANDROID_SINGLETON_STATIC_INSTANCE( GraphicBufferMapper )

GraphicBufferMapper::GraphicBufferMapper()
  : mMapper(std::make_unique<const Gralloc2::Mapper>())
{
    if (!mMapper->valid()) {
        mLoader = std::make_unique<Gralloc1::Loader>();
        mDevice = mLoader->getDevice();
    }
}



status_t GraphicBufferMapper::registerBuffer(buffer_handle_t handle)
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        error = static_cast<gralloc1_error_t>(mMapper->retain(handle));
    } else {
        // This always returns GRALLOC1_BAD_HANDLE when handle is from a
        // remote process and mDevice is backed by Gralloc1On0Adapter.
        error = mDevice->retain(handle);
        if (error == GRALLOC1_ERROR_BAD_HANDLE &&
                mDevice->hasCapability(GRALLOC1_CAPABILITY_ON_ADAPTER)) {
            ALOGE("registerBuffer by handle is not supported with "
                  "Gralloc1On0Adapter");
        }
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE, "registerBuffer(%p) failed: %d",
            handle, error);

    return error;
}

status_t GraphicBufferMapper::registerBuffer(const GraphicBuffer* buffer)
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        error = static_cast<gralloc1_error_t>(
                mMapper->retain(buffer->getNativeBuffer()->handle));
    } else {
        error = mDevice->retain(buffer);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE, "registerBuffer(%p) failed: %d",
            buffer->getNativeBuffer()->handle, error);

    return error;
}

status_t GraphicBufferMapper::unregisterBuffer(buffer_handle_t handle)
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        mMapper->release(handle);
        error = GRALLOC1_ERROR_NONE;
    } else {
        error = mDevice->release(handle);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE, "unregisterBuffer(%p): failed %d",
            handle, error);

    return error;
}

static inline gralloc1_rect_t asGralloc1Rect(const Rect& rect) {
    gralloc1_rect_t outRect{};
    outRect.left = rect.left;
    outRect.top = rect.top;
    outRect.width = rect.width();
    outRect.height = rect.height();
    return outRect;
}


status_t GraphicBufferMapper::getDimensions(buffer_handle_t handle,
        uint32_t* outWidth, uint32_t* outHeight) const
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        mMapper->getDimensions(handle, outWidth, outHeight);
        error = GRALLOC1_ERROR_NONE;
    } else {
        error = mDevice->getDimensions(handle, outWidth, outHeight);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE, "getDimensions(%p, ...): failed %d",
            handle, error);

    return error;
}

status_t GraphicBufferMapper::getFormat(buffer_handle_t handle,
        int32_t* outFormat) const
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        mMapper->getFormat(handle, outFormat);
        error = GRALLOC1_ERROR_NONE;
    } else {
        error = mDevice->getFormat(handle, outFormat);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE, "getFormat(%p, ...): failed %d",
            handle, error);

    return error;
}

status_t GraphicBufferMapper::getLayerCount(buffer_handle_t handle,
        uint32_t* outLayerCount) const
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        mMapper->getLayerCount(handle, outLayerCount);
        error = GRALLOC1_ERROR_NONE;
    } else {
        error = mDevice->getLayerCount(handle, outLayerCount);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE, "getLayerCount(%p, ...): failed %d",
            handle, error);

    return error;
}

status_t GraphicBufferMapper::getProducerUsage(buffer_handle_t handle,
        uint64_t* outProducerUsage) const
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        mMapper->getProducerUsage(handle, outProducerUsage);
        error = GRALLOC1_ERROR_NONE;
    } else {
        error = mDevice->getProducerUsage(handle, outProducerUsage);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE,
            "getProducerUsage(%p, ...): failed %d", handle, error);

    return error;
}

status_t GraphicBufferMapper::getConsumerUsage(buffer_handle_t handle,
        uint64_t* outConsumerUsage) const
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        mMapper->getConsumerUsage(handle, outConsumerUsage);
        error = GRALLOC1_ERROR_NONE;
    } else {
        error = mDevice->getConsumerUsage(handle, outConsumerUsage);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE,
            "getConsumerUsage(%p, ...): failed %d", handle, error);

    return error;
}

status_t GraphicBufferMapper::getBackingStore(buffer_handle_t handle,
        uint64_t* outBackingStore) const
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        mMapper->getBackingStore(handle, outBackingStore);
        error = GRALLOC1_ERROR_NONE;
    } else {
        error = mDevice->getBackingStore(handle, outBackingStore);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE,
            "getBackingStore(%p, ...): failed %d", handle, error);

    return error;
}

status_t GraphicBufferMapper::getStride(buffer_handle_t handle,
        uint32_t* outStride) const
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        mMapper->getStride(handle, outStride);
        error = GRALLOC1_ERROR_NONE;
    } else {
        error = mDevice->getStride(handle, outStride);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE, "getStride(%p, ...): failed %d",
            handle, error);

    return error;
}

status_t GraphicBufferMapper::lock(buffer_handle_t handle, uint32_t usage,
        const Rect& bounds, void** vaddr)
{
    return lockAsync(handle, usage, bounds, vaddr, -1);
}

status_t GraphicBufferMapper::lockYCbCr(buffer_handle_t handle, uint32_t usage,
        const Rect& bounds, android_ycbcr *ycbcr)
{
    return lockAsyncYCbCr(handle, usage, bounds, ycbcr, -1);
}

status_t GraphicBufferMapper::unlock(buffer_handle_t handle)
{
    int32_t fenceFd = -1;
    status_t error = unlockAsync(handle, &fenceFd);
    if (error == NO_ERROR) {
        sync_wait(fenceFd, -1);
        close(fenceFd);
    }
    return error;
}

status_t GraphicBufferMapper::lockAsync(buffer_handle_t handle,
        uint32_t usage, const Rect& bounds, void** vaddr, int fenceFd)
{
    return lockAsync(handle, usage, usage, bounds, vaddr, fenceFd);
}

status_t GraphicBufferMapper::lockAsync(buffer_handle_t handle,
        uint64_t producerUsage, uint64_t consumerUsage, const Rect& bounds,
        void** vaddr, int fenceFd)
{
    ATRACE_CALL();

    gralloc1_rect_t accessRegion = asGralloc1Rect(bounds);
    gralloc1_error_t error;
    if (mMapper->valid()) {
        const Gralloc2::IMapper::Rect& accessRect =
            *reinterpret_cast<Gralloc2::IMapper::Rect*>(&accessRegion);
        error = static_cast<gralloc1_error_t>(mMapper->lock(
                    handle, producerUsage, consumerUsage, accessRect,
                    fenceFd, vaddr));
    } else {
        sp<Fence> fence = new Fence(fenceFd);
        error = mDevice->lock(handle,
                static_cast<gralloc1_producer_usage_t>(producerUsage),
                static_cast<gralloc1_consumer_usage_t>(consumerUsage),
                &accessRegion, vaddr, fence);
    }

    ALOGW_IF(error != GRALLOC1_ERROR_NONE, "lock(%p, ...) failed: %d", handle,
            error);

    return error;
}

static inline bool isValidYCbCrPlane(const android_flex_plane_t& plane) {
    if (plane.bits_per_component != 8) {
        ALOGV("Invalid number of bits per component: %d",
                plane.bits_per_component);
        return false;
    }
    if (plane.bits_used != 8) {
        ALOGV("Invalid number of bits used: %d", plane.bits_used);
        return false;
    }

    bool hasValidIncrement = plane.h_increment == 1 ||
            (plane.component != FLEX_COMPONENT_Y && plane.h_increment == 2);
    hasValidIncrement = hasValidIncrement && plane.v_increment > 0;
    if (!hasValidIncrement) {
        ALOGV("Invalid increment: h %d v %d", plane.h_increment,
                plane.v_increment);
        return false;
    }

    return true;
}

status_t GraphicBufferMapper::lockAsyncYCbCr(buffer_handle_t handle,
        uint32_t usage, const Rect& bounds, android_ycbcr *ycbcr, int fenceFd)
{
    ATRACE_CALL();

    gralloc1_rect_t accessRegion = asGralloc1Rect(bounds);

    std::vector<android_flex_plane_t> planes;
    android_flex_layout_t flexLayout{};
    gralloc1_error_t error;

    if (mMapper->valid()) {
        const Gralloc2::IMapper::Rect& accessRect =
            *reinterpret_cast<Gralloc2::IMapper::Rect*>(&accessRegion);
        Gralloc2::FlexLayout layout{};
        error = static_cast<gralloc1_error_t>(mMapper->lock(
                    handle, usage, usage, accessRect, fenceFd, &layout));

        if (error == GRALLOC1_ERROR_NONE) {
            planes.resize(layout.planes.size());
            memcpy(planes.data(), layout.planes.data(),
                    sizeof(planes[0]) * planes.size());

            flexLayout.format = static_cast<android_flex_format_t>(
                    layout.format);
            flexLayout.num_planes = static_cast<uint32_t>(planes.size());
            flexLayout.planes = planes.data();
        }
    } else {
        sp<Fence> fence = new Fence(fenceFd);

        if (mDevice->hasCapability(GRALLOC1_CAPABILITY_ON_ADAPTER)) {
            error = mDevice->lockYCbCr(handle,
                    static_cast<gralloc1_producer_usage_t>(usage),
                    static_cast<gralloc1_consumer_usage_t>(usage),
                    &accessRegion, ycbcr, fence);
            ALOGW_IF(error != GRALLOC1_ERROR_NONE,
                    "lockYCbCr(%p, ...) failed: %d", handle, error);
            return error;
        }

        uint32_t numPlanes = 0;
        error = mDevice->getNumFlexPlanes(handle, &numPlanes);

        if (error != GRALLOC1_ERROR_NONE) {
            ALOGV("Failed to retrieve number of flex planes: %d", error);
            return error;
        }
        if (numPlanes < 3) {
            ALOGV("Not enough planes for YCbCr (%u found)", numPlanes);
            return GRALLOC1_ERROR_UNSUPPORTED;
        }

        planes.resize(numPlanes);
        flexLayout.num_planes = numPlanes;
        flexLayout.planes = planes.data();

        error = mDevice->lockFlex(handle,
                static_cast<gralloc1_producer_usage_t>(usage),
                static_cast<gralloc1_consumer_usage_t>(usage),
                &accessRegion, &flexLayout, fence);
    }

    if (error != GRALLOC1_ERROR_NONE) {
        ALOGW("lockFlex(%p, ...) failed: %d", handle, error);
        return error;
    }
    if (flexLayout.format != FLEX_FORMAT_YCbCr) {
        ALOGV("Unable to convert flex-format buffer to YCbCr");
        unlock(handle);
        return GRALLOC1_ERROR_UNSUPPORTED;
    }

    // Find planes
    auto yPlane = planes.cend();
    auto cbPlane = planes.cend();
    auto crPlane = planes.cend();
    for (auto planeIter = planes.cbegin(); planeIter != planes.cend();
            ++planeIter) {
        if (planeIter->component == FLEX_COMPONENT_Y) {
            yPlane = planeIter;
        } else if (planeIter->component == FLEX_COMPONENT_Cb) {
            cbPlane = planeIter;
        } else if (planeIter->component == FLEX_COMPONENT_Cr) {
            crPlane = planeIter;
        }
    }
    if (yPlane == planes.cend()) {
        ALOGV("Unable to find Y plane");
        unlock(handle);
        return GRALLOC1_ERROR_UNSUPPORTED;
    }
    if (cbPlane == planes.cend()) {
        ALOGV("Unable to find Cb plane");
        unlock(handle);
        return GRALLOC1_ERROR_UNSUPPORTED;
    }
    if (crPlane == planes.cend()) {
        ALOGV("Unable to find Cr plane");
        unlock(handle);
        return GRALLOC1_ERROR_UNSUPPORTED;
    }

    // Validate planes
    if (!isValidYCbCrPlane(*yPlane)) {
        ALOGV("Y plane is invalid");
        unlock(handle);
        return GRALLOC1_ERROR_UNSUPPORTED;
    }
    if (!isValidYCbCrPlane(*cbPlane)) {
        ALOGV("Cb plane is invalid");
        unlock(handle);
        return GRALLOC1_ERROR_UNSUPPORTED;
    }
    if (!isValidYCbCrPlane(*crPlane)) {
        ALOGV("Cr plane is invalid");
        unlock(handle);
        return GRALLOC1_ERROR_UNSUPPORTED;
    }
    if (cbPlane->v_increment != crPlane->v_increment) {
        ALOGV("Cb and Cr planes have different step (%d vs. %d)",
                cbPlane->v_increment, crPlane->v_increment);
        unlock(handle);
        return GRALLOC1_ERROR_UNSUPPORTED;
    }
    if (cbPlane->h_increment != crPlane->h_increment) {
        ALOGV("Cb and Cr planes have different stride (%d vs. %d)",
                cbPlane->h_increment, crPlane->h_increment);
        unlock(handle);
        return GRALLOC1_ERROR_UNSUPPORTED;
    }

    // Pack plane data into android_ycbcr struct
    ycbcr->y = yPlane->top_left;
    ycbcr->cb = cbPlane->top_left;
    ycbcr->cr = crPlane->top_left;
    ycbcr->ystride = static_cast<size_t>(yPlane->v_increment);
    ycbcr->cstride = static_cast<size_t>(cbPlane->v_increment);
    ycbcr->chroma_step = static_cast<size_t>(cbPlane->h_increment);

    return error;
}

status_t GraphicBufferMapper::unlockAsync(buffer_handle_t handle, int *fenceFd)
{
    ATRACE_CALL();

    gralloc1_error_t error;
    if (mMapper->valid()) {
        *fenceFd = mMapper->unlock(handle);
        error = GRALLOC1_ERROR_NONE;
    } else {
        sp<Fence> fence = Fence::NO_FENCE;
        error = mDevice->unlock(handle, &fence);
        if (error != GRALLOC1_ERROR_NONE) {
            ALOGE("unlock(%p) failed: %d", handle, error);
            return error;
        }

        *fenceFd = fence->dup();
    }
    return error;
}

// ---------------------------------------------------------------------------
}; // namespace android
