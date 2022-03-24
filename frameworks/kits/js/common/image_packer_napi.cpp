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

#include "image_packer_napi.h"
#include "hilog/log.h"
#include "media_errors.h"
#include "image_napi_utils.h"
#include "image_packer.h"
#include "image_source.h"
#include "image_source_napi.h"
#include "pixel_map_napi.h"

using OHOS::HiviewDFX::HiLog;
namespace {
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = {LOG_CORE, LOG_DOMAIN, "ImagePackerNapi"};
    constexpr uint32_t NUM_0 = 0;
    constexpr uint32_t NUM_1 = 1;
    constexpr uint32_t NUM_2 = 2;
}

namespace OHOS {
namespace Media {
static const std::string CLASS_NAME_IMAGEPACKER = "ImagePacker";
std::shared_ptr<ImagePacker> ImagePackerNapi::sImgPck_ = nullptr;
std::shared_ptr<ImageSource> ImagePackerNapi::sImgSource_ = nullptr;
thread_local napi_ref ImagePackerNapi::sConstructor_ = nullptr;

const int ARGS_THREE = 3;
const int PARAM0 = 0;
const int PARAM1 = 1;
const int PARAM2 = 2;
const int32_t SIZE = 100;
const int32_t TYPE_IMAGE_SOURCE = 1;
const int32_t TYPE_PIXEL_MAP = 2;

struct ImagePackerAsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef = nullptr;
    ImagePackerNapi *constructor_;
    bool status = false;
    std::shared_ptr<ImageSource> rImageSource;
    PackOption packOption;
    std::shared_ptr<ImagePacker> rImagePacker;
    std::shared_ptr<PixelMap> rPixelMap;
    void *resultBuffer = nullptr;
    int64_t packedSize = 0;
};

struct PackingOption {
    std::string format;
    uint8_t quality = 100;
};

ImagePackerNapi::ImagePackerNapi()
    :env_(nullptr), wrapper_(nullptr)
{}

ImagePackerNapi::~ImagePackerNapi()
{
    if (wrapper_ != nullptr) {
        napi_delete_reference(env_, wrapper_);
    }
}

static void CommonCallbackRoutine(napi_env env, ImagePackerAsyncContext* &connect, const napi_value &valueParam)
{
    HiLog::Debug(LABEL, "CommonCallbackRoutine enter");
    napi_value result[NUM_2] = {0};
    napi_value retVal;
    napi_value callback = nullptr;

    napi_get_undefined(env, &result[NUM_0]);
    napi_get_undefined(env, &result[NUM_1]);

    if (connect->status == SUCCESS) {
        result[1] = valueParam;
    }

    if (connect->deferred) {
        if (connect->status == SUCCESS) {
            napi_resolve_deferred(env, connect->deferred, result[NUM_1]);
        } else {
            napi_reject_deferred(env, connect->deferred, result[NUM_0]);
        }
    } else {
        napi_get_reference_value(env, connect->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, PARAM2, result, &retVal);
        napi_delete_reference(env, connect->callbackRef);
    }

    napi_delete_async_work(env, connect->work);

    delete connect;
    connect = nullptr;
    HiLog::Debug(LABEL, "CommonCallbackRoutine exit");
}

STATIC_EXEC_FUNC(Packing)
{
    HiLog::Debug(LABEL, "PackingExec enter");
    uint64_t bufferSize = 10 * 1024 * 1024; // 10M is the maximum packedSize
    int64_t packedSize = 0;
    auto context = static_cast<ImagePackerAsyncContext*>(data);
    HiLog::Debug(LABEL, "image packer get supported format");
    std::set<std::string> formats;
    uint32_t ret = context->rImagePacker->GetSupportedFormats(formats);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "image packer get supported format failed, ret=%{public}u.", ret);
    }

    uint32_t errorCode = 0;
    HiLog::Debug(LABEL, "image packer GetSourceInfo format");
    SourceInfo sourceInfo = context->rImageSource->GetSourceInfo(errorCode);
    HiLog::Debug(LABEL, "image packer GetSourceInfo format, ret=%{public}u.", errorCode);

    context->resultBuffer = malloc(bufferSize);
    if (context->resultBuffer == nullptr) {
        HiLog::Error(LABEL, "PackingExec failed, malloc buffer failed");

        context->status = ERROR;
    } else {
        context->rImagePacker->StartPacking(static_cast<uint8_t *>(context->resultBuffer),
            bufferSize, context->packOption);
        context->rImagePacker->AddImage(*(context->rImageSource));
        context->rImagePacker->FinalizePacking(packedSize);
        HiLog::Debug(LABEL, "packedSize=%{public}lld.", static_cast<long long>(packedSize));

        if (packedSize > 0 && packedSize < bufferSize) {
            context->packedSize = packedSize;
            context->status = SUCCESS;
        } else {
            context->status = ERROR;
        }
    }
    HiLog::Debug(LABEL, "PackingExec exit");
}

