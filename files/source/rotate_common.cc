/*
 *  Copyright 2011 The LibYuv Project Authors. All rights reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/row.h"
#include "libyuv/rotate_row.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

#if defined(HAS_TRANSPOSEWX16_MSA)
void TransposeWx16_C(const uint8_t *src, int32_t src_stride,
                     uint8_t *dst, int32_t dst_stride,
                     int width) {
  int x;

  for (x = 0; x < width; ++x) {
    dst[0] = src[0 * src_stride];
    dst[1] = src[1 * src_stride];
    dst[2] = src[2 * src_stride];
    dst[3] = src[3 * src_stride];
    dst[4] = src[4 * src_stride];
    dst[5] = src[5 * src_stride];
    dst[6] = src[6 * src_stride];
    dst[7] = src[7 * src_stride];
    dst[8] = src[8 * src_stride];
    dst[9] = src[9 * src_stride];
    dst[10] = src[10 * src_stride];
    dst[11] = src[11 * src_stride];
    dst[12] = src[12 * src_stride];
    dst[13] = src[13 * src_stride];
    dst[14] = src[14 * src_stride];
    dst[15] = src[15 * src_stride];
    ++src;
    dst += dst_stride;
  }
}
#else
void TransposeWx8_C(const uint8* src, int src_stride,
                    uint8* dst, int dst_stride, int width) {
  int i;
  for (i = 0; i < width; ++i) {
    dst[0] = src[0 * src_stride];
    dst[1] = src[1 * src_stride];
    dst[2] = src[2 * src_stride];
    dst[3] = src[3 * src_stride];
    dst[4] = src[4 * src_stride];
    dst[5] = src[5 * src_stride];
    dst[6] = src[6 * src_stride];
    dst[7] = src[7 * src_stride];
    ++src;
    dst += dst_stride;
  }
}
#endif

#if defined(HAS_TRANSPOSEUVWX16_MSA)
void TransposeUVWx16_C(const uint8_t *src, int32_t src_stride,
                       uint8_t *dst_a, int32_t dst_stride_a,
                       uint8_t *dst_b, int32_t dst_stride_b,
                       int width) {
  int x;

  for (x = 0; x < width; ++x) {
    dst_a[0] = src[0 * src_stride + 0];
    dst_b[0] = src[0 * src_stride + 1];
    dst_a[1] = src[1 * src_stride + 0];
    dst_b[1] = src[1 * src_stride + 1];
    dst_a[2] = src[2 * src_stride + 0];
    dst_b[2] = src[2 * src_stride + 1];
    dst_a[3] = src[3 * src_stride + 0];
    dst_b[3] = src[3 * src_stride + 1];
    dst_a[4] = src[4 * src_stride + 0];
    dst_b[4] = src[4 * src_stride + 1];
    dst_a[5] = src[5 * src_stride + 0];
    dst_b[5] = src[5 * src_stride + 1];
    dst_a[6] = src[6 * src_stride + 0];
    dst_b[6] = src[6 * src_stride + 1];
    dst_a[7] = src[7 * src_stride + 0];
    dst_b[7] = src[7 * src_stride + 1];
    dst_a[8] = src[8 * src_stride + 0];
    dst_b[8] = src[8 * src_stride + 1];
    dst_a[9] = src[9 * src_stride + 0];
    dst_b[9] = src[9 * src_stride + 1];
    dst_a[10] = src[10 * src_stride + 0];
    dst_b[10] = src[10 * src_stride + 1];
    dst_a[11] = src[11 * src_stride + 0];
    dst_b[11] = src[11 * src_stride + 1];
    dst_a[12] = src[12 * src_stride + 0];
    dst_b[12] = src[12 * src_stride + 1];
    dst_a[13] = src[13 * src_stride + 0];
    dst_b[13] = src[13 * src_stride + 1];
    dst_a[14] = src[14 * src_stride + 0];
    dst_b[14] = src[14 * src_stride + 1];
    dst_a[15] = src[15 * src_stride + 0];
    dst_b[15] = src[15 * src_stride + 1];
    src += 2;
    dst_a += dst_stride_a;
    dst_b += dst_stride_b;
  }
}
#else
void TransposeUVWx8_C(const uint8* src, int src_stride,
                      uint8* dst_a, int dst_stride_a,
                      uint8* dst_b, int dst_stride_b, int width) {
  int i;
  for (i = 0; i < width; ++i) {
    dst_a[0] = src[0 * src_stride + 0];
    dst_b[0] = src[0 * src_stride + 1];
    dst_a[1] = src[1 * src_stride + 0];
    dst_b[1] = src[1 * src_stride + 1];
    dst_a[2] = src[2 * src_stride + 0];
    dst_b[2] = src[2 * src_stride + 1];
    dst_a[3] = src[3 * src_stride + 0];
    dst_b[3] = src[3 * src_stride + 1];
    dst_a[4] = src[4 * src_stride + 0];
    dst_b[4] = src[4 * src_stride + 1];
    dst_a[5] = src[5 * src_stride + 0];
    dst_b[5] = src[5 * src_stride + 1];
    dst_a[6] = src[6 * src_stride + 0];
    dst_b[6] = src[6 * src_stride + 1];
    dst_a[7] = src[7 * src_stride + 0];
    dst_b[7] = src[7 * src_stride + 1];
    src += 2;
    dst_a += dst_stride_a;
    dst_b += dst_stride_b;
  }
}
#endif

void TransposeWxH_C(const uint8* src, int src_stride,
                    uint8* dst, int dst_stride,
                    int width, int height) {
  int i;
  for (i = 0; i < width; ++i) {
    int j;
    for (j = 0; j < height; ++j) {
      dst[i * dst_stride + j] = src[j * src_stride + i];
    }
  }
}

void TransposeUVWxH_C(const uint8* src, int src_stride,
                      uint8* dst_a, int dst_stride_a,
                      uint8* dst_b, int dst_stride_b,
                      int width, int height) {
  int i;
  for (i = 0; i < width * 2; i += 2) {
    int j;
    for (j = 0; j < height; ++j) {
      dst_a[j + ((i >> 1) * dst_stride_a)] = src[i + (j * src_stride)];
      dst_b[j + ((i >> 1) * dst_stride_b)] = src[i + (j * src_stride) + 1];
    }
  }
}

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
