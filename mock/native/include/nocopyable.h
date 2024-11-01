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

#ifndef MOCK_NATIVE_INCLUDE_NOCOPYABLE_H_
#define MOCK_NATIVE_INCLUDE_NOCOPYABLE_H_
namespace OHOS {
#define DISALLOW_COPY_AND_MOVE(className) \
do \
{ \
    DISALLOW_COPY(className); \
    DISALLOW_MOVE(className); \
} while (0)

#define DISALLOW_COPY(className) \
do \
{ \
    className(const className&) = delete; \
    (className)& operator= (const className&) = delete; \
} while (0)

#define DISALLOW_MOVE(className) \
do \
{ \
    className(className&&) = delete; \
    (className)& operator= ((className)&&) = delete; \
} while (0)
class NoCopyable {
protected:
    NoCopyable() {}
    virtual ~NoCopyable() {}

private:
    DISALLOW_COPY_AND_MOVE(NoCopyable);
};
} // namespace OHOS
#endif  // MOCK_NATIVE_INCLUDE_NOCOPYABLE_H_