STATIC_COMPLETE_FUNC(Packing)
{
    HiLog::Debug(LABEL, "PackingComplete enter");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    auto context = static_cast<ImagePackerAsyncContext*>(data);

    if (!ImageNapiUtils::CreateArrayBuffer(env, context->resultBuffer,
                                           context->packedSize, &result)) {
        context->status = ERROR;
        HiLog::Error(LABEL, "napi_create_arraybuffer failed!");
        napi_get_undefined(env, &result);
    } else {
        context->status = SUCCESS;
    }
    HiLog::Debug(LABEL, "PackingComplete exit");
    CommonCallbackRoutine(env, context, result);
}

STATIC_EXEC_FUNC(PackingFromPixelMap)
{
    HiLog::Debug(LABEL, "PackingFromPixelMapExec enter");
    uint64_t bufferSize = 2 * 1024 * 1024;
    int64_t packedSize = 0;
    auto context = static_cast<ImagePackerAsyncContext*>(data);
    HiLog::Debug(LABEL, "image packer get supported format");
    std::set<std::string> formats;
    uint32_t ret = context->rImagePacker->GetSupportedFormats(formats);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "image packer get supported format failed, ret=%{public}u.", ret);
    }

    context->resultBuffer = malloc(bufferSize);
    if (context->resultBuffer == nullptr) {
        HiLog::Error(LABEL, "PackingFromPixelMapExec failed, malloc buffer failed");

        context->status = ERROR;
    } else {
        context->rImagePacker->StartPacking(static_cast<uint8_t *>(context->resultBuffer),
            bufferSize, context->packOption);
        context->rImagePacker->AddImage(*(context->rPixelMap));
        context->rImagePacker->FinalizePacking(packedSize);
        HiLog::Debug(LABEL, "packedSize=%{public}lld.", static_cast<long long>(packedSize));

        if (packedSize > 0) {
            context->packedSize = packedSize;
            context->status = SUCCESS;
        } else {
            context->status = ERROR;
        }
    }
    HiLog::Debug(LABEL, "PackingFromPixelMapExec exit");
}

napi_value ImagePackerNapi::Init(napi_env env, napi_value exports)
{
    napi_property_descriptor props[] = {
        DECLARE_NAPI_FUNCTION("packing", Packing),
        DECLARE_NAPI_FUNCTION("packingFromPixelMap", PackingFromPixelMap),
        DECLARE_NAPI_FUNCTION("release", Release),
        DECLARE_NAPI_GETTER("supportedFormats", GetSupportedFormats),
    };
    napi_property_descriptor static_prop[] = {
        DECLARE_NAPI_STATIC_FUNCTION("createImagePacker", CreateImagePacker),
    };

    napi_value constructor = nullptr;

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(
        napi_define_class(env, CLASS_NAME_IMAGEPACKER.c_str(), NAPI_AUTO_LENGTH, Constructor,
        nullptr, IMG_ARRAY_SIZE(props), props, &constructor)), nullptr,
        HiLog::Error(LABEL, "define class fail")
    );

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(
        napi_create_reference(env, constructor, 1, &sConstructor_)),
        nullptr,
        HiLog::Error(LABEL, "create reference fail")
    );

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(
        napi_set_named_property(env, exports, CLASS_NAME_IMAGEPACKER.c_str(), constructor)),
        nullptr,
        HiLog::Error(LABEL, "set named property fail")
    );
    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(
        napi_define_properties(env, exports, IMG_ARRAY_SIZE(static_prop), static_prop)),
        nullptr,
        HiLog::Error(LABEL, "define properties fail")
    );

    HiLog::Debug(LABEL, "Init success");
    return exports;
}

