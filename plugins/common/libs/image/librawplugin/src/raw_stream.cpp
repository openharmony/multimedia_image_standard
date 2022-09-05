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
#include "raw_stream.h"
#include "hilog/log.h"
#include "log_tags.h"
namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "RawStream" };
}

RawStream::RawStream(InputDataStream &sourceStream)
{
    HiLog::Debug(LABEL, "create IN");

    inputStream_ = &sourceStream;

    HiLog::Debug(LABEL, "create OUT");
}

RawStream::~RawStream()
{
    HiLog::Debug(LABEL, "release IN");

    inputStream_ = nullptr;

    HiLog::Debug(LABEL, "release OUT");
}

// api for piex::StreamInterface
piex::Error RawStream::GetData(const size_t offset, const size_t length, uint8_t* data)
{
    if (inputStream_ == nullptr) {
        HiLog::Error(LABEL, "GetData, InputStream is null");
        return piex::kUnsupported;
    }

    uint32_t u32Offset = static_cast<uint32_t>(offset);
    uint32_t u32Length = static_cast<uint32_t>(length);

    if (inputStream_->Tell() != u32Offset) {
        if (!inputStream_->Seek(u32Offset)) {
            HiLog::Error(LABEL, "GetData, seek fail");
            return piex::kFail;
        }

        if (inputStream_->Tell() != u32Offset) {
            HiLog::Error(LABEL, "GetData, seeked fail");
            return piex::kFail;
        }
    }

    uint32_t readSize = 0;
    if (!inputStream_->Read(u32Length, data, u32Length, readSize)) {
        HiLog::Error(LABEL, "GetData, read fail");
        return piex::kFail;
    }

    if (readSize != u32Length) {
        HiLog::Error(LABEL, "GetData, read want:%{public}u, real:%{public}u", u32Length, readSize);
        return piex::kFail;
    }

    return piex::kOk;
}
} // namespace ImagePlugin
} // namespace OHOS
