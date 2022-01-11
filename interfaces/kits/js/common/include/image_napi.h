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

#ifndef IMAGE_NAPI_H_
#define IMAGE_NAPI_H_

#include <cerrno>
#include <dirent.h>
#include <fcntl.h>
#include <ftw.h>
#include <securec.h>
#include <sys/stat.h>
#include <unistd.h>
#include <variant>

#include <surface.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS {
namespace Media {
struct ImageAsyncContext;
class ImageNapi {
public:
    ImageNapi();
    ~ImageNapi();
    static napi_value Init(napi_env env, napi_value exports);
    static napi_value Create(napi_env env, sptr<SurfaceBuffer> surfaceBuffer);
    void NativeRelease();

private:
    static napi_value Constructor(napi_env env, napi_callback_info info);
    static void Destructor(napi_env env, void *nativeObject, void *finalize);

    static napi_value JSGetClipRect(napi_env env, napi_callback_info info);
    static napi_value JsGetSize(napi_env env, napi_callback_info info);
    static napi_value JsGetFormat(napi_env env, napi_callback_info info);
    static napi_value JsGetComponent(napi_env env, napi_callback_info info);
    static napi_value JsRelease(napi_env env, napi_callback_info info);

    static napi_value BuildComponent(napi_env env, napi_callback_info info);
    static std::unique_ptr<ImageAsyncContext> UnwarpContext(napi_env env, napi_callback_info info);
    static void JsGetComponentCallBack(napi_env env, napi_status status, ImageAsyncContext* context);

    static napi_ref sConstructor_;
    static sptr<SurfaceBuffer> staticInstance_;

    napi_env env_ = nullptr;
    napi_ref wrapper_ = nullptr;
    sptr<SurfaceBuffer> sSurfaceBuffer_;
};

struct ImageAsyncContext {
    napi_env env = nullptr;
    napi_async_work work = nullptr;
    napi_deferred deferred = nullptr;
    napi_ref callbackRef = nullptr;
    ImageNapi *constructor_ = nullptr;
    uint32_t status;
    int32_t componentType;
};
} // namespace Media
} // namespace OHOS
#endif /* IMAGE_NAPI_H_ */
