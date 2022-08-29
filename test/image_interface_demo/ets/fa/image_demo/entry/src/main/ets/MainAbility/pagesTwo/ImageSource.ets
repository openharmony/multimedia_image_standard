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
import multimedia_image from '@ohos.multimedia.image';
import resourceManager from '@ohos.resourceManager';
import fileio from '@ohos.fileio';
import featureAbility from '@ohos.ability.featureAbility';

@Entry
@Component
struct Index1 {
  @State imgSource: multimedia_image.ImageSource = undefined
  @State _pixelMap: multimedia_image.PixelMap = undefined
  @State op: number = 1

  async getFd(fileName) {
    console.info('q_yFd3 in')
    let mgr = await resourceManager.getResourceManager()
    let value = await mgr.getRawFileDescriptor(fileName)
    console.info('q_yFd4 out')
    return value.fd
  }

  async aboutToAppear() {
    console.info('q_yFd1 in')
//  1.从沙箱路径读取图片
//    将图片推入沙箱路径(/data/app/el2/100/base/com.example.myapplication/haps/entry/cache)中
//    修改权限777后读取
    let context = featureAbility.getContext()
    let data = await context.getCacheDir()
    console.info("q_y data = " + data)
    let fd = fileio.openSync(data + '/test_exif.jpg', 0o2)
    console.info('q_yFd1 in fd = ' + fd)

//    // 2.从本地rawfile读取图片
//    let fd = await this.getFd('test_exif.jpg')

    this.imgSource = multimedia_image.createImageSource(fd)
    if (this.imgSource == undefined) {
      console.info('q_y create imageSource error')
    }
    console.info('q_yFd2 out')
  }

  async btn() {
    console.info('q_yFd2 out')
    let decodingOptions = {
      editable: true,
      desiredPixelFormat: 3
    }
    this.imgSource.createPixelMap(decodingOptions).then(async (pixelMap) => {
      if (pixelMap == undefined) {
        console.info('q_yP createPixelMap failed')
      }
      this._pixelMap = pixelMap
    }).catch(error => {
      this.error = error
      console.log('pixelMap error' + error)
    })
  }

