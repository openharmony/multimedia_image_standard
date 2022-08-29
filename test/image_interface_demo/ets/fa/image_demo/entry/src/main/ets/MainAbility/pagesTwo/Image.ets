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

// @ts-nocheck
import router from '@ohos.router';
import multimedia_image from '@ohos.multimedia.image';
import resourceManager from '@ohos.resourceManager';
@Entry
@Component
struct Index1 {
  @State imgSource: multimedia_image.ImageSource = undefined
  @State _pixelMap: multimedia_image.PixelMap = undefined


  async pxiel1() {
    console.info('1111111 createPixelBYBuffer begin');
    var color = new ArrayBuffer(96);
    var bufferArr = new Uint8Array(color);
    for (var i = 0; i < bufferArr.length; i++) {
      bufferArr[i] = i + 100;
    }
    let opts = { editable: true, pixelFormat: 3, size: { height: 4, width: 6 } }

//    通过属性创建PixelMap，默认采用BGRA_8888格式处理数据，通过Promise返回结果。
    multimedia_image.createPixelMap(color, opts).then((pixelMap)=>{
      this._pixelMap = pixelMap;
    })
  }

  async pxiel2() {
    console.info('222222 createPixelBYBuffer begin');
    var color = new ArrayBuffer(96);
    var bufferArr = new Uint8Array(color);
    for (var i = 0; i < bufferArr.length; i++) {
      bufferArr[i] = i + 100;
    }
    let opts = { editable: true, pixelFormat: 3, size: { height: 4, width: 6 } }

//    通过属性创建PixelMap，默认采用BGRA_8888格式处理数据，通过callback返回结果。
    multimedia_image.createPixelMap(color, opts, (error, pixelmap) => {
      if(error) {
        console.log('222222 Failed to create pixelmap.');
      } else {
        this._pixelMap = pixelMap;
        console.log('222222 Succeeded in creating pixelmap.');
      }
    })
  }





  build() {
    Row() {
      Column() {
        Button('提交').onClick(() => {
          this.pxiel1()
        }).height('10%')
        .width('15%')
        .margin(20)

        Button('提交').onClick(() => {
          this.pxiel2()
        }).height('10%')
          .width('15%')
          .margin(20)

//        Button('imageS').onClick(() => {
//          this.imageS()
//        }).height('10%')
//          .width('15%')
//        .margin(20)

        Flex({ justifyContent: FlexAlign.SpaceAround }) {
          Image(this._pixelMap)
            .border({ width: 1, color: Color.Blue })
            .width(50).height(50)
            .objectFit(ImageFit.Cover)
//            .backgroundColor(Color.Red)
        }
      }
      .width('100%')
    }
    .height('100%')
  }
}