// -*- mode:c++; tab-width:2; indent-tabs-mode:nil; c-basic-offset:2 -*-
/*
 *  Copyright 2010-2011 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <fstream>
#include <string>
#include "src/ImageReaderSource.h"
#include <zxing/common/Counted.h>
#include <zxing/Binarizer.h>
#include <zxing/MultiFormatReader.h>
#include <zxing/Result.h>
#include <zxing/ReaderException.h>
#include <zxing/common/GlobalHistogramBinarizer.h>
#include <zxing/common/HybridBinarizer.h>
#include <exception>
#include <zxing/Exception.h>
#include <zxing/common/IllegalArgumentException.h>
#include <zxing/BinaryBitmap.h>
#include <zxing/DecodeHints.h>

#include <zxing/qrcode/QRCodeReader.h>
#include <zxing/multi/qrcode/QRCodeMultiReader.h>
#include <zxing/multi/ByQuadrantReader.h>
#include <zxing/multi/MultipleBarcodeReader.h>
#include <zxing/multi/GenericMultipleBarcodeReader.h>

using namespace std;
using namespace zxing;
using namespace zxing::multi;
using namespace zxing::qrcode;

vector<Ref<Result> > decode(Ref<BinaryBitmap> image, DecodeHints hints) {
  Ref<Reader> reader(new MultiFormatReader);
  return vector<Ref<Result> >(1, reader->decode(image, hints));
}

extern "C" {

  static const char *imagePtr = NULL;
  static zxing::ArrayRef<char> image = NULL;
  static Ref<LuminanceSource> source;

  const char* resize(int width, int height) {
    image = zxing::ArrayRef<char>(width * height);
    imagePtr = &image[0];
    source = Ref<LuminanceSource>(new ImageReaderSource(image, width, height, 1));
    return imagePtr;
  }


  int __decode(void *decode_callback(const char *resultStr, int resultStrLen, int resultIndex, int resultCount)) {
    vector<Ref<Result> > results;
    int res = -1;
    bool hybrid = true;

    try {
      Ref<Binarizer> binarizer;
      if (hybrid) {
        binarizer = new HybridBinarizer(source);
      } else {
        binarizer = new GlobalHistogramBinarizer(source);
      }
      DecodeHints hints(DecodeHints::DEFAULT_HINT);
      hints.setTryHarder(false);
      Ref<BinaryBitmap> binary(new BinaryBitmap(binarizer));
      results = decode(binary, hints);
      res = 0;
    } catch (const ReaderException& e) {
      // cell_result = "zxing::ReaderException: " + string(e.what());
      res = -2;
    } catch (const zxing::IllegalArgumentException& e) {
      // cell_result = "zxing::IllegalArgumentException: " + string(e.what());
      res = -3;
    } catch (const zxing::Exception& e) {
      // cell_result = "zxing::Exception: " + string(e.what());
      res = -4;
    } catch (const std::exception& e) {
      // cell_result = "std::exception: " + string(e.what());
      res = -5;
    }

    if (res == 0) {
      for (int i=0; i<results.size(); i++) {
        std::string result = results[i]->getText()->getText();
        decode_callback(result.c_str(), result.size(), i, results.size());
      }
    }

    return res;
  }

  int decode_any(void *callback(const char*, int, int, int)) {
    return __decode(callback);
  }

}