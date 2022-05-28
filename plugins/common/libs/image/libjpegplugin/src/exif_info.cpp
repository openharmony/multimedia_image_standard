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
#include <unistd.h>
#include "media_errors.h"
#include "string_ex.h"
#include "securec.h"

namespace OHOS {
namespace ImagePlugin {
namespace {
    using namespace OHOS::HiviewDFX;
    constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "Exif" };
    static const int PARSE_EXIF_SUCCESS = 0;
    static const int PARSE_EXIF_DATA_ERROR = 10001;
    static const int PARSE_EXIF_IFD_ERROR = 10002;
    static const int BUFFER_POSITION_4 = 4;
    static const int BUFFER_POSITION_5 = 5;
    static const int BUFFER_POSITION_6 = 6;
    static const int BUFFER_POSITION_7 = 7;
    static const int BUFFER_POSITION_8 = 8;
    static const int BUFFER_POSITION_9 = 9;
    static const int BUFFER_POSITION_12 = 12;
    static const int BUFFER_POSITION_13 = 13;
    static const int LENGTH_OFFSET_2 = 2;
    static const int MOVE_OFFSET_8 = 8;
    static const int CONSTANT_2 = 2;
    static const unsigned long MAX_FILE_SIZE = 1000 * 1000 * 1000;