  async imageS() {

//    modifyImageProperty(key: string, value: string): Promise<void>
//    通过指定的键修改图片属性的值，使用Promise形式返回结果。  需要写入权限
//    参数名：key	类型：string	 必填
//    参数名：value   类型string   必填
//    key参数说明：图片属性名。
//    value参数说明：属性值。
//    测试结果：success
    this.imgSource.modifyImageProperty("Orientation", "2")
      .then(() => {
        this.imgSource.getImageProperty("Orientation").then(data => {
          console.info(`q_yS1 Orientation = ` + data);
        }).catch((err) => {
          console.info(`q_yS2 getImageProperty failed, err`);
        })
      }).catch( error => {
      console.info('q_yS3 failed to modifyImageProperty ')
    })
//    modifyImageProperty(key: string, value: string, callback: AsyncCallback<void>): void
//    通过指定的键修改图片属性的值，使用callback形式返回结果。  需要写入权限
//    参数名：key	类型：string	 必填
//    参数名：value   类型：string   必填
//    参数名：callback   类型：AsyncCallback<void>   必填
//    key参数说明：图片属性名。
//    value参数说明：属性值。
//    callback参数说明：修改属性值，以callback形式返回结果。
//    测试结果：success
    this.imgSource.modifyImageProperty("Orientation", "2",() => {
      this.imgSource.getImageProperty("Orientation").then(data => {
        console.info(`q_yS4 Orientation = ` + data);
      }).catch((err) => {
        console.info(`q_yS5 getImageProperty failed, err`);
      })
    })

//    获取指定序号的图片信息，使用callback形式返回图片信息。 Success
    this.imgSource.getImageInfo(0,(error, imageInfo) => {
      if(error) {
        console.info('q_yS6 getImageInfo failed.');
      } else {
        console.info('q_yS7 getImageInfo succeeded and width is ' + imageInfo.size.width);
        console.info('q_yS8 getImageInfo succeeded and height is ' + imageInfo.size.height);
      }
    })

//    获取图片信息，使用callback形式返回图片信息  failed
    this.imgSource.getImageInfo(imageInfo => {
      console.info('q_yS9 getImageInfo succeeded and width is ' + imageInfo.size.width);
      console.info('q_yS10 getImageInfo succeeded and height is ' + imageInfo.size.height);
    })

//    获取图片信息，使用Promise形式返回图片信息。 Success
    this.imgSource.getImageInfo(0)
      .then(imageInfo => {
        console.info('q_yS11 getImageInfo succeeded and width is ' + imageInfo.size.width);
        console.info('q_yS12 getImageInfo succeeded and height is ' + imageInfo.size.height);
      }).catch(error => {
      console.log('q_yS13 Failed to obtain the image information.');
    })


//    更新增量数据，使用Promise形式返回结果。 Success
    const array = new ArrayBuffer(100);
    this.imgSource.updateData(array, false, 0, 10).then(() => {
      console.info('q_yS14 Succeeded in updating data.')
    })

//    更新增量数据，callback形式返回结果。 Success
    this.imgSource.updateData(array, false, 0, 10,(error,data )=> {
      if(data !== undefined){
        console.info('q_yS15 Succeeded in updating data.')
      }
    })

//    获取图片中给定索引处图像的指定属性键的值，用Promise形式返回结果。 Success
    this.imgSource.getImageProperty("Orientation")
      .then(data => {
        console.info('q_yS16 Succeeded in getting the value of the specified attribute key of the image and the data is:' + data)
      }).catch(error => {
      console.info("q_yS17 Failed to get the value of the specified attribute.")
    })
//    获取图片中给定索引处图像的指定属性键的值，用callback形式返回结果。 Success
    this.imgSource.getImageProperty("GPSLatitude",(error, data) => {
      if(error) {
        console.info('q_yS18 Failed to get the value of the specified attribute key of the image.')
      } else {
        console.info('q_yS19 Succeeded in getting the value of the specified attribute key of the image and the data is:' + data)
      }
    })
//    获取图片指定属性键的值，callback形式返回结果。 Success
    var property = {index: 1, defaultValue: '100'}
    this.imgSource.getImageProperty("Orientation",property,(error,data) => {
      if(error) {
        console.info('q_yS20 Failed to get the value of the specified attribute key of the image.')
      } else {
        console.info('q_yS21 Succeeded in getting the value of the specified attribute key of the image and the data is:' + data)
      }
    })

//    通过图片解码参数创建PixelMap对象 Success
    this.imgSource.createPixelMap().then(pixelmap => {
      console.log('q_yS22 Succeeded in creating pixelmap object through image decoding parameters.');
    }).catch(error => {
      console.log('q_yS23 Failed to create pixelmap object through image decoding parameters.');
    })

//    通过默认参数创建PixelMap对象，使用callback形式返回结果。 Success
    this.imgSource.createPixelMap((err, pixelmap) => {
        console.info('q_yS24 Succeeded in creating pixelmap object.')
      })

//    通过默认参数创建PixelMap对象，使用callback形式返回结果。 Success
    let decodingOptions = {
      editable: true,
      desiredPixelFormat: 3
    }
    this.imgSource.createPixelMap(decodingOptions, pixelmap => {
      console.log('q_yS25 Succeeded in creating pixelmap object.');
    })

////    释放图片源实例，使用Promise形式返回结果。 Success
//    this.imgSource.release().then(()=>{
//      console.info('q_yS26 Success in releasing the image source instance.');
//    })
//
////    释放图片源实例，使用callback形式返回结果。 Success
//    this.imgSource.release(() => {
//      console.info('q_yS27 Success in releasing the image source instance.');
//    })
  }

  build() {
    Row() {
      Column() {

        Text('透明度：' + this.op)
          .fontWeight(FontWeight.Bold).margin(15)

        TextInput({ placeholder: '透明度' })
          .type(InputType.Normal)
          .placeholderColor(Color.Gray)
          .placeholderFont({ size: 12 })
          .onChange((value: string) => {
            this.op = value
          })
//        Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center, wrap:FlexWrap.Wrap}) {
//          Text('key：' + this.key)
//            .fontWeight(FontWeight.Bold)
//        } .margin(20)

//        Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
//          TextInput({ placeholder: 'KEY' })
//            .type(InputType.Normal)
//            .placeholderColor(Color.Gray)
//            .placeholderFont({ size: 12 })
//            .onChange((value: string) => {
//              this.key = value
//            })
//        } .margin(20)

        Button('提交').onClick(() => {
          this.btn()
        }).height('8%')
          .width('20%')
          .margin(20)

        Button('imageS').onClick(() => {
          this.imageS()
        }).height('8%')
          .width('20%')
          .margin(20)

        Flex({ justifyContent: FlexAlign.SpaceAround }) {
          Image(this._pixelMap)
            .border({ width: 1, color: Color.Blue })
            .width(500).height(500)
            .objectFit(ImageFit.None)
            .backgroundColor(Color.Red)
        }
      }
      .width('100%')
    }
    .height('100%')
  }
}