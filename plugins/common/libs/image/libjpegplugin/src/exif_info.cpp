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
#include "exif_info.h"
#include <algorithm>
#include <cstdio>
#include "string_ex.h"

namespace OHOS {
namespace ImagePlugin {
namespace {
    using namespace OHOS::HiviewDFX;
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "Exif" };
    static const int PARSE_EXIF_SUCCESS = 0;
}

EXIFInfo::EXIFInfo() : imageFileDirectory_(0), exifData_(nullptr)
{
}

EXIFInfo::~EXIFInfo()
{
    if (exifData_ != nullptr) {
        exifData_ = nullptr;
    }
}

int EXIFInfo::ParseExifData(const unsigned char *buf, unsigned len)
{
    return PARSE_EXIF_SUCCESS;
}

int EXIFInfo::ParseExifData(const std::string &data)
{
    return ParseExifData((const unsigned char *)data.data(), data.length());
}

void EXIFInfo::SetExifTagValues(const std::string &tag, const std::string &value)
{
    HiLog::Error(LABEL, "No match tag name!");
}

bool EXIFInfo::ModifyExifData(const std::string &tag, const std::string &value, const std::string &path)
{
    if (imageFileDirectory_ == 0) {
        return false;
    }
    return true;
}

int EXIFInfo::GetImageFileDirectory(const std::string &tag)
{
    return 0;
}

void* EXIFInfo::InitExifTag(std::string *exif, int ifd, std::string tag)
{
    return nullptr;
}

void* EXIFInfo::CreateExifTag(std::string *exif, int ifd, std::string tag,
    size_t len, std::string format)
{
    return nullptr;
}

long EXIFInfo::GetFileSize(FILE *fp)
{
    long int position;
    long size;

    /* Save the current position. */
    position = ftell(fp);
    
    /* Jump to the end of the file. */
    fseek(fp, 0L, SEEK_END);

    /* Get the end position. */
    size = ftell(fp);

    /* Jump back to the original position. */
    fseek(fp, position, SEEK_SET);

    return size;
}

void EXIFInfo::ReleaseSource(unsigned char *buf, FILE *file)
{
    if (buf) {
        free(buf);
    }
    buf = nullptr;
    fclose(file);
    file = nullptr;
}
} // namespace ImagePlugin
} // namespace OHOS