    /* raw EXIF header data */
    static const unsigned char exifHeader[] = {
        0xff, 0xd8, 0xff, 0xe1
    };
}

EXIFInfo::EXIFInfo() : imageFileDirectory_(EXIF_IFD_COUNT), exifData_(nullptr)
{
}

EXIFInfo::~EXIFInfo()
{
    if (exifData_ != nullptr) {
        exif_data_unref(exifData_);
        exifData_ = nullptr;
    }
}

int EXIFInfo::ParseExifData(const unsigned char *buf, unsigned len)
{
    HiLog::Debug(LABEL, "ParseExifData ENTER");
    exifData_ = exif_data_new_from_data(buf, len);
    if (!exifData_) {
        return PARSE_EXIF_DATA_ERROR;
    }
    exif_data_foreach_content(exifData_,
        [](ExifContent *ec, void *userData) {
            ExifIfd ifd = exif_content_get_ifd(ec);
            ((EXIFInfo*)userData)->imageFileDirectory_ = ifd;
            if (ifd == EXIF_IFD_COUNT) {
                HiLog::Debug(LABEL, "GetIfd ERROR");
                return;
            }
            exif_content_foreach_entry(ec,
                [](ExifEntry *ee, void* userData) {
                    char tagValueChar[1024];
                    exif_entry_get_value(ee, tagValueChar, sizeof(tagValueChar));
                    std::string tagValueStr(&tagValueChar[0], &tagValueChar[strlen(tagValueChar)]);
                    ((EXIFInfo*)userData)->SetExifTagValues(ee->tag, tagValueStr);
                }, userData);
        }, this);

    if (imageFileDirectory_ == EXIF_IFD_COUNT) {
        return PARSE_EXIF_IFD_ERROR;
    }
    return PARSE_EXIF_SUCCESS;
}

int EXIFInfo::ParseExifData(const std::string &data)
{
    return ParseExifData((const unsigned char *)data.data(), data.length());
}

void EXIFInfo::SetExifTagValues(const ExifTag &tag, const std::string &value)
{
    if (tag == EXIF_TAG_BITS_PER_SAMPLE) {
        bitsPerSample_ = value;
    } else if (tag == EXIF_TAG_ORIENTATION) {
        orientation_ = value;
    } else if (tag == EXIF_TAG_IMAGE_LENGTH) {
        imageLength_ = value;
    } else if (tag == EXIF_TAG_IMAGE_WIDTH) {
        imageWidth_ = value;
    } else if (tag == EXIF_TAG_GPS_LATITUDE) {
        gpsLatitude_ = value;
    } else if (tag == EXIF_TAG_GPS_LONGITUDE) {
        gpsLongitude_ = value;
    } else if (tag == EXIF_TAG_GPS_LATITUDE_REF) {
        gpsLatitudeRef_ = value;
    } else if (tag == EXIF_TAG_GPS_LONGITUDE_REF) {
        gpsLongitudeRef_ = value;
    } else if (tag == EXIF_TAG_DATE_TIME_ORIGINAL) {
        dateTimeOriginal_ = value;
    } else if (tag == EXIF_TAG_EXPOSURE_TIME) {
        exposureTime_ = value;
    } else if (tag == EXIF_TAG_FNUMBER) {
        fNumber_ = value;
    } else if (tag == EXIF_TAG_ISO_SPEED_RATINGS) {
        isoSpeedRatings_ = value;
    } else if (tag == EXIF_TAG_SCENE_TYPE) {
        sceneType_ = value;
    } else {
        HiLog::Error(LABEL, "No match tag name!");
    }
}

uint32_t EXIFInfo::ModifyExifData(const ExifTag &tag, const std::string &value, const std::string &path)
{
    FILE *file = fopen(path.c_str(), "rb");
    if (file == nullptr) {
        HiLog::Error(LABEL, "Error creating file %{public}s", path.c_str());
        return Media::ERR_MEDIA_IO_ABNORMAL;
    }

    // read jpeg file to buff
    unsigned long fileLength = GetFileSize(file);
    if (fileLength == 0 || fileLength > MAX_FILE_SIZE) {
        HiLog::Error(LABEL, "Get file size failed.");
        fclose(file);
        return Media::ERR_MEDIA_BUFFER_TOO_SMALL;
    }

    unsigned char *fileBuf = static_cast<unsigned char *>(malloc(fileLength));
    if (fileBuf == nullptr) {
        HiLog::Error(LABEL, "Allocate buf for %{public}s failed.", path.c_str());
        fclose(file);
        return Media::ERR_IMAGE_MALLOC_ABNORMAL;
    }

    if (fread(fileBuf, fileLength, 1, file) != 1) {
        HiLog::Error(LABEL, "Read %{public}s failed.", path.c_str());
        ReleaseSource(&fileBuf, &file);
        return Media::ERR_MEDIA_READ_PARCEL_FAIL;
    }

    if (!(fileBuf[0] == 0xFF && fileBuf[1] == 0xD8)) {
        HiLog::Error(LABEL, "%{public}s is not jpeg file.", path.c_str());
        ReleaseSource(&fileBuf, &file);
        return Media::ERR_IMAGE_MISMATCHED_FORMAT;
    }

    ExifData *ptrExifData = nullptr;
    bool isNewExifData = false;
    if (!CreateExifData(fileBuf, fileLength, &ptrExifData, isNewExifData)) {
        ReleaseSource(&fileBuf, &file);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }
    (void)fclose(file);
    file = nullptr;

    unsigned int orginExifDataLength = GetOrginExifDataLength(isNewExifData, fileBuf);
    if (!isNewExifData && orginExifDataLength == 0) {
        HiLog::Error(LABEL, "There is no orginExifDataLength node in %{public}s.", path.c_str());
        exif_data_unref(ptrExifData);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }

    ExifByteOrder order = GetExifByteOrder(isNewExifData, fileBuf);
    FILE *newFile = fopen(path.c_str(), "wb+");
    if (newFile == nullptr) {
        HiLog::Error(LABEL, "Error create new file %{public}s", path.c_str());
        ReleaseSource(&fileBuf, &newFile);
        return Media::ERR_MEDIA_IO_ABNORMAL;
    }
    ExifEntry *entry = nullptr;
    if (!CreateExifEntry(tag, ptrExifData, value, order, &entry)) {
        ReleaseSource(&fileBuf, &newFile);
        exif_data_unref(ptrExifData);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }
    if (!WriteExifDataToFile(ptrExifData, orginExifDataLength, fileLength, fileBuf, newFile)) {
        ReleaseSource(&fileBuf, &newFile);
        exif_data_unref(ptrExifData);
        return Media::ERR_MEDIA_WRITE_PARCEL_FAIL;
    }
    ReleaseSource(&fileBuf, &newFile);
    exif_data_unref(ptrExifData);
    return Media::SUCCESS;
}

uint32_t EXIFInfo::ModifyExifData(const ExifTag &tag, const std::string &value, const int fd)
{
    const int localFd = dup(fd);
    FILE *file = fdopen(localFd, "wb+");
    if (file == nullptr) {
        HiLog::Error(LABEL, "Error creating file %{public}d", localFd);
        return Media::ERR_MEDIA_IO_ABNORMAL;
    }

    // read jpeg file to buff
    unsigned long fileLength = GetFileSize(file);
    if (fileLength == 0 || fileLength > MAX_FILE_SIZE) {
        HiLog::Error(LABEL, "Get file size failed.");
        fclose(file);
        return Media::ERR_MEDIA_BUFFER_TOO_SMALL;
    }

    unsigned char *fileBuf = static_cast<unsigned char *>(malloc(fileLength));
    if (fileBuf == nullptr) {
        HiLog::Error(LABEL, "Allocate buf for %{public}d failed.", localFd);
        fclose(file);
        return Media::ERR_IMAGE_MALLOC_ABNORMAL;
    }

    // Set current position to begin of file.
    (void)fseek(file, 0L, 0);
    if (fread(fileBuf, fileLength, 1, file) != 1) {
        HiLog::Error(LABEL, "Read %{public}d failed.", localFd);
        ReleaseSource(&fileBuf, &file);
        return Media::ERR_MEDIA_READ_PARCEL_FAIL;
    }

    if (!(fileBuf[0] == 0xFF && fileBuf[1] == 0xD8)) {
        HiLog::Error(LABEL, "%{public}d is not jpeg file.", localFd);
        ReleaseSource(&fileBuf, &file);
        return Media::ERR_IMAGE_MISMATCHED_FORMAT;
    }

    ExifData *ptrExifData = nullptr;
    bool isNewExifData = false;
    if (!CreateExifData(fileBuf, fileLength, &ptrExifData, isNewExifData)) {
        ReleaseSource(&fileBuf, &file);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }

    unsigned int orginExifDataLength = GetOrginExifDataLength(isNewExifData, fileBuf);
    if (!isNewExifData && orginExifDataLength == 0) {
        HiLog::Error(LABEL, "There is no orginExifDataLength node in %{public}d.", localFd);
        exif_data_unref(ptrExifData);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }

    ExifByteOrder order = GetExifByteOrder(isNewExifData, fileBuf);
    // Set current position to begin of new file.
    (void)fseek(file, 0L, 0);
    ExifEntry *entry = nullptr;
    if (!CreateExifEntry(tag, ptrExifData, value, order, &entry)) {
        ReleaseSource(&fileBuf, &file);
        exif_data_unref(ptrExifData);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }

    if (!WriteExifDataToFile(ptrExifData, orginExifDataLength, fileLength, fileBuf, file)) {
        ReleaseSource(&fileBuf, &file);
        exif_data_unref(ptrExifData);
        return Media::ERR_MEDIA_WRITE_PARCEL_FAIL;
    }
    ReleaseSource(&fileBuf, &file);
    exif_data_unref(ptrExifData);
    return Media::SUCCESS;
}

uint32_t EXIFInfo::ModifyExifData(const ExifTag &tag, const std::string &value,
    unsigned char *data, uint32_t size)
{
    if (data == nullptr) {
        HiLog::Error(LABEL, "buffer is nullptr.");
        return Media::ERR_IMAGE_SOURCE_DATA;
    }

    if (size == 0) {
        HiLog::Error(LABEL, "buffer size is 0.");
        return Media::ERR_MEDIA_BUFFER_TOO_SMALL;
    }

    if (!(data[0] == 0xFF && data[1] == 0xD8)) {
        HiLog::Error(LABEL, "This is not jpeg file.");
        return Media::ERR_IMAGE_MISMATCHED_FORMAT;
    }

    ExifData *ptrExifData = nullptr;
    bool isNewExifData = false;
    if (!CreateExifData(data, size, &ptrExifData, isNewExifData)) {
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }

    unsigned int orginExifDataLength = GetOrginExifDataLength(isNewExifData, data);
    if (!isNewExifData && orginExifDataLength == 0) {
        HiLog::Error(LABEL, "There is no orginExifDataLength node in buffer.");
        exif_data_unref(ptrExifData);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }

    ExifByteOrder order = GetExifByteOrder(isNewExifData, data);
    ExifEntry *entry = nullptr;
    if (!CreateExifEntry(tag, ptrExifData, value, order, &entry)) {
        exif_data_unref(ptrExifData);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }

    unsigned char* exifDataBuf = nullptr;
    unsigned int exifDataBufLength = 0;
    exif_data_save_data(ptrExifData, &exifDataBuf, &exifDataBufLength);
    if (exifDataBuf == nullptr) {
        HiLog::Error(LABEL, "Get Exif Data Buf failed!");
        exif_data_unref(ptrExifData);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }

    if (size == 0 || size > MAX_FILE_SIZE) {
        HiLog::Error(LABEL, "Buffer size is out of range.");
        exif_data_unref(ptrExifData);
        return Media::ERR_IMAGE_DECODE_EXIF_UNSUPPORT;
    }
    unsigned char *tempBuf = static_cast<unsigned char *>(malloc(size));
    if (tempBuf == nullptr) {
        HiLog::Error(LABEL, "Allocate temp buffer ailed.");
        exif_data_unref(ptrExifData);
        return Media::ERR_IMAGE_MALLOC_ABNORMAL;
    }

    // Write EXIF header to buffer
    uint32_t index = 0;
    if (sizeof(exifHeader) >= size) {
        HiLog::Error(LABEL, "There is not enough space for EXIF header!");
        free(tempBuf);
        tempBuf = nullptr;
        exif_data_unref(ptrExifData);
        return Media::ERR_MEDIA_OUT_OF_RANGE;
    }

    for (size_t i = 0; i < sizeof(exifHeader); i++) {
        tempBuf[index] = exifHeader[i];
        index += 1;
    }

    // Write EXIF block length in big-endian order
    unsigned char highBit = static_cast<unsigned char>((exifDataBufLength + LENGTH_OFFSET_2) >> MOVE_OFFSET_8);
    if (index >= size) {
        HiLog::Error(LABEL, "There is not enough space for writing EXIF block length!");
        free(tempBuf);
        tempBuf = nullptr;
        exif_data_unref(ptrExifData);
        return Media::ERR_MEDIA_OUT_OF_RANGE;
    }
    tempBuf[index] = highBit;
    index += 1;

    unsigned char lowBit = static_cast<unsigned char>((exifDataBufLength + LENGTH_OFFSET_2) & 0xff);
    if (index >= size) {
        HiLog::Error(LABEL, "There is not enough space for writing EXIF block length!");
        free(tempBuf);
        tempBuf = nullptr;
        exif_data_unref(ptrExifData);
        return Media::ERR_MEDIA_OUT_OF_RANGE;
    }
    tempBuf[index] = lowBit;
    index += 1;

    // Write EXIF data block
    if ((index +  exifDataBufLength) >= size) {
        HiLog::Error(LABEL, "There is not enough space for writing EXIF data block!");
        free(tempBuf);
        tempBuf = nullptr;
        exif_data_unref(ptrExifData);
        return Media::ERR_MEDIA_OUT_OF_RANGE;
    }
    for (unsigned int i = 0; i < exifDataBufLength; i++) {
        tempBuf[index] = exifDataBuf[i];
        index += 1;
    }

    // Write JPEG image data, skipping the non-EXIF header
    if ((index + size - orginExifDataLength - sizeof(exifHeader)) > size) {
        HiLog::Error(LABEL, "There is not enough space for writing JPEG image data!");
        free(tempBuf);
        tempBuf = nullptr;
        exif_data_unref(ptrExifData);
        return Media::ERR_MEDIA_OUT_OF_RANGE;
    }
    for (unsigned int i = 0; i < (size - orginExifDataLength - sizeof(exifHeader)); i++) {
        tempBuf[index] = data[orginExifDataLength + sizeof(exifHeader) + i];
        index += 1;
    }

    for (unsigned int i = 0; i < size; i++) {
        data[i] = tempBuf[i];
    }

    ParseExifData(data, static_cast<unsigned int>(index));
    free(tempBuf);
    tempBuf = nullptr;
    exif_data_unref(ptrExifData);
    return Media::SUCCESS;
}

ExifEntry* EXIFInfo::InitExifTag(ExifData *exif, ExifIfd ifd, ExifTag tag)
{
    ExifEntry *entry;
    /* Return an existing tag if one exists */
    if (!(entry = exif_content_get_entry(exif->ifd[ifd], tag))) {
        /* Allocate a new entry */
        entry = exif_entry_new();
        if (entry == nullptr) {
            HiLog::Error(LABEL, "Create new entry failed!");
            return nullptr;
        }
        entry->tag = tag; // tag must be set before calling exif_content_add_entry
        /* Attach the ExifEntry to an IFD */
        exif_content_add_entry (exif->ifd[ifd], entry);

        /* Allocate memory for the entry and fill with default data */
        exif_entry_initialize (entry, tag);

        /* Ownership of the ExifEntry has now been passed to the IFD.
         * One must be very careful in accessing a structure after
         * unref'ing it; in this case, we know "entry" won't be freed
         * because the reference count was bumped when it was added to
         * the IFD.
         */
        exif_entry_unref(entry);
    }
    return entry;
}

ExifEntry* EXIFInfo::CreateExifTag(ExifData *exif, ExifIfd ifd, ExifTag tag,
    size_t len, ExifFormat format)
{
    void *buf;
    ExifEntry *entry;

    if ((entry = exif_content_get_entry(exif->ifd[ifd], tag)) != nullptr) {
        return entry;
    }

    /* Create a memory allocator to manage this ExifEntry */
    ExifMem *mem = exif_mem_new_default();
    if (mem == nullptr) {
        HiLog::Error(LABEL, "Create mem failed!");
        return nullptr;
    }

    /* Create a new ExifEntry using our allocator */
    entry = exif_entry_new_mem (mem);
    if (entry == nullptr) {
        HiLog::Error(LABEL, "Create entry by mem failed!");
        return nullptr;
    }

    /* Allocate memory to use for holding the tag data */
    buf = exif_mem_alloc(mem, len);
    if (buf == nullptr) {
        HiLog::Error(LABEL, "Allocate memory failed!");
        return nullptr;
    }

    /* Fill in the entry */
    entry->data = static_cast<unsigned char*>(buf);
    entry->size = len;
    entry->tag = tag;
    entry->components = len;
    entry->format = format;

    /* Attach the ExifEntry to an IFD */
    exif_content_add_entry (exif->ifd[ifd], entry);

    /* The ExifMem and ExifEntry are now owned elsewhere */
    exif_mem_unref(mem);
    exif_entry_unref(entry);

    return entry;
}

unsigned long EXIFInfo::GetFileSize(FILE *fp)
{
    long int position;
    long size;

    /* Save the current position. */
    position = ftell(fp);

    /* Jump to the end of the file. */
    (void)fseek(fp, 0L, SEEK_END);

    /* Get the end position. */
    size = ftell(fp);

    /* Jump back to the original position. */
    (void)fseek(fp, position, SEEK_SET);

    return static_cast<unsigned long>(size);
}

bool EXIFInfo::CreateExifData(unsigned char *buf, unsigned long length, ExifData **ptrData, bool &isNewExifData)
{
    if ((buf[BUFFER_POSITION_6] == 'E' && buf[BUFFER_POSITION_7] == 'x' &&
        buf[BUFFER_POSITION_8] == 'i' && buf[BUFFER_POSITION_9] == 'f')) {
        *ptrData = exif_data_new_from_data(buf, static_cast<unsigned int>(length));
        if (!(*ptrData)) {
            HiLog::Error(LABEL, "Create exif data from file failed.");
            return false;
        }
        isNewExifData = false;
        HiLog::Error(LABEL, "Create exif data from buffer.");
    } else {
        *ptrData = exif_data_new();
        if (!(*ptrData)) {
            HiLog::Error(LABEL, "Create exif data failed.");
            return false;
        }
        /* Set the image options */
        exif_data_set_option(*ptrData, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
        exif_data_set_data_type(*ptrData, EXIF_DATA_TYPE_COMPRESSED);
        exif_data_set_byte_order(*ptrData, EXIF_BYTE_ORDER_INTEL);

        /* Create the mandatory EXIF fields with default data */
        exif_data_fix(*ptrData);
        isNewExifData = true;
        HiLog::Error(LABEL, "Create new exif data.");
    }
    return true;
}

unsigned int EXIFInfo::GetOrginExifDataLength(const bool &isNewExifData, unsigned char *buf)
{
    unsigned int orginExifDataLength = 0;
    if (!isNewExifData) {
        orginExifDataLength = static_cast<unsigned int>(buf[BUFFER_POSITION_5]) |
            static_cast<unsigned int>(buf[BUFFER_POSITION_4] << MOVE_OFFSET_8);
    }
    return orginExifDataLength;
}

ExifByteOrder EXIFInfo::GetExifByteOrder(const bool &isNewExifData, unsigned char *buf)
{
    if (isNewExifData) {
        return EXIF_BYTE_ORDER_INTEL;
    } else {
        if (buf[BUFFER_POSITION_12] == 'M' && buf[BUFFER_POSITION_13] == 'M') {
            return EXIF_BYTE_ORDER_MOTOROLA;
        } else {
            return EXIF_BYTE_ORDER_INTEL;
        }
    }
}

bool EXIFInfo::CreateExifEntry(const ExifTag &tag, ExifData *data, const std::string &value,
    ExifByteOrder order, ExifEntry **ptrEntry)
{
    switch (tag) {
        case EXIF_TAG_BITS_PER_SAMPLE: {
            *ptrEntry = InitExifTag(data, EXIF_IFD_1, EXIF_TAG_BITS_PER_SAMPLE);
            if ((*ptrEntry) == nullptr) {
                HiLog::Error(LABEL, "Get exif entry failed.");
                return false;
            }
            std::vector<std::string> bitsVec;
            SplitStr(value, ",", bitsVec);
            if (bitsVec.size() > CONSTANT_2) {
                HiLog::Error(LABEL, "BITS_PER_SAMPLE Invalid value %{public}s", value.c_str());
                return false;
            }
            if (bitsVec.size() != 0) {
                for (size_t i = 0; i < bitsVec.size(); i++) {
                    exif_set_short((*ptrEntry)->data + i * CONSTANT_2, order, (ExifShort)atoi(bitsVec[i].c_str()));
                }
            }
            break;
        }
        case EXIF_TAG_ORIENTATION: {
            *ptrEntry = InitExifTag(data, EXIF_IFD_0, EXIF_TAG_ORIENTATION);
            if ((*ptrEntry) == nullptr) {
                HiLog::Error(LABEL, "Get exif entry failed.");
                return false;
            }
            exif_set_short((*ptrEntry)->data, order, (ExifShort)atoi(value.c_str()));
            break;
        }
        case EXIF_TAG_IMAGE_LENGTH: {
            *ptrEntry = InitExifTag(data, EXIF_IFD_1, EXIF_TAG_IMAGE_LENGTH);
            if ((*ptrEntry) == nullptr) {
                HiLog::Error(LABEL, "Get exif entry failed.");
                return false;
            }
            exif_set_long((*ptrEntry)->data, order, (ExifLong)atoi(value.c_str()));
            break;
        }
        case EXIF_TAG_IMAGE_WIDTH: {
            *ptrEntry = InitExifTag(data, EXIF_IFD_1, EXIF_TAG_IMAGE_WIDTH);
            if ((*ptrEntry) == nullptr) {
                HiLog::Error(LABEL, "Get exif entry failed.");
                return false;
            }
            exif_set_long((*ptrEntry)->data, order, (ExifLong)atoi(value.c_str()));
            break;
        }
        case EXIF_TAG_GPS_LATITUDE: {
            std::vector<std::string> latVec;
            SplitStr(value, ",", latVec);
            if (latVec.size() != CONSTANT_2) {
                HiLog::Error(LABEL, "GPS_LATITUDE Invalid value %{public}s", value.c_str());
                return false;
            }

            ExifRational latRational;
            latRational.numerator = static_cast<ExifSLong>(atoi(latVec[0].c_str()));
            latRational.denominator = static_cast<ExifSLong>(atoi(latVec[1].c_str()));
            *ptrEntry = CreateExifTag(data, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE,
                sizeof(latRational), EXIF_FORMAT_RATIONAL);
            if ((*ptrEntry) == nullptr) {
                HiLog::Error(LABEL, "Get exif entry failed.");
                return false;
            }
            exif_set_rational((*ptrEntry)->data, order, latRational);
            break;
        }
        case EXIF_TAG_GPS_LONGITUDE: {
            std::vector<std::string> longVec;
            SplitStr(value, ",", longVec);
            if (longVec.size() != CONSTANT_2) {
                HiLog::Error(LABEL, "GPS_LONGITUDE Invalid value %{public}s", value.c_str());
                return false;
            }

            ExifRational longRational;
            longRational.numerator = static_cast<ExifSLong>(atoi(longVec[0].c_str()));
            longRational.denominator = static_cast<ExifSLong>(atoi(longVec[1].c_str()));
            *ptrEntry = CreateExifTag(data, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE,
                sizeof(longRational), EXIF_FORMAT_RATIONAL);
            if ((*ptrEntry) == nullptr) {
                HiLog::Error(LABEL, "Get exif entry failed.");
                return false;
            }
            exif_set_rational((*ptrEntry)->data, order, longRational);
            break;
        }
        case EXIF_TAG_GPS_LATITUDE_REF: {
            *ptrEntry = CreateExifTag(data, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE_REF,
                value.length(), EXIF_FORMAT_ASCII);
            if ((*ptrEntry) == nullptr) {
                HiLog::Error(LABEL, "Get exif entry failed.");
                return false;
            }
            if (memcpy_s((*ptrEntry)->data, value.length(), value.c_str(), value.length()) != 0) {
                HiLog::Error(LABEL, "LATITUDE ref memcpy error");
            }
            break;
        }
        case EXIF_TAG_GPS_LONGITUDE_REF: {
            *ptrEntry = CreateExifTag(data, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE_REF,
                value.length(), EXIF_FORMAT_ASCII);
            if ((*ptrEntry) == nullptr) {
                HiLog::Error(LABEL, "Get exif entry failed.");
                return false;
            }
            if (memcpy_s((*ptrEntry)->data, value.length(), value.c_str(), value.length()) != 0) {
                HiLog::Error(LABEL, "LONGITUDE ref memcpy error");
            }
            break;
        }
        default:
            break;
    }
    return true;
}

bool EXIFInfo::WriteExifDataToFile(ExifData *data, unsigned int orginExifDataLength, unsigned long fileLength,
    unsigned char *buf, FILE *fp)
{
    unsigned char* exifDataBuf = nullptr;
    unsigned int exifDataBufLength = 0;
    exif_data_save_data(data, &exifDataBuf, &exifDataBufLength);
    if (exifDataBuf == nullptr) {
        HiLog::Error(LABEL, "Get Exif Data Buf failed!");
        return false;
    }

    // Write EXIF header
    if (fwrite(exifHeader, sizeof(exifHeader), 1, fp) != 1) {
        HiLog::Error(LABEL, "Error writing EXIF header to file!");
        return false;
    }

    // Write EXIF block length in big-endian order
    if (fputc((exifDataBufLength + LENGTH_OFFSET_2) >> MOVE_OFFSET_8, fp) < 0) {
        HiLog::Error(LABEL, "Error writing EXIF block length to file!");
        return false;
    }

    if (fputc((exifDataBufLength + LENGTH_OFFSET_2) & 0xff, fp) < 0) {
        HiLog::Error(LABEL, "Error writing EXIF block length to file!");
        return false;
    }

    // Write EXIF data block
    if (fwrite(exifDataBuf, exifDataBufLength, 1, fp) != 1) {
        HiLog::Error(LABEL, "Error writing EXIF data block to file!");
        return false;
    }
    // Write JPEG image data, skipping the non-EXIF header
    unsigned int dataOffset = orginExifDataLength + sizeof(exifHeader);
    if (fwrite(buf + dataOffset, fileLength - dataOffset, 1, fp) != 1) {
        HiLog::Error(LABEL, "Error writing JPEG image data to file!");
        return false;
    }

    UpdateCacheExifData(fp);
    return true;
}

void EXIFInfo::ReleaseSource(unsigned char **ptrBuf, FILE **ptrFile)
{
    if (*ptrBuf) {
        free(*ptrBuf);
        *ptrBuf = nullptr;
        ptrBuf = nullptr;
    }

    if (*ptrFile != nullptr) {
        fclose(*ptrFile);
        *ptrFile = nullptr;
        ptrFile = nullptr;
    }
}

void EXIFInfo::UpdateCacheExifData(FILE *fp)
{
    unsigned long fileLength = GetFileSize(fp);
    if (fileLength == 0 || fileLength > MAX_FILE_SIZE) {
        HiLog::Error(LABEL, "Get file size failed.");
        return;
    }

    unsigned char *fileBuf = static_cast<unsigned char *>(malloc(fileLength));
    if (fileBuf == nullptr) {
        HiLog::Error(LABEL, "Allocate buf failed.");
        return;
    }

    // Set current position to begin of file.
    (void)fseek(fp, 0L, 0);
    if (fread(fileBuf, fileLength, 1, fp) != 1) {
        HiLog::Error(LABEL, "Read new file failed.");
        free(fileBuf);
        fileBuf = nullptr;
        return;
    }

    ParseExifData(fileBuf, static_cast<unsigned int>(fileLength));
    free(fileBuf);
    fileBuf = nullptr;
}
} // namespace ImagePlugin
} // namespace OHOS
