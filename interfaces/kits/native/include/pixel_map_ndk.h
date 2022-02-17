/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * @file pixel_map_ndk.h
 *
 * @brief Declares functions for you to lock and access or unlock pixel data,
 * and obtain the width and height of a pixel map.
 *
 * @since 8
 * @version 1.0
 */

#ifndef PIXEL_MAP_NDK_H
#define PIXEL_MAP_NDK_H

#include "napi/native_api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct OhosPixelMapInfo {
    /** Image width, in pixels. */
    uint32_t width;

    /** Image height, in pixels. */
    uint32_t height;

    /** Number of bytes in each row of a pixel map */
    uint32_t rowSize;

    /** Pixel format */
    int32_t pixelFormat;
};

int32_t OH_GetImageInfo(napi_env env, napi_value value, OhosPixelMapInfo *info);

int32_t OH_AccessPixels(napi_env env, napi_value value, void** addrPtr);

int32_t OH_UnAccessPixels(napi_env env, napi_value value);

#ifdef __cplusplus
}
#endif

#endif // PIXEL_MAP_NDK_H
