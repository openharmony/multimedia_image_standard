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

import router from '@ohos.router';
import multimedia_image from '@ohos.multimedia.image';
import resourceManager from '@ohos.resourceManager';

@Entry
@Component
struct imagePacker {

  @State imgSource: multimedia_image.ImageSource = undefined
  @State _pixelMap: multimedia_image.PixelMap = undefined
  @State imagePackerApi: multimedia_image.ImagePacker = undefined
  @State error: object = undefined

  async getFd(fileName) {
    let mgr = await resourceManager.getResourceManager()
    let value = await mgr.getRawFileDescriptor(fileName)
    return value.fd
  }

  async aboutToAppear() {
    let fd = await this.getFd('test_exif.jpg')
    this.imgSource = multimedia_image.createImageSource(fd)
  }

  async btn() {
    let decodingOptions = {
      editable: true,
      desiredPixelFormat: 3
    }
    this.imgSource.createPixelMap(decodingOptions).then(async (pixelMap) => {
      this._pixelMap = pixelMap
    }).catch(error => {
      this.error = error
      console.log('q_yP1 pixelMap error' + error);
    })
  }

  async pac() {
    console.info("q_yP start")
    this.imagePackerApi = multimedia_image.createImagePacker();
    console.info("q_yP log1")
    let packOpts = { format:"image/jpeg", quality:98 };
    console.info("q_yP log2")

//    图片压缩或重新打包，使用callback形式返回结果。 failed
//    this.imagePackerApi.packing(this.imgSource, packOpts, data => {
//      console.info("q_yP2 The packed buffer is :" + data)
//    })
//    console.info("q_yP log3")

//    图片压缩或重新打包，使用Promise形式返回结果 success
    this.imagePackerApi.packing(this.imgSource, packOpts)
      .then( data => {
        console.info("q_yP3 The packed buffer is :" + data)
      }).catch(error => {
      console.info('q_yP4 packing failed.');
    })
    console.info("q_yP log4")

//    图片压缩或重新打包，使用callback形式返回结果。 failed
//    this.imagePackerApi.packing(this._pixelMap, packOpts, data => {
//      console.info("q_yP5 The packed buffer is :" + data)
//    })
//    console.info("q_yP log5")

//    const Color = new ArrayBuffer(96);
//    var opts = { editable: true, pixelFormat: 2, size: { height: 4, width: 6 } }
//    multimedia_image.createPixelMap(Color, opts)
//      .then(pixelmap => {
//        if (pixelmap == undefined) {
//          console.info('q_yP create pixelmap error.');
//        } else {
//            this.imagePackerApi.packing(pixelmap, packOpts, (err, data) => {
//              console.info("q_yP3 The packed buffer is :" + data)
//            })
//        }
//      })

//    图片压缩或重新打包，使用Promise形式返回结果 success
//    this.imagePackerApi.packing(this._pixelMap, packOpts)
//      .then( data => {
//        console.info("q_yP6 The packed buffer is :" + data)
//      }).catch(error => {
//      console.error('q_yP7 Failed to pack the image..');
//    })
    console.info("q_yP log6")

  }

  build() {
    Row() {
      Column() {
        Button('提交').onClick(() => {
          this.btn()
        })
        Button('packer').onClick(() => {
          this.pac()
        })
        Flex({ justifyContent: FlexAlign.SpaceAround }) {
          Image(this._pixelMap).border({ width: 1, color: Color.Blue }).width(500).height(500).objectFit(ImageFit.None).backgroundColor(Color.Red)
        }

        Button() {
          Text('Back')
            .fontSize(25)
            .fontWeight(FontWeight.Bold)
        }
        .type(ButtonType.Capsule)
        .margin({
          top: 20
        })
        .backgroundColor('#0D9FFB')
        .width('40%')
        .height('5%')
        // 返回按钮绑定onClick事件，点击按钮时返回到第一页
        .onClick(() => {
          router.back()
        })
      }
      .width('100%')
    }
    .height('100%')
  }
}