napi_value ImagePackerNapi::Constructor(napi_env env, napi_callback_info info)
{
    napi_value undefineVar = nullptr;
    napi_get_undefined(env, &undefineVar);

    napi_status status;
    napi_value thisVar = nullptr;

    HiLog::Debug(LABEL, "Constructor in");
    status = napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
    if (status == napi_ok && thisVar != nullptr) {
        std::unique_ptr<ImagePackerNapi> pImgPackerNapi = std::make_unique<ImagePackerNapi>();
        if (pImgPackerNapi != nullptr) {
            pImgPackerNapi->env_ = env;
            pImgPackerNapi->nativeImgPck = sImgPck_;
            status = napi_wrap(env, thisVar, reinterpret_cast<void *>(pImgPackerNapi.get()),
                               ImagePackerNapi::Destructor, nullptr, &(pImgPackerNapi->wrapper_));
            if (status == napi_ok) {
                pImgPackerNapi.release();
                return thisVar;
            } else {
                HiLog::Error(LABEL, "Failure wrapping js to native napi");
            }
        }
    }

    return undefineVar;
}

napi_value ImagePackerNapi::CreateImagePacker(napi_env env, napi_callback_info info)
{
    napi_value constructor = nullptr;
    napi_value result = nullptr;
    napi_status status;

    HiLog::Debug(LABEL, "CreateImagePacker IN");
    std::shared_ptr<ImagePacker> imagePacker = std::make_shared<ImagePacker>();
    status = napi_get_reference_value(env, sConstructor_, &constructor);
    if (IMG_IS_OK(status)) {
        sImgPck_ = imagePacker;
        status = napi_new_instance(env, constructor, 0, nullptr, &result);
        if (status == napi_ok) {
            return result;
        } else {
            HiLog::Error(LABEL, "New instance could not be obtained");
        }
    }
    return result;
}

void ImagePackerNapi::Destructor(napi_env env, void *nativeObject, void *finalize)
{
    ImagePackerNapi *pImagePackerNapi = reinterpret_cast<ImagePackerNapi*>(nativeObject);

    if (IMG_NOT_NULL(pImagePackerNapi)) {
        pImagePackerNapi->~ImagePackerNapi();
    }
}

static bool parsePackOptions(napi_env env, napi_value root, PackOption* opts)
{
    char buffer[SIZE];
    napi_value property = nullptr;
    bool present = false;
    size_t res = 0;
    uint32_t len = 0;
    napi_value stringItem = nullptr;
    int32_t quality = 0;
    HiLog::Debug(LABEL, "GetPackingOptionsParam IN");

    napi_has_named_property(env, root, "format", &present);
    if (present && napi_get_named_property(env, root, "format", &property) == napi_ok) {
        HiLog::Debug(LABEL, "GetPackingOptionsParam IN 1");
        napi_get_array_length(env, property, &len);
        for (size_t i = 0; i < len; i++) {
            napi_get_element(env, property, i, &stringItem);
            napi_get_value_string_utf8(env, stringItem, buffer, SIZE, &res);
            opts->format = std::string(buffer);
            HiLog::Debug(LABEL, "format is %{public}s.", opts->format.c_str());
            if (memset_s(buffer, SIZE, 0, sizeof(buffer)) != EOK) {
                HiLog::Error(LABEL, "memset buffer failed.");
                free(buffer);
            }
        }
    }
    present = false;
    napi_has_named_property(env, root, "quality", &present);
    if (present) {
        if (napi_get_named_property(env, root, "quality", &property) != napi_ok) {
            HiLog::Error(LABEL, "Could not get the quality argument!");
        } else {
            napi_get_value_int32(env, property, &quality);
            opts->quality = quality;
            HiLog::Debug(LABEL, "quality is %{public}d.", quality);
        }
    }
    return true;
}

static int32_t ParserPackingArguments(napi_env env, napi_value argv)
{
    napi_value constructor = nullptr;
    napi_value global = nullptr;
    bool isInstance = false;
    napi_status ret = napi_invalid_arg;

    napi_get_global(env, &global);

    ret = napi_get_named_property(env, global, "ImageSource", &constructor);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "Get ImageSourceNapi property failed!");
    }

    ret = napi_instanceof(env, argv, constructor, &isInstance);
    if (ret == napi_ok && isInstance) {
        HiLog::Debug(LABEL, "This is ImageSourceNapi type!");
        return TYPE_IMAGE_SOURCE;
    }

    ret = napi_get_named_property(env, global, "PixelMap", &constructor);
    if (ret != napi_ok) {
        HiLog::Error(LABEL, "Get PixelMapNapi property failed!");
    }

    ret = napi_instanceof(env, argv, constructor, &isInstance);
    if (ret == napi_ok && isInstance) {
        HiLog::Debug(LABEL, "This is PixelMapNapi type!");
        return TYPE_PIXEL_MAP;
    }

    HiLog::Error(LABEL, "Inalued type!");
    return TYPE_IMAGE_SOURCE;
}

