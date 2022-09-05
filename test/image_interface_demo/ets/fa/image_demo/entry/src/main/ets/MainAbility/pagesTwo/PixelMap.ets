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
  @State str1: number = 0
  @State str2: number = 0
  @State str1H: number = 0
  @State str2H: number = 0
  @State error: object = undefined
  @State scaleX: number = 1
  @State scaleY: number = 1
  @State translateX: number = 0
  @State translateY: number = 0
  @State rotateC: number = 0
  @State flipX: boolean = false
  @State flipY: boolean = false
  @State regionH: number = 0
  @State regionW: number = 0
  @State regionX: number = 0
  @State regionY: number = 0
  @State op: number = 1

  // 通过访问指定资源文件下的fileName，获取fd
  //文件路径为/resources/rawfile
  async getFd(fileName) {
    let mgr = await resourceManager.getResourceManager()
    let value = await mgr.getRawFileDescriptor(fileName)
    return value.fd
  }

  async aboutToAppear() {
    let fd = await this.getFd('opacity.png')
    this.imgSource = multimedia_image.createImageSource(fd)
  }

  async btn() {
    let decodingOptions = {
      editable: true,
      desiredPixelFormat: 3
    }
    this.imgSource.createPixelMap(decodingOptions).then(async (pixelMap) => {
      let orgInfo = await pixelMap.getImageInfo()
      this.str1 = orgInfo.size.width
      this.str1H = orgInfo.size.height

//      根据输入的尺寸对图片进行裁剪
      let region = { size: { height: Number(this.regionH), width: Number(this.regionW) }, x: Number(this.regionX), y: Number(this.regionY) };
      await pixelMap.crop(region);

//      根据输入的宽高对图片进行缩放。
      await pixelMap.scale(Number(this.scaleX), Number(this.scaleY))

//      根据输入的坐标对图片进行位置变换。
      await pixelMap.translate(Number(this.translateX), Number(this.translateY))

//      根据输入的角度对图片进行旋转。
      await pixelMap.rotate(this.rotateC)

//      根据输入的条件对图片进行翻转
      await pixelMap.flip(this.flipX, this.flipY);

//      通过设置透明比率来让PixelMap达到对应的透明效果
      await pixelMap.opacity(Number(this.op)) //要求图片带有透明度通道

      let newInfo = await pixelMap.getImageInfo()
      this.str2 = newInfo.size.width
      this.str2H = newInfo.size.height

//      readPixelsToBuffer(dst: ArrayBuffer): Promise<void>
//     读取图像像素数据，结果写入ArrayBuffer里，使用promise形式返回。指定BGRA_8888格式创建pixelmap，读取的像素数据与原数据保持一致。
//      参数名：dst	类型：ArrayBuffer	必填
//      dst参数说明：缓冲区，函数执行结束后获取的图像像素数据写入到该内存区域内。缓冲区大小由getPixelBytesNumber接口获取。
//     测试结果：success
      const readBuffer = new ArrayBuffer(pixelMap.getPixelBytesNumber());
      pixelMap.readPixelsToBuffer(readBuffer).then(() => {
        //符合条件则进入
        console.info('q_y2 Succeeded in reading image pixel data --promise.');
      }).catch(error => {
        //不符合条件则进入
        console.info('q_y3 Failed to read image pixel data --promise.');
      })

//      readPixelsToBuffer(dst: ArrayBuffer, callback: AsyncCallback<void>): void
//     读取图像像素数据，结果写入ArrayBuffer里，使用callback形式返回。指定BGRA_8888格式创建pixelmap，读取的像素数据与原数据保持一致。
//      参数名：dst	类型：ArrayBuffer	必填
//      参数名：callback  类型：AsyncCallback<void>   必填
//      dst参数说明：缓冲区，函数执行结束后获取的图像像素数据写入到该内存区域内。缓冲区大小由getPixelBytesNumber接口获取。
//      callback参数说明：获取回调，失败时返回错误信息。
//     测试结果：success
      pixelMap.readPixelsToBuffer(readBuffer, () => {
        //符合条件则进入
        console.info('q_y4 Succeeded in reading the image data in the area --promise.');
      })

//      readPixels(area: PositionArea): Promise<void>
//     读取区域内的图片数据，使用promise形式返回读取结果。
//      参数名：area	类型：PositionArea	必填
//      area参数说明：区域大小，根据区域读取。
//     测试结果：success
      const area = { pixels: new ArrayBuffer(8),
        offset: 0,
        stride: 8,
        region: { size: { height: 1, width: 2 }, x: 0, y: 0 }
      };
      pixelMap.readPixels(area).then(() => {
        console.info('q_y6 Succeeded in reading the image data in the area --promise.'); //符合条件则进入
      }).catch(error => {
        console.info('q_y7 Failed to read the image data in the area --promise.'); //不符合条件则进入
      })

//      readPixels(area: PositionArea, callback: AsyncCallback<void>): void
//     读取区域内的图片数据，使用callback形式返回读取结果。
//      参数名：area	类型：PositionArea	必填
//      参数名：callback  类型：AsyncCallback<void>   必填
//      area参数说明：区域大小，根据区域读取。
//      callback参数说明：获取回调，失败时返回错误信息。
//     测试结果：success
      pixelMap.readPixels(area, () => {
        console.info('q_y8 ReadPixels success --callback');
      })

//      writePixels(area: PositionArea): Promise<void>
//     将PixelMap写入指定区域内，使用promise形式返回写入结果。
//      参数名：area	类型：PositionArea	必填
//      area参数说明：区域大小，根据区域写入。
//     测试结果：success
      const writeArea = { pixels: new ArrayBuffer(8),
        offset: 0,
        stride: 8,
        region: { size: { height: 1, width: 2 }, x: 0, y: 0 }
      };
      pixelMap.writePixels(writeArea).then(() => {
        //符合条件则进入
        console.info('q_y10 Succeeded in writing the image data in the area --promise.');
      }).catch(error => {
        //不符合条件则进入
        console.info('q_y9 Failed to write the image data in the area --promise.');
      })

//      writePixels(area: PositionArea, callback: AsyncCallback<void>): void
//     将PixelMap写入指定区域内，使用callback形式返回写入结果。
//      参数名：area	类型：PositionArea	必填
//      参数名：callback  类型：AsyncCallback<void>  必填
//      area参数说明：区域大小，根据区域写入。
//      callback参数说明：获取回调，失败时返回错误信息。
//     测试结果：success
      pixelMap.writePixels(writeArea, (error) => {
        if (error != undefined) {
          console.info('q_y11 Failed to write pixelMap into the specified area --callback.');
        } else {
          console.info('q_y12 Succeeded to write pixelMap into the specified area --callback.');
        }
      })

      const writeColor = new ArrayBuffer(96);
//      writeBufferToPixels(src: ArrayBuffer): Promise<void>
//     读取缓冲区中的图片数据，结果写入PixelMap中，使用callback形式返回。
//      参数名：src	类型：ArrayBuffer	必填
//      src参数说明：图像像素数据。
//     测试结果：Failed
//      var bufferArr = new Uint8Array(writeColor);
//      for (var i = 0; i < bufferArr.length; i++) {
//        bufferArr[i] = i + 1;
//      }
//      pixelMap.writeBufferToPixels(writeColor).then(() => {
//        pixelMap.readPixelsToBuffer(readBuffer).then(() => {
//          var res = true;
//          for (var i = 0; i < bufferArr.length; i++) {
//            if (bufferArr[i] == 0) {
//              res = false;
//              console.info('q_y13 failed');
//              expect(false).assertTrue()
//              done();
//              break;
//            }
//          }
//          if (res) {
//            console.info('q_y14 success');
//            expect(true).assertTrue();
//            done();
//          }
//        })
//      })

//      writeBufferToPixels(src: ArrayBuffer, callback: AsyncCallback<void>): void
//     读取缓冲区中的图片数据，结果写入PixelMap中，使用callback形式返回。
//      参数名：src	类型：ArrayBuffer	必填
//      参数名：callback  类型：AsyncCallback<void>  必填
//      src参数说明：图像像素数据。
//      callback参数说明：获取回调，失败时返回错误信息。
//     测试结果：success
      pixelMap.writeBufferToPixels(writeColor,() => {
        pixelMap.readPixelsToBuffer(readBuffer,() => {
          var bufferArr = new Uint8Array(readBuffer);
          var res = true;
          for (var i = 0; i < bufferArr.length; i++) {
            if(res) {
              if (bufferArr[i] == 0) {
                res = false;
                console.info('q_y15 Success writeBufferToPixels');
                expect(true).assertTrue()
                done();
                break;
              }
            }
          }
          if (res) {
            console.info('q_y16 no change after writeBuffer');
            expect(false).assertTrue();
            done();
          }
        })
      })

//      getImageInfo(): Promise<ImageInfo>
//     获取图像像素信息，使用promise形式返回获取的图像像素信息。
//      返回值类型：Promise<ImageInfo>	必填
//      返回值说明：Promise实例，用于异步获取图像像素信息，失败时返回错误信息。
//     测试结果：success
      pixelMap.getImageInfo().then(function(info) {
        console.info('q_y17 --promise info.width = ' + info.size.width);
        console.info('q_y18 --promise info.height = ' + info.size.height);
      }).catch((err) => {
        console.info("q_y19 Failed to obtain the image pixel map information --promise.");
      });

//      getImageInfo(callback: AsyncCallback<ImageInfo>): void
//     获取图像像素信息，使用callback形式返回获取的图像像素信息。 success 但是打印不出宽高
//      参数名：callback	类型：AsyncCallback<ImageInfo>	必填
//      callback参数说明：获取图像像素信息回调，异步返回图像像素信息，失败时返回错误信息。
//     测试结果：failed
      pixelMap.getImageInfo((imageInfo) => {
        console.info('q_y20 --callback imageInfo.width = ' + imageInfo.size.width);
        console.info('q_y21 --callback imageInfo.height = ' + imageInfo.size.height);
      })

//      获取图像像素每行字节数。
      let rowCount = pixelMap.getBytesNumberPerRow();
      console.info("q_y22 pixelBytesNumber =" + rowCount);

//     获取图像像素的总字节数。 success
      const pixelBytesNumber = pixelMap.getPixelBytesNumber();
      console.info("q_y23 pixelBytesNumber =" + pixelBytesNumber);

//     获取当前图像像素的密度。 success
      const getDensity = pixelMap.getDensity();
      console.info("q_y24 getDensity =" + getDensity);

//      opacity(rate: number, callback: AsyncCallback<void>): void
//     通过设置透明比率来让PixelMap达到对应的透明效果，使用callback形式返回，要求图片带有开发通道。 success
//      参数名：rate	类型：number	必填
//      参数名：callback  类型：AsyncCallback<void>  必填
//      rate参数说明：透明比率的值,取值范围：0-1。
//     callback参数说明：获取回调，失败时返回错误信息。
//     测试结果：success
      pixelMap.opacity((this.op),()=>{
        console.info("q_y25 Succeeded in obtaining the opacity and the opacity is : " + this.op);
      })

//      createAlphaPixelmap(): Promise<PixelMap>
//     根据Alpha通道的信息，来生成一个仅包含Alpha通道信息的pixelMap，可用于阴影效果，使用promise形式返回。 success
//      返回值类型：Promise<PixelMap>		必填
//      返回值说明：Promise实例，返回pixelmap。
//     测试结果：success
      pixelMap.createAlphaPixelmap(async (err, alphaPixelMap) => {
        if (alphaPixelMap == undefined) {
          console.error('q_y26 Failed to obtain new pixel map --promise.');
        } else {
          console.info('q_y27 Succeed in obtaining new pixel map --promise.');
        }
      })

//      createAlphaPixelmap(callback: AsyncCallback<PixelMap>): void
//     根据Alpha通道的信息，来生成一个仅包含Alpha通道信息的pixelMap，可用于阴影效果，使用callback形式返回。 success
//      参数名：rate	类型：number	必填
//      参数名：callback  类型：AsyncCallback<PixelMap>  必填
//      参数说明：获取回调，异步返回PixelMap。
//     测试结果：success
      pixelMap.createAlphaPixelmap(async (err, alphaPixelMap) => {
        console.info('q_y28 Succeed in obtain new pixel map.');
      })
      this._pixelMap = pixelMap

//    release():Promise<void>
//     释放PixelMap对象，使用promiss形式返回释放结果。 success

//      pixelMap.release()
//        .then(() => {
//        console.info('q_y29 Succeeded in releasing pixelMap object.');
//      }).catch(error => {
//        console.info('q_y30 Failed to release pixelMap object.');
//      })

////     释放PixelMap对象，使用callback形式返回释放结果。 success
//      pixelMap.release(() => {
//        console.log('q_y31 Succeeded in releasing pixelmap object.');
//      })

    }).catch(error => {
      this.error = error
      console.log('pixelMap error' + error);
    })
  }

  build() {
    Row() {
      Column() {
        Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center, wrap:FlexWrap.Wrap}) {
          Text('原始宽度：' + this.str1)
            .fontWeight(FontWeight.Bold)
          Text('最终宽度：' + this.str2)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('原始宽度：' + this.str1H)
            .fontWeight(FontWeight.Bold)
          Text('最终宽度：' + this.str2H)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('缩放参数X：' + this.scaleX)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('缩放参数Y：' + this.scaleY)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('平移参数X：' + this.translateX)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('平移参数Y：' + this.translateY)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('旋转角度：' + this.rotateC)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('水平翻转：' + this.flipX)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('垂直翻转：' + this.flipY)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('截取宽度：' + this.regionW)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('截取高度：' + this.regionH)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('截取X坐标：' + this.regionX)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('截取Y坐标：' + this.regionY)
            .fontWeight(FontWeight.Bold).margin(15)
          Text('透明度：' + this.op)
            .fontWeight(FontWeight.Bold).margin(15)
        }

        Flex({ direction: FlexDirection.Row, alignItems: ItemAlign.Center, justifyContent: FlexAlign.Center }) {
          TextInput({ placeholder: '缩放参数X' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.scaleX = value
            })
          TextInput({ placeholder: '缩放参数Y' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.scaleY = value
            })
          TextInput({ placeholder: '平移参数X' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.translateX = value
            })
          TextInput({ placeholder: '平移参数Y' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.translateY = value
            })
          TextInput({ placeholder: '旋转角度' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.rotateC = value
            })
          Text('水平翻转'+this.flipX).onClick(()=>{
            this.flipX = !this.flipX
          }).border({width:1, color:Color.Gray, radius:5}).textAlign(TextAlign.Center).width(500)
          Text('垂直翻转'+this.flipY).onClick(()=>{
            this.flipY = !this.flipY
          }).border({width:1, color:Color.Gray, radius:5}).textAlign(TextAlign.Center).width(500)
          TextInput({ placeholder: '截取宽度' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.regionW = value
            })
          TextInput({ placeholder: '截取高度' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.regionH = value
            })
          TextInput({ placeholder: '截取X坐标' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.regionX = value
            })
          TextInput({ placeholder: '截取Y坐标' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.regionY = value
            })
          TextInput({ placeholder: '透明度' })
            .type(InputType.Normal)
            .placeholderColor(Color.Gray)
            .placeholderFont({ size: 12 })
            .onChange((value: string) => {
              this.op = value
            })
        }
        Button('提交').onClick(() => {
          this.btn()
        }).height('10%')
          .width('15%')
        Flex({ justifyContent: FlexAlign.SpaceAround }) {
          Image($r("app.media.opacity"))
            .border({ width: 1, color: Color.Blue })
            .width(500)
            .height(500)
            .objectFit(ImageFit.None)
          Image(this._pixelMap).border({ width: 1, color: Color.Blue }).width(500).height(500).objectFit(ImageFit.None)
            .backgroundColor(Color.Red)
        }
      }
      .width('100%')
    }
    .height('100%')
  }
}