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
 
#include <gtest/gtest.h>
#include "jpeg_decoder.h"
#include <map>
#include "jerror.h"
#include "media_errors.h"
#include "string_ex.h"
#ifndef _WIN32
#include "securec.h"
#else
#include "memory.h"
#endif

using namespace testing::ext;
using namespace OHOS::Media;

namespace OHOS {
namespace Multimedia {
class ImagePluginsLibjpegpluginTest : public testing::Test {
public:
    ImagePluginsLibjpegpluginTest() {}
    ~ImagePluginsLibjpegpluginTest() {}
};

/**
 * @tc.name: Libjpegplugin001
 * @tc.desc: test ProcessPremulF16Pixel
 * @tc.type: FUNC
 */
HWTEST_F(ImagePluginsLibjpegpluginTest, Libjpegplugin001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImagePluginsLibjpegpluginTest: Libjpegplugin001 start";
    /**
     * @tc.steps: step1. set index, key, value.
     * @tc.expected: step1. The new value is default_exif_value.
     */
    uint32_t index = 0;
    std::string value = "default_exif_value";
    std::string key = "BitsPerSample";
    uint32_t ret = JpegDecoder::GetImagePropertyString(index, key, value);
    EXPECT_EQ(res, true);
    GTEST_LOG_(INFO) << "ImagePluginsLibjpegpluginTest: Libjpegplugin001 end";
}

/**
 * @tc.name: Libjpegplugin001
 * @tc.desc: test ProcessPremulF16Pixel
 * @tc.type: FUNC
 */
HWTEST_F(ImagePluginsLibjpegpluginTest, Libjpegplugin001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImagePluginsLibjpegpluginTest: Libjpegplugin001 start";
    /**
     * @tc.steps: step1. set index, key, value.
     * @tc.expected: step1. The new value is 0.
     */
    uint32_t index = 0;
    std::string value = "0";
    std::string key = "BitsPerSample";
    uint32_t ret = JpegDecoder::GetImagePropertyString(index, key, value);
    EXPECT_EQ(res, true);
    GTEST_LOG_(INFO) << "ImagePluginsLibjpegpluginTest: Libjpegplugin001 end";
}
}
}