napi_value ImagePackerNapi::Packing(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    size_t argc = ARGS_THREE;
    napi_value argv[ARGS_THREE] = {0};
    napi_value thisVar = nullptr;
    int32_t refCount = 1;
    int32_t packType = TYPE_IMAGE_SOURCE;

    std::shared_ptr<ImageSource> imagesourceObj = nullptr;
    std::unique_ptr<PixelMap> pixelMap = nullptr;
    HiLog::Debug(LABEL, "Packing IN");

    IMG_JS_ARGS(env, info, status, argc, argv, thisVar);
    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "fail to napi_get_cb_info"));

    std::unique_ptr<ImagePackerAsyncContext> asyncContext = std::make_unique<ImagePackerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->constructor_));
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->constructor_),
        nullptr, HiLog::Error(LABEL, "fail to unwrap constructor_"));

    asyncContext->rImagePacker = std::move(asyncContext->constructor_->nativeImgPck);
    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype argvType = ImageNapiUtils::getType(env, argv[i]);
        if (i == PARAM0 && argvType == napi_object) {
            packType = ParserPackingArguments(env, argv[0]);
            if (packType == TYPE_IMAGE_SOURCE) {
                std::shared_ptr<ImageSourceNapi> imageSourceNapi = std::make_unique<ImageSourceNapi>();
                status = napi_unwrap(env, argv[i], reinterpret_cast<void**>(&imageSourceNapi));
                IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, imageSourceNapi), nullptr,
                    HiLog::Error(LABEL, "fail to unwrap ImageSourceNapi"));
                asyncContext->rImageSource = imageSourceNapi->nativeImgSrc;
                IMG_NAPI_CHECK_RET_D(IMG_IS_OK(asyncContext->rImageSource == nullptr), nullptr,
                    HiLog::Error(LABEL, "fail to napi_get rImageSource"));
            } else {
                asyncContext->rPixelMap = PixelMapNapi::GetPixelMap(env, argv[i]);
                IMG_NAPI_CHECK_RET_D(IMG_IS_OK(asyncContext->rPixelMap == nullptr), nullptr,
                    HiLog::Error(LABEL, "fail to napi_get rPixelMap"));
            }
        } else if (i == PARAM1 && ImageNapiUtils::getType(env, argv[i]) == napi_object) {
            IMG_NAPI_CHECK_RET_D(parsePackOptions(env, argv[i], &(asyncContext->packOption)),
                nullptr, HiLog::Error(LABEL, "PackOptions mismatch"));
        } else if (i == PARAM2 && ImageNapiUtils::getType(env, argv[i]) == napi_function) {
            napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
            break;
        }
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }

    ImageNapiUtils::HicheckerReport();

    if (packType == TYPE_IMAGE_SOURCE) {
        IMG_CREATE_CREATE_ASYNC_WORK(env, status, "Packing",
            PackingExec, PackingComplete, asyncContext, asyncContext->work);
    } else {
        IMG_CREATE_CREATE_ASYNC_WORK(env, status, "PackingFromPixelMap",
            PackingFromPixelMapExec, PackingComplete, asyncContext, asyncContext->work);
    }

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to create async work"));
    return result;
}

napi_value ImagePackerNapi::PackingFromPixelMap(napi_env env, napi_callback_info info)
{
    napi_status status;
    napi_value result = nullptr;
    size_t argc = ARGS_THREE;
    napi_value argv[ARGS_THREE] = {0};
    napi_value thisVar = nullptr;
    int32_t refCount = 1;

    std::shared_ptr<ImageSource> imagesourceObj = nullptr;
    std::unique_ptr<PixelMap> pixelMap = nullptr;
    HiLog::Debug(LABEL, "PackingFromPixelMap IN");

    IMG_JS_ARGS(env, info, status, argc, argv, thisVar);
    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), nullptr, HiLog::Error(LABEL, "fail to napi_get_cb_info"));

    std::unique_ptr<ImagePackerAsyncContext> asyncContext = std::make_unique<ImagePackerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&asyncContext->constructor_));
    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, asyncContext->constructor_),
        nullptr, HiLog::Error(LABEL, "fail to unwrap constructor_"));

    asyncContext->rImagePacker = std::move(asyncContext->constructor_->nativeImgPck);
    for (size_t i = PARAM0; i < argc; i++) {
        napi_valuetype argvType = ImageNapiUtils::getType(env, argv[i]);
        if (i == PARAM0 && argvType == napi_object) {
            asyncContext->rPixelMap = PixelMapNapi::GetPixelMap(env, argv[i]);
            IMG_NAPI_CHECK_RET_D(IMG_IS_OK(asyncContext->rPixelMap == nullptr), nullptr,
                HiLog::Error(LABEL, "fail to napi_get rPixelMap"));
        } else if (i == PARAM1 &&argvType == napi_object) {
            IMG_NAPI_CHECK_RET_D(parsePackOptions(env, argv[i], &(asyncContext->packOption)),
                nullptr, HiLog::Error(LABEL, "PackOptions mismatch"));
        } else if (i == PARAM2 && argvType == napi_function) {
            napi_create_reference(env, argv[i], refCount, &asyncContext->callbackRef);
            break;
        }
    }

    if (asyncContext->callbackRef == nullptr) {
        napi_create_promise(env, &(asyncContext->deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }

    IMG_CREATE_CREATE_ASYNC_WORK(env, status, "PackingFromPixelMap",
        PackingFromPixelMapExec, PackingComplete, asyncContext, asyncContext->work);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status),
        nullptr, HiLog::Error(LABEL, "fail to create async work"));
    return result;
}

