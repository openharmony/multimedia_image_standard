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
#ifndef EXIF_INFO_H
#define EXIF_INFO_H
#include <string>
#include "hilog/log.h"
#include "log_tags.h"
namespace OHOS {
namespace ImagePlugin {
/*
 * Class responsible for storing and parsing EXIF information from a JPEG blob
 */
class EXIFInfo {
public:
    EXIFInfo();
    ~EXIFInfo();
    /*
     * Parsing function for an entire JPEG image buffer.
     * PARAM 'data': A pointer to a JPEG image.
     * PARAM 'length': The length of the JPEG image.
     * RETURN:  PARSE_EXIF_SUCCESS (0) on succes with 'result' filled out
     *          error code otherwise, as defined by the PARSE_EXIF_ERROR_* macros
     */
    int ParseExifData(const unsigned char *buf, unsigned len);
    int ParseExifData(const std::string &data);
    bool ModifyExifData(const std::string &tag, const std::string &value, const std::string &path);

public:
    std::string bitsPerSample_; // Number of bits in each pixel of an image.
    std::string orientation_;
    std::string imageLength_;   // Image length.
    std::string imageWidth_;    // mage width.
    std::string gpsLatitude_;
    std::string gpsLongitude_;
    std::string gpsLatitudeRef_;
    std::string gpsLongitudeRef_;
    std::string dateTimeOriginal_;  // Original date and time.

private:
    void SetExifTagValues(const std::string &tag, const std::string &value);
    int GetImageFileDirectory(const std::string &tag);
    void* InitExifTag(std::string *exif, int ifd, std::string tag);
    void* CreateExifTag(std::string *exif, int ifd, std::string tag, size_t len, std::string format);
    long GetFileSize(FILE *fp);
    void ReleaseSource(unsigned char *buf, FILE *file);

private:
    int imageFileDirectory_;
    std::string* exifData_;
};
} // namespace ImagePlugin
} // namespace OHOS
#endif // EXIF_INFO_H
