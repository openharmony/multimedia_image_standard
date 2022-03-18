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
    static const int LENGTH_ARRAY_SIZE = 2;
    static const int CONSTANT_2 = 2;

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
    } else {
        HiLog::Error(LABEL, "No match tag name!");
    }
}

bool EXIFInfo::ModifyExifData(const ExifTag &tag, const std::string &value, const std::string &path)
{
    FILE *file = fopen(path.c_str(), "rb");
    if (file == nullptr) {
        HiLog::Error(LABEL, "Error creating file %{public}s", path.c_str());
        return false;
    }

    // read jpeg file to buff
    unsigned long fileLength = GetFileSize(file);
    if (fileLength == 0) {
        HiLog::Error(LABEL, "Get file size failed.");
        fclose(file);
        return false;
    }
    unsigned char *fileBuf = (unsigned char *)malloc(fileLength);
    if (fileBuf == nullptr) {
        HiLog::Error(LABEL, "Allocate buf for %{public}s failed.", path.c_str());
        fclose(file);
        return false;
    }

    if (fread(fileBuf, fileLength, 1, file) != 1) {
        HiLog::Error(LABEL, "Read %{public}s failed.", path.c_str());
        ReleaseSource(fileBuf, file);
        return false;
    }

    if (!(fileBuf[0] == 0xFF && fileBuf[1] == 0xD8)) {
        HiLog::Error(LABEL, "%{public}s is not jpeg file.", path.c_str());
        ReleaseSource(fileBuf, file);
        return false;
    }

    unsigned char lenthArray[LENGTH_ARRAY_SIZE] = {
        fileBuf[BUFFER_POSITION_5], fileBuf[BUFFER_POSITION_4]
    };
    unsigned int orginExifDataLength = *(unsigned int*)lenthArray;

    ExifData *ptrExifData = nullptr;
    if ((fileBuf[BUFFER_POSITION_6] == 'E' && fileBuf[BUFFER_POSITION_7] == 'x' &&
        fileBuf[BUFFER_POSITION_8] == 'i' && fileBuf[BUFFER_POSITION_9] == 'f')) {
        ptrExifData = exif_data_new_from_file(path.c_str());
        if (!ptrExifData) {
            HiLog::Error(LABEL, "Create exif data from file failed.");
            ReleaseSource(fileBuf, file);
            return false;
        }
    } else {
        ptrExifData = exif_data_new();
        if (!ptrExifData) {
            HiLog::Error(LABEL, "Create exif data failed.");
            ReleaseSource(fileBuf, file);
            return false;
        }
        /* Set the image options */
        exif_data_set_option(ptrExifData, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
        exif_data_set_data_type(ptrExifData, EXIF_DATA_TYPE_COMPRESSED);
        exif_data_set_byte_order(ptrExifData, EXIF_BYTE_ORDER_INTEL);

        /* Create the mandatory EXIF fields with default data */
        exif_data_fix(ptrExifData);
    }
    (void)fclose(file);
    file = nullptr;

    ExifByteOrder order = EXIF_BYTE_ORDER_MOTOROLA;
    if (fileBuf[BUFFER_POSITION_12] == 'M' && fileBuf[BUFFER_POSITION_13] == 'M') {
        order = EXIF_BYTE_ORDER_MOTOROLA;
    } else {
        order = EXIF_BYTE_ORDER_INTEL;
    }

    FILE *newFile = fopen(path.c_str(), "wb");
    if (newFile == nullptr) {
        HiLog::Error(LABEL, "Error create new file %{public}s", path.c_str());
        ReleaseSource(fileBuf, newFile);
        return false;
    }

    ExifEntry *entry = nullptr;
    switch (tag) {
        case EXIF_TAG_BITS_PER_SAMPLE: {
            entry = InitExifTag(ptrExifData, EXIF_IFD_1, EXIF_TAG_BITS_PER_SAMPLE);
            std::vector<std::string> bitsVec;
            SplitStr(value, ",", bitsVec);
            if (bitsVec.size() > CONSTANT_2) {
                HiLog::Error(LABEL, "BITS_PER_SAMPLE Invalid value %{public}s", value.c_str());
                ReleaseSource(fileBuf, newFile);
                return false;
            }
            if (entry == nullptr) {
                return false;
            }
            if (bitsVec.size() != 0) {
                for (size_t i = 0; i < bitsVec.size(); i++) {
                    exif_set_short(entry->data + i * CONSTANT_2, order, (ExifShort)atoi(bitsVec[i].c_str()));
                }
            }
            break;
        }
        case EXIF_TAG_ORIENTATION: {
            entry = InitExifTag(ptrExifData, EXIF_IFD_0, EXIF_TAG_ORIENTATION);
            if (entry == nullptr) {
                return false;
            }
            exif_set_short(entry->data, order, (ExifShort)atoi(value.c_str()));
            break;
        }
        case EXIF_TAG_IMAGE_LENGTH: {
            entry = InitExifTag(ptrExifData, EXIF_IFD_1, EXIF_TAG_IMAGE_LENGTH);
            if (entry == nullptr) {
                return false;
            }
            exif_set_short(entry->data, order, (ExifShort)atoi(value.c_str()));
            break;
        }
        case EXIF_TAG_IMAGE_WIDTH: {
            entry = InitExifTag(ptrExifData, EXIF_IFD_1, EXIF_TAG_IMAGE_WIDTH);
            if (entry == nullptr) {
                return false;
            }
            exif_set_short(entry->data, order, (ExifShort)atoi(value.c_str()));
            break;
        }
        case EXIF_TAG_GPS_LATITUDE: {
            std::vector<std::string> latVec;
            SplitStr(value, ",", latVec);
            if (latVec.size() != CONSTANT_2) {
                HiLog::Error(LABEL, "GPS_LATITUDE Invalid value %{public}s", value.c_str());
                ReleaseSource(fileBuf, newFile);
                return false;
            }

            ExifRational latRational;
            latRational.numerator = atoi(latVec[0].c_str());
            latRational.denominator = atoi(latVec[1].c_str());
            entry = CreateExifTag(ptrExifData, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE,
                sizeof(latRational), EXIF_FORMAT_RATIONAL);
            if (entry == nullptr) {
                return false;
            }
            exif_set_rational(entry->data, order, latRational);
            break;
        }
        case EXIF_TAG_GPS_LONGITUDE: {
            std::vector<std::string> longVec;
            SplitStr(value, ",", longVec);
            if (longVec.size() != CONSTANT_2) {
                HiLog::Error(LABEL, "GPS_LONGITUDE Invalid value %{public}s", value.c_str());
                ReleaseSource(fileBuf, newFile);
                return false;
            }

            ExifRational longRational;
            longRational.numerator = atoi(longVec[0].c_str());
            longRational.denominator = atoi(longVec[1].c_str());
            entry = CreateExifTag(ptrExifData, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE,
                sizeof(longRational), EXIF_FORMAT_RATIONAL);
            if (entry == nullptr) {
                return false;
            }
            exif_set_rational(entry->data, order, longRational);
            break;
        }
        case EXIF_TAG_GPS_LATITUDE_REF: {
            entry = CreateExifTag(ptrExifData, EXIF_IFD_GPS, EXIF_TAG_GPS_LATITUDE_REF,
                value.length(), EXIF_FORMAT_ASCII);
            if (entry == nullptr) {
                return false;
            }
            if (memcpy_s(entry->data, value.length(), value.c_str(), value.length()) != 0) {
                HiLog::Error(LABEL, "LATITUDE ref memcpy error");
            }
            break;
        }
        case EXIF_TAG_GPS_LONGITUDE_REF: {
            entry = CreateExifTag(ptrExifData, EXIF_IFD_GPS, EXIF_TAG_GPS_LONGITUDE_REF,
                value.length(), EXIF_FORMAT_ASCII);
            if (entry == nullptr) {
                return false;
            }
            if (memcpy_s(entry->data, value.length(), value.c_str(), value.length()) != 0) {
                HiLog::Error(LABEL, "LONGITUDE ref memcpy error");
            }
            break;
        }
        default:
            break;
    }

    unsigned char* exifDataBuf = nullptr;
    unsigned int exifDataBufLength = 0;
    exif_data_save_data(ptrExifData, &exifDataBuf, &exifDataBufLength);
    if (exifDataBuf == nullptr) {
        HiLog::Error(LABEL, "Get Exif Data Buf failed!");
        return false;
    }

    /* Write EXIF header */
    if (fwrite(exifHeader, sizeof(exifHeader), 1, newFile) != 1) {
        HiLog::Error(LABEL, "Error writing EXIF header to file!");
        ReleaseSource(fileBuf, newFile);
        return false;
    }

    /* Write EXIF block length in big-endian order */
    if (fputc((exifDataBufLength + LENGTH_OFFSET_2) >> MOVE_OFFSET_8, newFile) < 0) {
        HiLog::Error(LABEL, "Error writing EXIF block length to file!");
        ReleaseSource(fileBuf, newFile);
        return false;
    }
    if (fputc((exifDataBufLength + LENGTH_OFFSET_2) & 0xff, newFile) < 0) {
        HiLog::Error(LABEL, "Error writing EXIF block length to file!");
        ReleaseSource(fileBuf, newFile);
        return false;
    }

    /* Write EXIF data block */
    if (fwrite(exifDataBuf, exifDataBufLength, 1, newFile) != 1) {
        HiLog::Error(LABEL, "Error writing EXIF data block to file!");
        ReleaseSource(fileBuf, newFile);
        return false;
    }
    /* Write JPEG image data, skipping the non-EXIF header */
    unsigned int dataOffset = orginExifDataLength + sizeof(exifHeader);
    if (fwrite(fileBuf + dataOffset, fileLength - dataOffset, 1, newFile) != 1) {
        HiLog::Error(LABEL, "Error writing JPEG image data to file!");
        ReleaseSource(fileBuf, newFile);
        return false;
    }

    ReleaseSource(fileBuf, newFile);
    return true;
}

ExifIfd EXIFInfo::GetImageFileDirectory(const ExifTag &tag)
{
    switch (tag) {
        case EXIF_TAG_BITS_PER_SAMPLE:
        case EXIF_TAG_ORIENTATION:
        case EXIF_TAG_IMAGE_LENGTH:
        case EXIF_TAG_IMAGE_WIDTH: {
            return EXIF_IFD_0;
        }
        case EXIF_TAG_DATE_TIME_ORIGINAL: {
            return EXIF_IFD_EXIF;
        }
        case EXIF_TAG_GPS_LATITUDE:
        case EXIF_TAG_GPS_LONGITUDE:
        case EXIF_TAG_GPS_LATITUDE_REF:
        case EXIF_TAG_GPS_LONGITUDE_REF: {
            return EXIF_IFD_GPS;
        }
        default:
            break;
    }
    return EXIF_IFD_COUNT;
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
        buf = nullptr;
    }

    if (file != nullptr) {
        fclose(file);
        file = nullptr;
    }
}
} // namespace ImagePlugin
} // namespace OHOS