napi_value ImagePackerNapi::GetSupportedFormats(napi_env env, napi_callback_info info)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    napi_status status;
    napi_value thisVar = nullptr;
    size_t argCount = 0;
    HiLog::Debug(LABEL, "GetSupportedFormats IN");

    IMG_JS_ARGS(env, info, status, argCount, nullptr, thisVar);

    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), result, HiLog::Error(LABEL, "fail to napi_get_cb_info"));

    std::unique_ptr<ImagePackerAsyncContext> context = std::make_unique<ImagePackerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&context->constructor_));

    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, context->constructor_),
        nullptr, HiLog::Error(LABEL, "fail to unwrap context"));
    std::set<std::string> formats;
    uint32_t ret = context->constructor_->nativeImgPck->GetSupportedFormats(formats);

    IMG_NAPI_CHECK_RET_D((ret == SUCCESS),
        nullptr, HiLog::Error(LABEL, "fail to get supported formats"));

    napi_create_array(env, &result);
    size_t i = 0;
    for (const std::string& formatStr: formats) {
        napi_value format = nullptr;
        napi_create_string_latin1(env, formatStr.c_str(), formatStr.length(), &format);
        napi_set_element(env, result, i, format);
        i++;
    }
    return result;
}

static void ReleaseComplete(napi_env env, napi_status status, void *data)
{
    HiLog::Debug(LABEL, "ReleaseComplete IN");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    auto context = static_cast<ImagePackerAsyncContext*>(data);
    context->constructor_->~ImagePackerNapi();
    HiLog::Debug(LABEL, "ReleaseComplete OUT");
    CommonCallbackRoutine(env, context, result);
}

napi_value ImagePackerNapi::Release(napi_env env, napi_callback_info info)
{
    HiLog::Debug(LABEL, "Release enter");
    napi_value result = nullptr;
    napi_get_undefined(env, &result);

    int32_t refCount = 1;
    napi_status status;
    napi_value thisVar = nullptr;
    napi_value argValue[NUM_1] = {0};
    size_t argCount = 1;

    IMG_JS_ARGS(env, info, status, argCount, argValue, thisVar);
    HiLog::Debug(LABEL, "Release argCount is [%{public}zu]", argCount);
    IMG_NAPI_CHECK_RET_D(IMG_IS_OK(status), result, HiLog::Error(LABEL, "fail to napi_get_cb_info"));

    std::unique_ptr<ImagePackerAsyncContext> context = std::make_unique<ImagePackerAsyncContext>();
    status = napi_unwrap(env, thisVar, reinterpret_cast<void**>(&context->constructor_));

    IMG_NAPI_CHECK_RET_D(IMG_IS_READY(status, context->constructor_), result,
        HiLog::Error(LABEL, "fail to unwrap context"));
    HiLog::Debug(LABEL, "Release argCount is [%{public}zu]", argCount);
    if (argCount == 1 && ImageNapiUtils::getType(env, argValue[NUM_0]) == napi_function) {
        napi_create_reference(env, argValue[NUM_0], refCount, &context->callbackRef);
    }

    if (context->callbackRef == nullptr) {
        napi_create_promise(env, &(context->deferred), &result);
    }

    IMG_CREATE_CREATE_ASYNC_WORK(env, status, "Release",
        [](napi_env env, void *data) {}, ReleaseComplete, context, context->work);
    HiLog::Debug(LABEL, "Release exit");
    return result;
}
}  // namespace Media
}  // namespace OHOS
