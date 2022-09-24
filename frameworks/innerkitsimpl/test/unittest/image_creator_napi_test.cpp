/*
 * Copyright (C) 2022 Huawei Device Co., Ltd.
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
 
#include "image_creator_napi.h"
#include <uv.h>
#include "media_errors.h"
#include "hilog/log.h"
#include "image_napi_utils.h"
#include "image_creator_context.h"
#include "image_napi.h"
#include "image_creator_manager.h"

using namespace testing::ext;
using namespace OHOS::Media;

namespace OHOS {
namespace Multimedia {
class ImageCreatorNapiTest : public testing::Test {
public:
    ImageCreatorNapiTest() {}
    ~ImageCreatorNapiTest() {}
};

/**
 * @tc.name: ImageCreatorNapi001
 * @tc.desc: test ProcessPremulF16Pixel
 * @tc.type: FUNC
 */
HWTEST_F(ImageCreatorNapiTest, ImageCreatorNapi001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImageCreatorNapiTest: ImageCreatorNapi001 start";
    /**
     * @tc.steps: step1. set env, info.
     * @tc.expected: step1. The new env and info are nullptr.
     */
    napi_env env = nullptr;
    napi_callback_info info = nullptr;
    uint32_t ret = ImageCreatorNapi::JsTest(env, info);
    EXPECT_EQ(res, true);
    GTEST_LOG_(INFO) << "ImageCreatorNapiTest: ImageCreatorNapi001 end";
}
}
}