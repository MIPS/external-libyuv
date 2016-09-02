/*
 *  Copyright (c) 2012 The LibYuv project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS. All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "libyuv/row.h"

#ifdef __cplusplus
namespace libyuv {
extern "C" {
#endif

// The following are available on Mips platforms:
#if !defined(LIBYUV_DISABLE_MIPS) && defined(LIBYUV_MIPS)

#ifdef HAS_COPYROW_MIPS
void CopyRow_MIPS(const uint8* src, uint8* dst, int count) {
  __asm__ __volatile__ (
    ".set      noreorder                         \n"
    ".set      noat                              \n"
    "slti      $at, %[count], 8                  \n"
    "bne       $at ,$zero, $last8                \n"
    "xor       $t8, %[src], %[dst]               \n"
    "andi      $t8, $t8, 0x3                     \n"

    "bne       $t8, $zero, unaligned             \n"
    "negu      $a3, %[dst]                       \n"
    // make dst/src aligned
    "andi      $a3, $a3, 0x3                     \n"
    "beq       $a3, $zero, $chk16w               \n"
    // word-aligned now count is the remining bytes count
    "subu     %[count], %[count], $a3            \n"

    "lwr       $t8, 0(%[src])                    \n"
    "addu      %[src], %[src], $a3               \n"
    "swr       $t8, 0(%[dst])                    \n"
    "addu      %[dst], %[dst], $a3               \n"

    // Now the dst/src are mutually word-aligned with word-aligned addresses
    "$chk16w:                                    \n"
    "andi      $t8, %[count], 0x3f               \n"  // whole 64-B chunks?
    // t8 is the byte count after 64-byte chunks
    "beq       %[count], $t8, chk8w              \n"
    // There will be at most 1 32-byte chunk after it
    "subu      $a3, %[count], $t8                \n"  // the reminder
    // Here a3 counts bytes in 16w chunks
    "addu      $a3, %[dst], $a3                  \n"
    // Now a3 is the final dst after 64-byte chunks
    "addu      $t0, %[dst], %[count]             \n"
    // t0 is the "past the end" address

    // When in the loop we exercise "pref 30,x(a1)", the a1+x should not be past
    // the "t0-32" address
    // This means: for x=128 the last "safe" a1 address is "t0-160"
    // Alternatively, for x=64 the last "safe" a1 address is "t0-96"
    // we will use "pref 30,128(a1)", so "t0-160" is the limit
    "subu      $t9, $t0, 160                     \n"
    // t9 is the "last safe pref 30,128(a1)" address
    "pref      0, 0(%[src])                      \n"  // first line of src
    "pref      0, 32(%[src])                     \n"  // second line of src
    "pref      0, 64(%[src])                     \n"
    "pref      30, 32(%[dst])                    \n"
    // In case the a1 > t9 don't use "pref 30" at all
    "sgtu      $v1, %[dst], $t9                  \n"
    "bgtz      $v1, $loop16w                     \n"
    "nop                                         \n"
    // otherwise, start with using pref30
    "pref      30, 64(%[dst])                    \n"
    "$loop16w:                                    \n"
    "pref      0, 96(%[src])                     \n"
    "lw        $t0, 0(%[src])                    \n"
    "bgtz      $v1, $skip_pref30_96              \n"  // skip
    "lw        $t1, 4(%[src])                    \n"
    "pref      30, 96(%[dst])                    \n"  // continue
    "$skip_pref30_96:                            \n"
    "lw        $t2, 8(%[src])                    \n"
    "lw        $t3, 12(%[src])                   \n"
    "lw        $t4, 16(%[src])                   \n"
    "lw        $t5, 20(%[src])                   \n"
    "lw        $t6, 24(%[src])                   \n"
    "lw        $t7, 28(%[src])                   \n"
    "pref      0, 128(%[src])                    \n"
    //  bring the next lines of src, addr 128
    "sw        $t0, 0(%[dst])                    \n"
    "sw        $t1, 4(%[dst])                    \n"
    "sw        $t2, 8(%[dst])                    \n"
    "sw        $t3, 12(%[dst])                   \n"
    "sw        $t4, 16(%[dst])                   \n"
    "sw        $t5, 20(%[dst])                   \n"
    "sw        $t6, 24(%[dst])                   \n"
    "sw        $t7, 28(%[dst])                   \n"
    "lw        $t0, 32(%[src])                   \n"
    "bgtz      $v1, $skip_pref30_128             \n"  // skip pref 30,128(a1)
    "lw        $t1, 36(%[src])                   \n"
    "pref      30, 128(%[dst])                   \n"  // set dest, addr 128
    "$skip_pref30_128:                           \n"
    "lw        $t2, 40(%[src])                   \n"
    "lw        $t3, 44(%[src])                   \n"
    "lw        $t4, 48(%[src])                   \n"
    "lw        $t5, 52(%[src])                   \n"
    "lw        $t6, 56(%[src])                   \n"
    "lw        $t7, 60(%[src])                   \n"
    "pref      0, 160(%[src])                    \n"
    // bring the next lines of src, addr 160
    "sw        $t0, 32(%[dst])                   \n"
    "sw        $t1, 36(%[dst])                   \n"
    "sw        $t2, 40(%[dst])                   \n"
    "sw        $t3, 44(%[dst])                   \n"
    "sw        $t4, 48(%[dst])                   \n"
    "sw        $t5, 52(%[dst])                   \n"
    "sw        $t6, 56(%[dst])                   \n"
    "sw        $t7, 60(%[dst])                   \n"

    "addiu     %[dst], %[dst], 64                \n"  // adding 64 to dest
    "sgtu      $v1, %[dst], $t9                  \n"
    "bne       %[dst], $a3, $loop16w             \n"
    " addiu    %[src], %[src], 64                \n"  // adding 64 to src
    "move      %[count], $t8                     \n"

    // Here we have src and dest word-aligned but less than 64-bytes to go

    "chk8w:                                      \n"
    "pref      0, 0x0(%[src])                    \n"
    "andi      $t8, %[count], 0x1f               \n"  // 32-byte chunk?
    // the t8 is the reminder count past 32-bytes
    "beq       %[count], $t8, chk1w              \n"
    // count=t8,no 32-byte chunk
    " nop                                        \n"

    "lw        $t0, 0(%[src])                    \n"
    "lw        $t1, 4(%[src])                    \n"
    "lw        $t2, 8(%[src])                    \n"
    "lw        $t3, 12(%[src])                   \n"
    "lw        $t4, 16(%[src])                   \n"
    "lw        $t5, 20(%[src])                   \n"
    "lw        $t6, 24(%[src])                   \n"
    "lw        $t7, 28(%[src])                   \n"
    "addiu     %[src], %[src], 32                \n"

    "sw        $t0, 0(%[dst])                    \n"
    "sw        $t1, 4(%[dst])                    \n"
    "sw        $t2, 8(%[dst])                    \n"
    "sw        $t3, 12(%[dst])                   \n"
    "sw        $t4, 16(%[dst])                   \n"
    "sw        $t5, 20(%[dst])                   \n"
    "sw        $t6, 24(%[dst])                   \n"
    "sw        $t7, 28(%[dst])                   \n"
    "addiu     %[dst], %[dst], 32                \n"

    "chk1w:                                      \n"
    "andi      %[count], $t8, 0x3                \n"
    // now count is the reminder past 1w chunks
    "beq       %[count], $t8, $last8             \n"
    " subu     $a3, $t8, %[count]                \n"
    // a3 is count of bytes in 1w chunks
    "addu      $a3, %[dst], $a3                  \n"
    // now a3 is the dst address past the 1w chunks
    // copying in words (4-byte chunks)
    "$wordCopy_loop:                             \n"
    "lw        $t3, 0(%[src])                    \n"
    // the first t3 may be equal t0 ... optimize?
    "addiu     %[src], %[src],4                  \n"
    "addiu     %[dst], %[dst],4                  \n"
    "bne       %[dst], $a3,$wordCopy_loop        \n"
    " sw       $t3, -4(%[dst])                   \n"

    // For the last (<8) bytes
    "$last8:                                     \n"
    "blez      %[count], leave                   \n"
    " addu     $a3, %[dst], %[count]             \n"  // a3 -last dst address
    "$last8loop:                                 \n"
    "lb        $v1, 0(%[src])                    \n"
    "addiu     %[src], %[src], 1                 \n"
    "addiu     %[dst], %[dst], 1                 \n"
    "bne       %[dst], $a3, $last8loop           \n"
    " sb       $v1, -1(%[dst])                   \n"

    "leave:                                      \n"
    "  j       $ra                               \n"
    "  nop                                       \n"

    //
    // UNALIGNED case
    //

    "unaligned:                                  \n"
    // got here with a3="negu a1"
    "andi      $a3, $a3, 0x3                     \n"  // a1 is word aligned?
    "beqz      $a3, $ua_chk16w                   \n"
    " subu     %[count], %[count], $a3           \n"
    // bytes left after initial a3 bytes
    "lwr       $v1, 0(%[src])                    \n"
    "lwl       $v1, 3(%[src])                    \n"
    "addu      %[src], %[src], $a3               \n"  // a3 may be 1, 2 or 3
    "swr       $v1, 0(%[dst])                    \n"
    "addu      %[dst], %[dst], $a3               \n"
    // below the dst will be word aligned (NOTE1)
    "$ua_chk16w:                                 \n"
    "andi      $t8, %[count], 0x3f               \n"  // whole 64-B chunks?
    // t8 is the byte count after 64-byte chunks
    "beq       %[count], $t8, ua_chk8w           \n"
    // if a2==t8, no 64-byte chunks
    // There will be at most 1 32-byte chunk after it
    "subu      $a3, %[count], $t8                \n"  // the reminder
    // Here a3 counts bytes in 16w chunks
    "addu      $a3, %[dst], $a3                  \n"
    // Now a3 is the final dst after 64-byte chunks
    "addu      $t0, %[dst], %[count]             \n"  // t0 "past the end"
    "subu      $t9, $t0, 160                     \n"
    // t9 is the "last safe pref 30,128(a1)" address
    "pref      0, 0(%[src])                      \n"  // first line of src
    "pref      0, 32(%[src])                     \n"  // second line  addr 32
    "pref      0, 64(%[src])                     \n"
    "pref      30, 32(%[dst])                    \n"
    // safe, as we have at least 64 bytes ahead
    // In case the a1 > t9 don't use "pref 30" at all
    "sgtu      $v1, %[dst], $t9                  \n"
    "bgtz      $v1, $ua_loop16w                  \n"
    // skip "pref 30,64(a1)" for too short arrays
    " nop                                        \n"
    // otherwise, start with using pref30
    "pref      30, 64(%[dst])                    \n"
    "$ua_loop16w:                                \n"
    "pref      0, 96(%[src])                     \n"
    "lwr       $t0, 0(%[src])                    \n"
    "lwl       $t0, 3(%[src])                    \n"
    "lwr       $t1, 4(%[src])                    \n"
    "bgtz      $v1, $ua_skip_pref30_96           \n"
    " lwl      $t1, 7(%[src])                    \n"
    "pref      30, 96(%[dst])                    \n"
    // continue setting up the dest, addr 96
    "$ua_skip_pref30_96:                         \n"
    "lwr       $t2, 8(%[src])                    \n"
    "lwl       $t2, 11(%[src])                   \n"
    "lwr       $t3, 12(%[src])                   \n"
    "lwl       $t3, 15(%[src])                   \n"
    "lwr       $t4, 16(%[src])                   \n"
    "lwl       $t4, 19(%[src])                   \n"
    "lwr       $t5, 20(%[src])                   \n"
    "lwl       $t5, 23(%[src])                   \n"
    "lwr       $t6, 24(%[src])                   \n"
    "lwl       $t6, 27(%[src])                   \n"
    "lwr       $t7, 28(%[src])                   \n"
    "lwl       $t7, 31(%[src])                   \n"
    "pref      0, 128(%[src])                    \n"
    // bring the next lines of src, addr 128
    "sw        $t0, 0(%[dst])                    \n"
    "sw        $t1, 4(%[dst])                    \n"
    "sw        $t2, 8(%[dst])                    \n"
    "sw        $t3, 12(%[dst])                   \n"
    "sw        $t4, 16(%[dst])                   \n"
    "sw        $t5, 20(%[dst])                   \n"
    "sw        $t6, 24(%[dst])                   \n"
    "sw        $t7, 28(%[dst])                   \n"
    "lwr       $t0, 32(%[src])                   \n"
    "lwl       $t0, 35(%[src])                   \n"
    "lwr       $t1, 36(%[src])                   \n"
    "bgtz      $v1, ua_skip_pref30_128           \n"
    " lwl      $t1, 39(%[src])                   \n"
    "pref      30, 128(%[dst])                   \n"
    // continue setting up the dest, addr 128
    "ua_skip_pref30_128:                         \n"

    "lwr       $t2, 40(%[src])                   \n"
    "lwl       $t2, 43(%[src])                   \n"
    "lwr       $t3, 44(%[src])                   \n"
    "lwl       $t3, 47(%[src])                   \n"
    "lwr       $t4, 48(%[src])                   \n"
    "lwl       $t4, 51(%[src])                   \n"
    "lwr       $t5, 52(%[src])                   \n"
    "lwl       $t5, 55(%[src])                   \n"
    "lwr       $t6, 56(%[src])                   \n"
    "lwl       $t6, 59(%[src])                   \n"
    "lwr       $t7, 60(%[src])                   \n"
    "lwl       $t7, 63(%[src])                   \n"
    "pref      0, 160(%[src])                    \n"
    // bring the next lines of src, addr 160
    "sw        $t0, 32(%[dst])                   \n"
    "sw        $t1, 36(%[dst])                   \n"
    "sw        $t2, 40(%[dst])                   \n"
    "sw        $t3, 44(%[dst])                   \n"
    "sw        $t4, 48(%[dst])                   \n"
    "sw        $t5, 52(%[dst])                   \n"
    "sw        $t6, 56(%[dst])                   \n"
    "sw        $t7, 60(%[dst])                   \n"

    "addiu     %[dst],%[dst],64                  \n"  // adding 64 to dest
    "sgtu      $v1,%[dst],$t9                    \n"
    "bne       %[dst],$a3,$ua_loop16w            \n"
    " addiu    %[src],%[src],64                  \n"  // adding 64 to src
    "move      %[count],$t8                      \n"

    // Here we have src and dest word-aligned but less than 64-bytes to go

    "ua_chk8w:                                   \n"
    "pref      0, 0x0(%[src])                    \n"
    "andi      $t8, %[count], 0x1f               \n"  // 32-byte chunk?
    // the t8 is the reminder count
    "beq       %[count], $t8, $ua_chk1w          \n"
    // when count==t8, no 32-byte chunk

    "lwr       $t0, 0(%[src])                    \n"
    "lwl       $t0, 3(%[src])                    \n"
    "lwr       $t1, 4(%[src])                    \n"
    "lwl       $t1, 7(%[src])                    \n"
    "lwr       $t2, 8(%[src])                    \n"
    "lwl       $t2, 11(%[src])                   \n"
    "lwr       $t3, 12(%[src])                   \n"
    "lwl       $t3, 15(%[src])                   \n"
    "lwr       $t4, 16(%[src])                   \n"
    "lwl       $t4, 19(%[src])                   \n"
    "lwr       $t5, 20(%[src])                   \n"
    "lwl       $t5, 23(%[src])                   \n"
    "lwr       $t6, 24(%[src])                   \n"
    "lwl       $t6, 27(%[src])                   \n"
    "lwr       $t7, 28(%[src])                   \n"
    "lwl       $t7, 31(%[src])                   \n"
    "addiu     %[src], %[src], 32                \n"

    "sw        $t0, 0(%[dst])                    \n"
    "sw        $t1, 4(%[dst])                    \n"
    "sw        $t2, 8(%[dst])                    \n"
    "sw        $t3, 12(%[dst])                   \n"
    "sw        $t4, 16(%[dst])                   \n"
    "sw        $t5, 20(%[dst])                   \n"
    "sw        $t6, 24(%[dst])                   \n"
    "sw        $t7, 28(%[dst])                   \n"
    "addiu     %[dst], %[dst], 32                \n"

    "$ua_chk1w:                                  \n"
    "andi      %[count], $t8, 0x3                \n"
    // now count is the reminder past 1w chunks
    "beq       %[count], $t8, ua_smallCopy       \n"
    "subu      $a3, $t8, %[count]                \n"
    // a3 is count of bytes in 1w chunks
    "addu      $a3, %[dst], $a3                  \n"
    // now a3 is the dst address past the 1w chunks

    // copying in words (4-byte chunks)
    "$ua_wordCopy_loop:                          \n"
    "lwr       $v1, 0(%[src])                    \n"
    "lwl       $v1, 3(%[src])                    \n"
    "addiu     %[src], %[src], 4                 \n"
    "addiu     %[dst], %[dst], 4                 \n"
    // note: dst=a1 is word aligned here, see NOTE1
    "bne       %[dst], $a3, $ua_wordCopy_loop    \n"
    " sw       $v1,-4(%[dst])                    \n"

    // Now less than 4 bytes (value in count) left to copy
    "ua_smallCopy:                               \n"
    "beqz      %[count], leave                   \n"
    " addu     $a3, %[dst], %[count]             \n" // a3 = last dst address
    "$ua_smallCopy_loop:                         \n"
    "lb        $v1, 0(%[src])                    \n"
    "addiu     %[src], %[src], 1                 \n"
    "addiu     %[dst], %[dst], 1                 \n"
    "bne       %[dst],$a3,$ua_smallCopy_loop     \n"
    " sb       $v1, -1(%[dst])                   \n"

    "j         $ra                               \n"
    " nop                                        \n"
    ".set      at                                \n"
    ".set      reorder                           \n"
       : [dst] "+r" (dst), [src] "+r" (src)
       : [count] "r" (count)
       : "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
       "t8", "t9", "a3", "v1", "at"
  );
}
#endif  // HAS_COPYROW_MIPS

void MirrorRow_DSPR2(const uint8* src, uint8* dst, int width) {
  __asm__ __volatile__ (
    ".set push                             \n"
    ".set noreorder                        \n"

    "srl       $t4, %[width], 4            \n"  // multiplies of 16
    "andi      $t5, %[width], 0xf          \n"
    "blez      $t4, 2f                     \n"
    " addu     %[src], %[src], %[width]    \n"  // src += width

   "1:                                     \n"
    "lw        $t0, -16(%[src])            \n"  // |3|2|1|0|
    "lw        $t1, -12(%[src])            \n"  // |7|6|5|4|
    "lw        $t2, -8(%[src])             \n"  // |11|10|9|8|
    "lw        $t3, -4(%[src])             \n"  // |15|14|13|12|
    "wsbh      $t0, $t0                    \n"  // |2|3|0|1|
    "wsbh      $t1, $t1                    \n"  // |6|7|4|5|
    "wsbh      $t2, $t2                    \n"  // |10|11|8|9|
    "wsbh      $t3, $t3                    \n"  // |14|15|12|13|
    "rotr      $t0, $t0, 16                \n"  // |0|1|2|3|
    "rotr      $t1, $t1, 16                \n"  // |4|5|6|7|
    "rotr      $t2, $t2, 16                \n"  // |8|9|10|11|
    "rotr      $t3, $t3, 16                \n"  // |12|13|14|15|
    "addiu     %[src], %[src], -16         \n"
    "addiu     $t4, $t4, -1                \n"
    "sw        $t3, 0(%[dst])              \n"  // |15|14|13|12|
    "sw        $t2, 4(%[dst])              \n"  // |11|10|9|8|
    "sw        $t1, 8(%[dst])              \n"  // |7|6|5|4|
    "sw        $t0, 12(%[dst])             \n"  // |3|2|1|0|
    "bgtz      $t4, 1b                     \n"
    " addiu    %[dst], %[dst], 16          \n"
    "beqz      $t5, 3f                     \n"
    " nop                                  \n"

   "2:                                     \n"
    "lbu       $t0, -1(%[src])             \n"
    "addiu     $t5, $t5, -1                \n"
    "addiu     %[src], %[src], -1          \n"
    "sb        $t0, 0(%[dst])              \n"
    "bgez      $t5, 2b                     \n"
    " addiu    %[dst], %[dst], 1           \n"

   "3:                                     \n"
    ".set pop                              \n"
      : [src] "+r" (src), [dst] "+r" (dst)
      : [width] "r" (width)
      : "t0", "t1", "t2", "t3", "t4", "t5"
  );
}

void RGB24ToARGBRow_DSPR2(const uint8* src_rgb24, uint8* dst_argb, int width) {
  int x;
  uint32 tmp_mask = 0xff;
  uint32 tmp_t1;
  for (x = 0; x < width; ++x) {
    __asm__ __volatile__ (
     ".set push                                                  \n"
     ".set noreorder                                             \n"
     "lw              %[tmp_t1],    0(%[src_rgb24])              \n"
     "addiu           %[dst_argb],  %[dst_argb],     4           \n"
     "addiu           %[src_rgb24], %[src_rgb24],    3           \n"
     "ins             %[tmp_t1],    %[tmp_mask],     24,    8    \n"
     "sw              %[tmp_t1],    -4(%[dst_argb])              \n"
     ".set pop                                                   \n"
     :[src_rgb24] "+r" (src_rgb24), [dst_argb] "+r" (dst_argb),
      [tmp_t1] "=&r" (tmp_t1)
     :[tmp_mask] "r" (tmp_mask)
     : "memory"
    );
  }
}

void RAWToARGBRow_DSPR2(const uint8* src_raw, uint8* dst_argb, int width) {
  int x;
  uint32 tmp_mask = 0xff;
  uint32 tmp_t1, tmp_t2;
  for (x = 0; x < width; ++x) {
    __asm__ __volatile__ (
     ".set push                                               \n"
     ".set noreorder                                          \n"
     "lw                %[tmp_t1],   0(%[src_raw])            \n"
     "addiu             %[dst_argb], %[dst_argb],      4      \n"
     "addiu             %[src_raw],  %[src_raw],       3      \n"
     "srl               %[tmp_t2],   %[tmp_t1],        16     \n"
     "ins               %[tmp_t1],   %[tmp_mask],      24, 8  \n"
     "ins               %[tmp_t1],   %[tmp_t1],        16, 8  \n"
     "ins               %[tmp_t1],   %[tmp_t2],        0,  8  \n"
     "sw                %[tmp_t1],   -4(%[dst_argb])          \n"
     ".set pop                                                \n"
     :[src_raw] "+r" (src_raw), [dst_argb] "+r" (dst_argb),
      [tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2)
     :[tmp_mask] "r" (tmp_mask)
    );
  }
}

void RGB565ToARGBRow_DSPR2(const uint8* src_rgb565, uint8* dst_argb,
                           int width) {
  int x;
  uint32 tmp_mask = 0xff;
  uint32 tmp_t1, tmp_t2, tmp_t3;
  for (x = 0; x < width; ++x) {
    __asm__ __volatile__ (
     ".set push                                                   \n"
     ".set noreorder                                              \n"
     "lhu               %[tmp_t1],     0(%[src_rgb565])           \n"
     "addiu             %[dst_argb],   %[dst_argb],      4        \n"
     "addiu             %[src_rgb565], %[src_rgb565],    2        \n"
     "sll               %[tmp_t2],     %[tmp_t1],        8        \n"
     "ins               %[tmp_t2],     %[tmp_mask],      24,8     \n"
     "ins               %[tmp_t2],     %[tmp_t1],        3, 16    \n"
     "ins               %[tmp_t2],     %[tmp_t1],        5, 11    \n"
     "srl               %[tmp_t3],     %[tmp_t1],        9        \n"
     "ins               %[tmp_t2],     %[tmp_t3],        8, 2     \n"
     "ins               %[tmp_t2],     %[tmp_t1],        3, 5     \n"
     "srl               %[tmp_t3],     %[tmp_t1],        2        \n"
     "ins               %[tmp_t2],     %[tmp_t3],        0, 3     \n"
     "sw                %[tmp_t2],     -4(%[dst_argb])            \n"
     ".set pop                                                    \n"
     :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
      [tmp_t3] "=&r" (tmp_t3), [src_rgb565] "+r" (src_rgb565),
      [dst_argb] "+r" (dst_argb)
    :[tmp_mask] "r" (tmp_mask)
    );
  }
}

void ARGB1555ToARGBRow_DSPR2(const uint8* src_argb1555, uint8* dst_argb,
                             int width) {
  int x;
  uint32 tmp_t1, tmp_t2, tmp_t3;
  for (x = 0; x < width; ++x) {
    __asm__ __volatile__ (
     ".set push                                                   \n"
     ".set noreorder                                              \n"
     "lh                %[tmp_t1],       0(%[src_argb1555])       \n"
     "addiu             %[dst_argb],     %[dst_argb],      4      \n"
     "addiu             %[src_argb1555], %[src_argb1555],  2      \n"
     "sll               %[tmp_t2],       %[tmp_t1],        9      \n"
     "ins               %[tmp_t2],       %[tmp_t1],        4, 15  \n"
     "ins               %[tmp_t2],       %[tmp_t1],        6, 10  \n"
     "srl               %[tmp_t3],       %[tmp_t1],        7      \n"
     "ins               %[tmp_t2],       %[tmp_t3],        8, 3   \n"
     "ins               %[tmp_t2],       %[tmp_t1],        3, 5   \n"
     "srl               %[tmp_t3],       %[tmp_t1],        2      \n"
     "ins               %[tmp_t2],       %[tmp_t3],        0, 3   \n"
     "sw                %[tmp_t2],       -4(%[dst_argb])          \n"
     ".set pop                                                    \n"
     :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
      [tmp_t3] "=&r" (tmp_t3), [src_argb1555] "+r" (src_argb1555),
      [dst_argb] "+r" (dst_argb)
     :
    );
  }
}

void ARGB4444ToARGBRow_DSPR2(const uint8* src_argb4444, uint8* dst_argb,
                             int width) {
  int x;
  uint32 tmp_t1;
  for (x = 0; x < width; ++x) {
    __asm__ __volatile__ (
     ".set push                                                    \n"
     ".set noreorder                                               \n"
     "lh                %[tmp_t1],       0(%[src_argb4444])        \n"
     "addiu             %[dst_argb],     %[dst_argb],       4      \n"
     "addiu             %[src_argb4444], %[src_argb4444],   2      \n"
     "ins               %[tmp_t1],       %[tmp_t1],         16, 16 \n"
     "ins               %[tmp_t1],       %[tmp_t1],         12, 16 \n"
     "ins               %[tmp_t1],       %[tmp_t1],         8,  12 \n"
     "ins               %[tmp_t1],       %[tmp_t1],         4,  8  \n"
     "sw                %[tmp_t1],       -4(%[dst_argb])           \n"
     ".set pop                                                     \n"
     :[src_argb4444] "+r" (src_argb4444), [dst_argb] "+r" (dst_argb),
      [tmp_t1] "=&r" (tmp_t1)
    );
  }
}
#endif  // defined(LIBYUV_MIPS)

// DSPR2 functions
#if !defined(LIBYUV_DISABLE_MIPS) && defined(LIBYUV_DSPR2) \
    && (__mips_isa_rev < 6)

void SplitUVRow_DSPR2(const uint8* src_uv, uint8* dst_u, uint8* dst_v,
                      int width) {
  __asm__ __volatile__ (
    ".set push                                     \n"
    ".set noreorder                                \n"
    "srl             $t4, %[width], 4              \n"  // multiplies of 16
    "blez            $t4, 2f                       \n"
    " andi           %[width], %[width], 0xf       \n"  // residual

  "1:                                              \n"
    "addiu           $t4, $t4, -1                  \n"
    "lw              $t0, 0(%[src_uv])             \n"  // V1 | U1 | V0 | U0
    "lw              $t1, 4(%[src_uv])             \n"  // V3 | U3 | V2 | U2
    "lw              $t2, 8(%[src_uv])             \n"  // V5 | U5 | V4 | U4
    "lw              $t3, 12(%[src_uv])            \n"  // V7 | U7 | V6 | U6
    "lw              $t5, 16(%[src_uv])            \n"  // V9 | U9 | V8 | U8
    "lw              $t6, 20(%[src_uv])            \n"  // V11 | U11 | V10 | U10
    "lw              $t7, 24(%[src_uv])            \n"  // V13 | U13 | V12 | U12
    "lw              $t8, 28(%[src_uv])            \n"  // V15 | U15 | V14 | U14
    "addiu           %[src_uv], %[src_uv], 32      \n"
    "precrq.qb.ph    $t9, $t1, $t0                 \n"  // V3 | V2 | V1 | V0
    "precr.qb.ph     $t0, $t1, $t0                 \n"  // U3 | U2 | U1 | U0
    "precrq.qb.ph    $t1, $t3, $t2                 \n"  // V7 | V6 | V5 | V4
    "precr.qb.ph     $t2, $t3, $t2                 \n"  // U7 | U6 | U5 | U4
    "precrq.qb.ph    $t3, $t6, $t5                 \n"  // V11 | V10 | V9 | V8
    "precr.qb.ph     $t5, $t6, $t5                 \n"  // U11 | U10 | U9 | U8
    "precrq.qb.ph    $t6, $t8, $t7                 \n"  // V15 | V14 | V13 | V12
    "precr.qb.ph     $t7, $t8, $t7                 \n"  // U15 | U14 | U13 | U12
    "sw              $t9, 0(%[dst_v])              \n"
    "sw              $t0, 0(%[dst_u])              \n"
    "sw              $t1, 4(%[dst_v])              \n"
    "sw              $t2, 4(%[dst_u])              \n"
    "sw              $t3, 8(%[dst_v])              \n"
    "sw              $t5, 8(%[dst_u])              \n"
    "sw              $t6, 12(%[dst_v])             \n"
    "sw              $t7, 12(%[dst_u])             \n"
    "addiu           %[dst_v], %[dst_v], 16        \n"
    "bgtz            $t4, 1b                       \n"
    " addiu          %[dst_u], %[dst_u], 16        \n"

    "beqz            %[width], 3f                  \n"
    " nop                                          \n"

  "2:                                              \n"
    "lbu             $t0, 0(%[src_uv])             \n"
    "lbu             $t1, 1(%[src_uv])             \n"
    "addiu           %[src_uv], %[src_uv], 2       \n"
    "addiu           %[width], %[width], -1        \n"
    "sb              $t0, 0(%[dst_u])              \n"
    "sb              $t1, 0(%[dst_v])              \n"
    "addiu           %[dst_u], %[dst_u], 1         \n"
    "bgtz            %[width], 2b                  \n"
    " addiu          %[dst_v], %[dst_v], 1         \n"

  "3:                                              \n"
    ".set pop                                      \n"
     : [src_uv] "+r" (src_uv),
       [width] "+r" (width),
       [dst_u] "+r" (dst_u),
       [dst_v] "+r" (dst_v)
     :
     : "t0", "t1", "t2", "t3",
     "t4", "t5", "t6", "t7", "t8", "t9"
  );
}

void MirrorUVRow_DSPR2(const uint8* src_uv, uint8* dst_u, uint8* dst_v,
                       int width) {
  int x;
  int y;
  __asm__ __volatile__ (
    ".set push                                    \n"
    ".set noreorder                               \n"

    "addu            $t4, %[width], %[width]      \n"
    "srl             %[x], %[width], 4            \n"
    "andi            %[y], %[width], 0xf          \n"
    "blez            %[x], 2f                     \n"
    " addu           %[src_uv], %[src_uv], $t4    \n"

   "1:                                            \n"
    "lw              $t0, -32(%[src_uv])          \n"  // |3|2|1|0|
    "lw              $t1, -28(%[src_uv])          \n"  // |7|6|5|4|
    "lw              $t2, -24(%[src_uv])          \n"  // |11|10|9|8|
    "lw              $t3, -20(%[src_uv])          \n"  // |15|14|13|12|
    "lw              $t4, -16(%[src_uv])          \n"  // |19|18|17|16|
    "lw              $t6, -12(%[src_uv])          \n"  // |23|22|21|20|
    "lw              $t7, -8(%[src_uv])           \n"  // |27|26|25|24|
    "lw              $t8, -4(%[src_uv])           \n"  // |31|30|29|28|

    "rotr            $t0, $t0, 16                 \n"  // |1|0|3|2|
    "rotr            $t1, $t1, 16                 \n"  // |5|4|7|6|
    "rotr            $t2, $t2, 16                 \n"  // |9|8|11|10|
    "rotr            $t3, $t3, 16                 \n"  // |13|12|15|14|
    "rotr            $t4, $t4, 16                 \n"  // |17|16|19|18|
    "rotr            $t6, $t6, 16                 \n"  // |21|20|23|22|
    "rotr            $t7, $t7, 16                 \n"  // |25|24|27|26|
    "rotr            $t8, $t8, 16                 \n"  // |29|28|31|30|
    "precr.qb.ph     $t9, $t0, $t1                \n"  // |0|2|4|6|
    "precrq.qb.ph    $t5, $t0, $t1                \n"  // |1|3|5|7|
    "precr.qb.ph     $t0, $t2, $t3                \n"  // |8|10|12|14|
    "precrq.qb.ph    $t1, $t2, $t3                \n"  // |9|11|13|15|
    "precr.qb.ph     $t2, $t4, $t6                \n"  // |16|18|20|22|
    "precrq.qb.ph    $t3, $t4, $t6                \n"  // |17|19|21|23|
    "precr.qb.ph     $t4, $t7, $t8                \n"  // |24|26|28|30|
    "precrq.qb.ph    $t6, $t7, $t8                \n"  // |25|27|29|31|
    "addiu           %[src_uv], %[src_uv], -32    \n"
    "addiu           %[x], %[x], -1               \n"
    "swr             $t4, 0(%[dst_u])             \n"
    "swl             $t4, 3(%[dst_u])             \n"  // |30|28|26|24|
    "swr             $t6, 0(%[dst_v])             \n"
    "swl             $t6, 3(%[dst_v])             \n"  // |31|29|27|25|
    "swr             $t2, 4(%[dst_u])             \n"
    "swl             $t2, 7(%[dst_u])             \n"  // |22|20|18|16|
    "swr             $t3, 4(%[dst_v])             \n"
    "swl             $t3, 7(%[dst_v])             \n"  // |23|21|19|17|
    "swr             $t0, 8(%[dst_u])             \n"
    "swl             $t0, 11(%[dst_u])            \n"  // |14|12|10|8|
    "swr             $t1, 8(%[dst_v])             \n"
    "swl             $t1, 11(%[dst_v])            \n"  // |15|13|11|9|
    "swr             $t9, 12(%[dst_u])            \n"
    "swl             $t9, 15(%[dst_u])            \n"  // |6|4|2|0|
    "swr             $t5, 12(%[dst_v])            \n"
    "swl             $t5, 15(%[dst_v])            \n"  // |7|5|3|1|
    "addiu           %[dst_v], %[dst_v], 16       \n"
    "bgtz            %[x], 1b                     \n"
    " addiu          %[dst_u], %[dst_u], 16       \n"
    "beqz            %[y], 3f                     \n"
    " nop                                         \n"
    "b               2f                           \n"
    " nop                                         \n"

   "2:                                            \n"
    "lbu             $t0, -2(%[src_uv])           \n"
    "lbu             $t1, -1(%[src_uv])           \n"
    "addiu           %[src_uv], %[src_uv], -2     \n"
    "addiu           %[y], %[y], -1               \n"
    "sb              $t0, 0(%[dst_u])             \n"
    "sb              $t1, 0(%[dst_v])             \n"
    "addiu           %[dst_u], %[dst_u], 1        \n"
    "bgtz            %[y], 2b                     \n"
    " addiu          %[dst_v], %[dst_v], 1        \n"

   "3:                                            \n"
    ".set pop                                     \n"
      : [src_uv] "+r" (src_uv),
        [dst_u] "+r" (dst_u),
        [dst_v] "+r" (dst_v),
        [x] "=&r" (x),
        [y] "=&r" (y)
      : [width] "r" (width)
      : "t0", "t1", "t2", "t3", "t4",
      "t5", "t7", "t8", "t9"
  );
}

// Bilinear filter 8x2 -> 8x1
void InterpolateRow_DSPR2(uint8* dst_ptr, const uint8* src_ptr,
                          ptrdiff_t src_stride, int dst_width,
                          int source_y_fraction) {
    int y0_fraction = 256 - source_y_fraction;
    const uint8* src_ptr1 = src_ptr + src_stride;

  __asm__ __volatile__ (
     ".set push                                           \n"
     ".set noreorder                                      \n"

     "replv.ph          $t0, %[y0_fraction]               \n"
     "replv.ph          $t1, %[source_y_fraction]         \n"

   "1:                                                    \n"
     "lw                $t2, 0(%[src_ptr])                \n"
     "lw                $t3, 0(%[src_ptr1])               \n"
     "lw                $t4, 4(%[src_ptr])                \n"
     "lw                $t5, 4(%[src_ptr1])               \n"
     "muleu_s.ph.qbl    $t6, $t2, $t0                     \n"
     "muleu_s.ph.qbr    $t7, $t2, $t0                     \n"
     "muleu_s.ph.qbl    $t8, $t3, $t1                     \n"
     "muleu_s.ph.qbr    $t9, $t3, $t1                     \n"
     "muleu_s.ph.qbl    $t2, $t4, $t0                     \n"
     "muleu_s.ph.qbr    $t3, $t4, $t0                     \n"
     "muleu_s.ph.qbl    $t4, $t5, $t1                     \n"
     "muleu_s.ph.qbr    $t5, $t5, $t1                     \n"
     "addq.ph           $t6, $t6, $t8                     \n"
     "addq.ph           $t7, $t7, $t9                     \n"
     "addq.ph           $t2, $t2, $t4                     \n"
     "addq.ph           $t3, $t3, $t5                     \n"
     "shra_r.ph         $t6, $t6, 8                       \n"
     "shra_r.ph         $t7, $t7, 8                       \n"
     "shra_r.ph         $t2, $t2, 8                       \n"
     "shra_r.ph         $t3, $t3, 8                       \n"
     "precr.qb.ph       $t6, $t6, $t7                     \n"
     "precr.qb.ph       $t2, $t2, $t3                     \n"
     "addiu             %[src_ptr], %[src_ptr], 8         \n"
     "addiu             %[src_ptr1], %[src_ptr1], 8       \n"
     "addiu             %[dst_width], %[dst_width], -8    \n"
     "sw                $t6, 0(%[dst_ptr])                \n"
     "sw                $t2, 4(%[dst_ptr])                \n"
     "bgtz              %[dst_width], 1b                  \n"
     " addiu            %[dst_ptr], %[dst_ptr], 8         \n"

     ".set pop                                            \n"
  : [dst_ptr] "+r" (dst_ptr),
    [src_ptr1] "+r" (src_ptr1),
    [src_ptr] "+r" (src_ptr),
    [dst_width] "+r" (dst_width)
  : [source_y_fraction] "r" (source_y_fraction),
    [y0_fraction] "r" (y0_fraction),
    [src_stride] "r" (src_stride)
  : "t0", "t1", "t2", "t3", "t4", "t5",
    "t6", "t7", "t8", "t9"
  );
}

void BGRAToUVRow_DSPR2(const uint8* src_rgb0,
                       int src_stride_rgb,
                       uint8* dst_u,
                       uint8* dst_v, int width){
  const uint8* src_rgb1 = src_rgb0 + src_stride_rgb;
  int x;
  int const1 = 0xffda0000;
  int const2 = 0x0070ffb6;
  int const3 = 0x00700000;
  int const4 = 0xffeeffa2;
  int const5 = 0x100;
  for (x = 0; x < width - 1; x += 2){
    int tmp_t1, tmp_t2, tmp_t3, tmp_t4, tmp_t5;
    int tmp_t6, tmp_t7, tmp_t8;
    __asm__ __volatile__ (
     ".set push                                                 \n"
     ".set noreorder                                            \n"
     "lw                %[tmp_t1],   0(%[src_rgb0])             \n"
     "lw                %[tmp_t2],   4(%[src_rgb0])             \n"
     "lw                %[tmp_t3],   0(%[src_rgb1])             \n"
     "lw                %[tmp_t4],   4(%[src_rgb1])             \n"
     "preceu.ph.qbr     %[tmp_t5],   %[tmp_t1]                  \n"
     "preceu.ph.qbl     %[tmp_t1],   %[tmp_t1]                  \n"
     "preceu.ph.qbr     %[tmp_t6],   %[tmp_t2]                  \n"
     "preceu.ph.qbl     %[tmp_t2],   %[tmp_t2]                  \n"
     "preceu.ph.qbr     %[tmp_t7],   %[tmp_t3]                  \n"
     "preceu.ph.qbl     %[tmp_t3],   %[tmp_t3]                  \n"
     "preceu.ph.qbr     %[tmp_t8],   %[tmp_t4]                  \n"
     "preceu.ph.qbl     %[tmp_t4],   %[tmp_t4]                  \n"
     "addu.ph           %[tmp_t5],   %[tmp_t5],     %[tmp_t6]   \n"
     "addu.ph           %[tmp_t7],   %[tmp_t7],     %[tmp_t8]   \n"
     "addu.ph           %[tmp_t1],   %[tmp_t1],     %[tmp_t2]   \n"
     "addu.ph           %[tmp_t3],   %[tmp_t3],     %[tmp_t4]   \n"
     "addu.ph           %[tmp_t5],   %[tmp_t5],     %[tmp_t7]   \n"
     "addu.ph           %[tmp_t1],   %[tmp_t1],     %[tmp_t3]   \n"
     "shrl.ph           %[tmp_t5],   %[tmp_t5],     2           \n"
     "shrl.ph           %[tmp_t1],   %[tmp_t1],     2           \n"
     "mult              $ac0,        %[const5],     %[const5]   \n"
     "mult              $ac1,        %[const5],     %[const5]   \n"
     "dpaq_s.w.ph       $ac0,        %[tmp_t5],     %[const1]   \n"
     "dpaq_s.w.ph       $ac1,        %[tmp_t5],     %[const3]   \n"
     "dpaq_s.w.ph       $ac0,        %[tmp_t1],     %[const2]   \n"
     "dpaq_s.w.ph       $ac1,        %[tmp_t1],     %[const4]   \n"
     "extr_r.w          %[tmp_t7],   $ac0,          9           \n"
     "extr_r.w          %[tmp_t8],   $ac1,          9           \n"
     "addiu             %[dst_u],    %[dst_u],      1           \n"
     "addiu             %[dst_v],    %[dst_v],      1           \n"
     "addiu             %[src_rgb0], %[src_rgb0],   8           \n"
     "addiu             %[src_rgb1], %[src_rgb1],   8           \n"
     "sb                %[tmp_t7],   -1(%[dst_u])               \n"
     "sb                %[tmp_t8],   -1(%[dst_v])               \n"
     ".set pop                                                  \n"
      :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
       [tmp_t3] "=&r" (tmp_t3), [tmp_t4] "=&r" (tmp_t4),
       [tmp_t5] "=&r" (tmp_t5), [tmp_t6] "=&r" (tmp_t6),
       [tmp_t7] "=&r" (tmp_t7), [tmp_t8] "=&r" (tmp_t8),
       [src_rgb0] "+r" (src_rgb0), [src_rgb1] "+r" (src_rgb1),
       [dst_u] "+r" (dst_u), [dst_v] "+r" (dst_v)
      :[const1] "r" (const1), [const2] "r" (const2),
       [const3] "r" (const3), [const4] "r" (const4),
       [const5] "r" (const5)
      :"hi", "lo"
    );
  }
  if (width & 1){
    uint8 ab = (src_rgb0[3] + src_rgb1[3]) >> 1;
    uint8 ag = (src_rgb0[2] + src_rgb1[2]) >> 1;
    uint8 ar = (src_rgb0[1] + src_rgb1[1]) >> 1;
    dst_u[0] = (112 * ab - 74 * ag - 38 * ar + 0x8080) >> 8;
    dst_v[0] = (112 * ar - 94 * ag - 18 * ab + 0x8080) >> 8;
  }
}

void BGRAToYRow_DSPR2(const uint8* src_argb0, uint8* dst_y, int width)
{
  int x;
  int const1 = 0x00420000;
  int const2 = 0x00190081;
  int const5 = 0x40;
  for (x = 0; x < width; x+=4){
    int tmp_t1, tmp_t2, tmp_t3, tmp_t4, tmp_t5;
    int tmp_t6, tmp_t7, tmp_t8;
    __asm__ __volatile__ (
     ".set push                                                \n"
     ".set noreorder                                           \n"
     "lw                %[tmp_t1],   0(%[src_argb0])           \n"
     "lw                %[tmp_t2],   4(%[src_argb0])           \n"
     "lw                %[tmp_t3],   8(%[src_argb0])           \n"
     "lw                %[tmp_t4],   12(%[src_argb0])          \n"
     "preceu.ph.qbr     %[tmp_t5],   %[tmp_t1]                 \n"
     "preceu.ph.qbl     %[tmp_t1],   %[tmp_t1]                 \n"
     "preceu.ph.qbr     %[tmp_t6],   %[tmp_t2]                 \n"
     "preceu.ph.qbl     %[tmp_t2],   %[tmp_t2]                 \n"
     "preceu.ph.qbr     %[tmp_t7],   %[tmp_t3]                 \n"
     "preceu.ph.qbl     %[tmp_t3],   %[tmp_t3]                 \n"
     "preceu.ph.qbr     %[tmp_t8],   %[tmp_t4]                 \n"
     "preceu.ph.qbl     %[tmp_t4],   %[tmp_t4]                 \n"
     "mult              $ac0,        %[const5],     %[const5]  \n"
     "mult              $ac1,        %[const5],     %[const5]  \n"
     "mult              $ac2,        %[const5],     %[const5]  \n"
     "mult              $ac3,        %[const5],     %[const5]  \n"
     "dpa.w.ph          $ac0,        %[tmp_t5],     %[const1]  \n"
     "dpa.w.ph          $ac1,        %[tmp_t6],     %[const1]  \n"
     "dpa.w.ph          $ac2,        %[tmp_t7],     %[const1]  \n"
     "dpa.w.ph          $ac3,        %[tmp_t8],     %[const1]  \n"
     "dpa.w.ph          $ac0,        %[tmp_t1],     %[const2]  \n"
     "dpa.w.ph          $ac1,        %[tmp_t2],     %[const2]  \n"
     "dpa.w.ph          $ac2,        %[tmp_t3],     %[const2]  \n"
     "dpa.w.ph          $ac3,        %[tmp_t4],     %[const2]  \n"
     "extr_r.w          %[tmp_t1],   $ac0,          8          \n"
     "extr_r.w          %[tmp_t2],   $ac1,          8          \n"
     "extr_r.w          %[tmp_t3],   $ac2,          8          \n"
     "extr_r.w          %[tmp_t4],   $ac3,          8          \n"
     "addiu             %[src_argb0],%[src_argb0],  16         \n"
     "addiu             %[dst_y],    %[dst_y],      4          \n"
     "sb                %[tmp_t1],   -4(%[dst_y])              \n"
     "sb                %[tmp_t2],   -3(%[dst_y])              \n"
     "sb                %[tmp_t3],   -2(%[dst_y])              \n"
     "sb                %[tmp_t4],   -1(%[dst_y])              \n"
     ".set pop                                                 \n"
      :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
       [tmp_t3] "=&r" (tmp_t3), [tmp_t4] "=&r" (tmp_t4),
       [tmp_t5] "=&r" (tmp_t5), [tmp_t6] "=&r" (tmp_t6),
       [tmp_t7] "=&r" (tmp_t7), [tmp_t8] "=&r" (tmp_t8),
       [src_argb0] "+r" (src_argb0), [dst_y] "+r" (dst_y)
      :[const1] "r" (const1), [const2] "r" (const2),
       [const5] "r" (const5)
      :"hi", "lo"
    );
  }
  for (x = 0; x < (width & 3); ++x){
    uint8 ar = src_argb0[1];
    uint8 ag = src_argb0[2];
    uint8 ab = src_argb0[3];
    dst_y[0] = (66 * ar + 129 * ag +  25 * ab + 0x1080) >> 8;
    src_argb0 += 4;
    dst_y += 1;
  }
}

void ABGRToUVRow_DSPR2(const uint8* src_rgb0, int src_stride_rgb,
                       uint8* dst_u, uint8* dst_v, int width){
  const uint8* src_rgb1 = src_rgb0 + src_stride_rgb;
  int x;
  int const1 = 0xffb6ffda;
  int const2 = 0x00000070;
  int const3 = 0xffa20070;
  int const4 = 0x0000ffee;
  int const5 = 0x100;

  for (x = 0; x < width - 1; x += 2){
    int tmp_t1, tmp_t2, tmp_t3, tmp_t4, tmp_t5;
    int tmp_t6, tmp_t7, tmp_t8;
    __asm__ __volatile__ (
     ".set push                                                \n"
     ".set noreorder                                           \n"
     "lw                %[tmp_t1],   0(%[src_rgb0])            \n"
     "lw                %[tmp_t2],   4(%[src_rgb0])            \n"
     "lw                %[tmp_t3],   0(%[src_rgb1])            \n"
     "lw                %[tmp_t4],   4(%[src_rgb1])            \n"
     "preceu.ph.qbr     %[tmp_t5],   %[tmp_t1]                 \n"
     "preceu.ph.qbl     %[tmp_t1],   %[tmp_t1]                 \n"
     "preceu.ph.qbr     %[tmp_t6],   %[tmp_t2]                 \n"
     "preceu.ph.qbl     %[tmp_t2],   %[tmp_t2]                 \n"
     "preceu.ph.qbr     %[tmp_t7],   %[tmp_t3]                 \n"
     "preceu.ph.qbl     %[tmp_t3],   %[tmp_t3]                 \n"
     "preceu.ph.qbr     %[tmp_t8],   %[tmp_t4]                 \n"
     "preceu.ph.qbl     %[tmp_t4],   %[tmp_t4]                 \n"
     "addu.ph           %[tmp_t5],   %[tmp_t5],     %[tmp_t6]  \n"
     "addu.ph           %[tmp_t7],   %[tmp_t7],     %[tmp_t8]  \n"
     "addu.ph           %[tmp_t1],   %[tmp_t1],     %[tmp_t2]  \n"
     "addu.ph           %[tmp_t3],   %[tmp_t3],     %[tmp_t4]  \n"
     "addu.ph           %[tmp_t5],   %[tmp_t5],     %[tmp_t7]  \n"
     "addu.ph           %[tmp_t1],   %[tmp_t1],     %[tmp_t3]  \n"
     "shrl.ph           %[tmp_t5],   %[tmp_t5],     2          \n"
     "shrl.ph           %[tmp_t1],   %[tmp_t1],     2          \n"
     "mult              $ac0,        %[const5],     %[const5]  \n"
     "mult              $ac1,        %[const5],     %[const5]  \n"
     "dpaq_s.w.ph       $ac0,        %[tmp_t5],     %[const1]  \n"
     "dpaq_s.w.ph       $ac1,        %[tmp_t5],     %[const3]  \n"
     "dpaq_s.w.ph       $ac0,        %[tmp_t1],     %[const2]  \n"
     "dpaq_s.w.ph       $ac1,        %[tmp_t1],     %[const4]  \n"
     "extr_r.w          %[tmp_t7],   $ac0,          9          \n"
     "extr_r.w          %[tmp_t8],   $ac1,          9          \n"
     "addiu             %[dst_u],    %[dst_u],      1          \n"
     "addiu             %[dst_v],    %[dst_v],      1          \n"
     "addiu             %[src_rgb0], %[src_rgb0],   8          \n"
     "addiu             %[src_rgb1], %[src_rgb1],   8          \n"
     "sb                %[tmp_t7],   -1(%[dst_u])              \n"
     "sb                %[tmp_t8],   -1(%[dst_v])              \n"
     ".set pop                                                 \n"
      :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
       [tmp_t3] "=&r" (tmp_t3), [tmp_t4] "=&r" (tmp_t4),
       [tmp_t5] "=&r" (tmp_t5), [tmp_t6] "=&r" (tmp_t6),
       [tmp_t7] "=&r" (tmp_t7), [tmp_t8] "=&r" (tmp_t8),
       [src_rgb0] "+r" (src_rgb0), [src_rgb1] "+r" (src_rgb1),
       [dst_u] "+r" (dst_u), [dst_v] "+r" (dst_v)
      :[const1] "r" (const1), [const2] "r" (const2),
       [const3] "r" (const3), [const4] "r" (const4),
       [const5] "r" (const5)
      :"hi", "lo"
    );
  }
  if (width & 1){
    uint8 ab = (src_rgb0[2] + src_rgb1[2]) >> 1;
    uint8 ag = (src_rgb0[1] + src_rgb1[1]) >> 1;
    uint8 ar = (src_rgb0[0] + src_rgb1[0]) >> 1;
    dst_u[0] = (112 * ab - 74 * ag - 38 * ar + 0x8080) >> 8;
    dst_v[0] = (112 * ar - 94 * ag - 18 * ab + 0x8080) >> 8;
  }
}

void ARGBToYRow_DSPR2(const uint8* src_argb0, uint8* dst_y, int width){
  int x;
  int const1 = 0x00810019;
  int const2 = 0x00000042;
  int const5 = 0x40;
  for (x = 0; x < width; x+=4){
    int tmp_t1, tmp_t2, tmp_t3, tmp_t4, tmp_t5;
    int tmp_t6, tmp_t7, tmp_t8;
    __asm__ __volatile__ (
     ".set push                                                \n"
     ".set noreorder                                           \n"
     "lw                %[tmp_t1],   0(%[src_argb0])           \n"
     "lw                %[tmp_t2],   4(%[src_argb0])           \n"
     "lw                %[tmp_t3],   8(%[src_argb0])           \n"
     "lw                %[tmp_t4],   12(%[src_argb0])          \n"
     "preceu.ph.qbr     %[tmp_t5],   %[tmp_t1]                 \n"
     "preceu.ph.qbl     %[tmp_t1],   %[tmp_t1]                 \n"
     "preceu.ph.qbr     %[tmp_t6],   %[tmp_t2]                 \n"
     "preceu.ph.qbl     %[tmp_t2],   %[tmp_t2]                 \n"
     "preceu.ph.qbr     %[tmp_t7],   %[tmp_t3]                 \n"
     "preceu.ph.qbl     %[tmp_t3],   %[tmp_t3]                 \n"
     "preceu.ph.qbr     %[tmp_t8],   %[tmp_t4]                 \n"
     "preceu.ph.qbl     %[tmp_t4],   %[tmp_t4]                 \n"
     "mult              $ac0,        %[const5],     %[const5]  \n"
     "mult              $ac1,        %[const5],     %[const5]  \n"
     "mult              $ac2,        %[const5],     %[const5]  \n"
     "mult              $ac3,        %[const5],     %[const5]  \n"
     "dpa.w.ph          $ac0,        %[tmp_t5],     %[const1]  \n"
     "dpa.w.ph          $ac1,        %[tmp_t6],     %[const1]  \n"
     "dpa.w.ph          $ac2,        %[tmp_t7],     %[const1]  \n"
     "dpa.w.ph          $ac3,        %[tmp_t8],     %[const1]  \n"
     "dpa.w.ph          $ac0,        %[tmp_t1],     %[const2]  \n"
     "dpa.w.ph          $ac1,        %[tmp_t2],     %[const2]  \n"
     "dpa.w.ph          $ac2,        %[tmp_t3],     %[const2]  \n"
     "dpa.w.ph          $ac3,        %[tmp_t4],     %[const2]  \n"
     "extr_r.w          %[tmp_t1],   $ac0,          8          \n"
     "extr_r.w          %[tmp_t2],   $ac1,          8          \n"
     "extr_r.w          %[tmp_t3],   $ac2,          8          \n"
     "extr_r.w          %[tmp_t4],   $ac3,          8          \n"
     "addiu             %[dst_y],    %[dst_y],      4          \n"
     "addiu             %[src_argb0],%[src_argb0],  16         \n"
     "sb                %[tmp_t1],   -4(%[dst_y])              \n"
     "sb                %[tmp_t2],   -3(%[dst_y])              \n"
     "sb                %[tmp_t3],   -2(%[dst_y])              \n"
     "sb                %[tmp_t4],   -1(%[dst_y])              \n"
     ".set pop                                                 \n"
      :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
       [tmp_t3] "=&r" (tmp_t3), [tmp_t4] "=&r" (tmp_t4),
       [tmp_t5] "=&r" (tmp_t5), [tmp_t6] "=&r" (tmp_t6),
       [tmp_t7] "=&r" (tmp_t7), [tmp_t8] "=&r" (tmp_t8),
       [src_argb0] "+r" (src_argb0), [dst_y] "+r" (dst_y)
      :[const1] "r" (const1), [const2] "r" (const2),
       [const5] "r" (const5)
      :"hi", "lo"
    );
  }
  for (x = 0; x < (width & 3); ++x){
    uint8 ar = src_argb0[2];
    uint8 ag = src_argb0[1];
    uint8 ab = src_argb0[0];
    dst_y[0] = (66 * ar + 129 * ag +  25 * ab + 0x1080) >> 8;
    src_argb0 += 4;
    dst_y += 1;
  }
}

void ABGRToYRow_DSPR2(const uint8* src_argb0, uint8* dst_y, int width){
  int x;
  int const1 = 0x00810042;
  int const2 = 0x00000019;
  int const5 = 0x40;
  for (x = 0; x < width; x+=4){
    int tmp_t1, tmp_t2, tmp_t3, tmp_t4, tmp_t5;
    int tmp_t6, tmp_t7, tmp_t8;
    __asm__ __volatile__ (
     ".set push                                                \n"
     ".set noreorder                                           \n"
     "lw                %[tmp_t1],   0(%[src_argb0])           \n"
     "lw                %[tmp_t2],   4(%[src_argb0])           \n"
     "lw                %[tmp_t3],   8(%[src_argb0])           \n"
     "lw                %[tmp_t4],   12(%[src_argb0])          \n"
     "preceu.ph.qbr     %[tmp_t5],   %[tmp_t1]                 \n"
     "preceu.ph.qbl     %[tmp_t1],   %[tmp_t1]                 \n"
     "preceu.ph.qbr     %[tmp_t6],   %[tmp_t2]                 \n"
     "preceu.ph.qbl     %[tmp_t2],   %[tmp_t2]                 \n"
     "preceu.ph.qbr     %[tmp_t7],   %[tmp_t3]                 \n"
     "preceu.ph.qbl     %[tmp_t3],   %[tmp_t3]                 \n"
     "preceu.ph.qbr     %[tmp_t8],   %[tmp_t4]                 \n"
     "preceu.ph.qbl     %[tmp_t4],   %[tmp_t4]                 \n"
     "mult              $ac0,        %[const5],     %[const5]  \n"
     "mult              $ac1,        %[const5],     %[const5]  \n"
     "mult              $ac2,        %[const5],     %[const5]  \n"
     "mult              $ac3,        %[const5],     %[const5]  \n"
     "dpa.w.ph          $ac0,        %[tmp_t5],     %[const1]  \n"
     "dpa.w.ph          $ac1,        %[tmp_t6],     %[const1]  \n"
     "dpa.w.ph          $ac2,        %[tmp_t7],     %[const1]  \n"
     "dpa.w.ph          $ac3,        %[tmp_t8],     %[const1]  \n"
     "dpa.w.ph          $ac0,        %[tmp_t1],     %[const2]  \n"
     "dpa.w.ph          $ac1,        %[tmp_t2],     %[const2]  \n"
     "dpa.w.ph          $ac2,        %[tmp_t3],     %[const2]  \n"
     "dpa.w.ph          $ac3,        %[tmp_t4],     %[const2]  \n"
     "extr_r.w          %[tmp_t1],   $ac0,          8          \n"
     "extr_r.w          %[tmp_t2],   $ac1,          8          \n"
     "extr_r.w          %[tmp_t3],   $ac2,          8          \n"
     "extr_r.w          %[tmp_t4],   $ac3,          8          \n"
     "addiu             %[src_argb0],%[src_argb0],  16         \n"
     "addiu             %[dst_y],    %[dst_y],      4          \n"
     "sb                %[tmp_t1],   -4(%[dst_y])              \n"
     "sb                %[tmp_t2],   -3(%[dst_y])              \n"
     "sb                %[tmp_t3],   -2(%[dst_y])              \n"
     "sb                %[tmp_t4],   -1(%[dst_y])              \n"
     ".set pop                                                 \n"
     :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
      [tmp_t3] "=&r" (tmp_t3), [tmp_t4] "=&r" (tmp_t4),
      [tmp_t5] "=&r" (tmp_t5), [tmp_t6] "=&r" (tmp_t6),
      [tmp_t7] "=&r" (tmp_t7), [tmp_t8] "=&r" (tmp_t8),
      [src_argb0] "+r" (src_argb0), [dst_y] "+r" (dst_y)
     :[const1] "r" (const1), [const2] "r" (const2),
      [const5] "r" (const5)
     :"hi", "lo"
    );
  }
  for (x = 0; x < (width & 3); ++x){
    uint8 ar = src_argb0[0];
    uint8 ag = src_argb0[1];
    uint8 ab = src_argb0[2];
    dst_y[0] = (66 * ar + 129 * ag +  25 * ab + 0x1080) >> 8;
    src_argb0 += 4;
    dst_y += 1;
  }
}

void RGBAToUVRow_DSPR2(const uint8* src_rgb0, int src_stride_rgb,
                       uint8* dst_u, uint8* dst_v, int width){
  const uint8* src_rgb1 = src_rgb0 + src_stride_rgb;
  int x;
  int const1 = 0xffb60070;
  int const2 = 0x0000ffda;
  int const3 = 0xffa2ffee;
  int const4 = 0x00000070;
  int const5 = 0x100;

  for (x = 0; x < width - 1; x += 2){
    int tmp_t1, tmp_t2, tmp_t3, tmp_t4, tmp_t5;
    int tmp_t6, tmp_t7, tmp_t8;
    __asm__ __volatile__ (
     ".set push                                                \n"
     ".set noreorder                                           \n"
     "ulw               %[tmp_t1],   0+1(%[src_rgb0])          \n"
     "ulw               %[tmp_t2],   4+1(%[src_rgb0])          \n"
     "ulw               %[tmp_t3],   0+1(%[src_rgb1])          \n"
     "ulw               %[tmp_t4],   4+1(%[src_rgb1])          \n"
     "preceu.ph.qbr     %[tmp_t5],   %[tmp_t1]                 \n"
     "preceu.ph.qbl     %[tmp_t1],   %[tmp_t1]                 \n"
     "preceu.ph.qbr     %[tmp_t6],   %[tmp_t2]                 \n"
     "preceu.ph.qbl     %[tmp_t2],   %[tmp_t2]                 \n"
     "preceu.ph.qbr     %[tmp_t7],   %[tmp_t3]                 \n"
     "preceu.ph.qbl     %[tmp_t3],   %[tmp_t3]                 \n"
     "preceu.ph.qbr     %[tmp_t8],   %[tmp_t4]                 \n"
     "preceu.ph.qbl     %[tmp_t4],   %[tmp_t4]                 \n"
     "addu.ph           %[tmp_t5],   %[tmp_t5],     %[tmp_t6]  \n"
     "addu.ph           %[tmp_t7],   %[tmp_t7],     %[tmp_t8]  \n"
     "addu.ph           %[tmp_t1],   %[tmp_t1],     %[tmp_t2]  \n"
     "addu.ph           %[tmp_t3],   %[tmp_t3],     %[tmp_t4]  \n"
     "addu.ph           %[tmp_t5],   %[tmp_t5],     %[tmp_t7]  \n"
     "addu.ph           %[tmp_t1],   %[tmp_t1],     %[tmp_t3]  \n"
     "shrl.ph           %[tmp_t5],   %[tmp_t5],     2          \n"
     "shrl.ph           %[tmp_t1],   %[tmp_t1],     2          \n"
     "mult              $ac0,        %[const5],     %[const5]  \n"
     "mult              $ac1,        %[const5],     %[const5]  \n"
     "dpaq_s.w.ph       $ac0,        %[tmp_t5],     %[const1]  \n"
     "dpaq_s.w.ph       $ac1,        %[tmp_t5],     %[const3]  \n"
     "dpaq_s.w.ph       $ac0,        %[tmp_t1],     %[const2]  \n"
     "dpaq_s.w.ph       $ac1,        %[tmp_t1],     %[const4]  \n"
     "extr_r.w          %[tmp_t7],   $ac0,          9          \n"
     "extr_r.w          %[tmp_t8],   $ac1,          9          \n"
     "addiu             %[src_rgb0], %[src_rgb0],   8          \n"
     "addiu             %[src_rgb1], %[src_rgb1],   8          \n"
     "addiu             %[dst_u],    %[dst_u],      1          \n"
     "addiu             %[dst_v],    %[dst_v],      1          \n"
     "sb                %[tmp_t7],   -1(%[dst_u])              \n"
     "sb                %[tmp_t8],   -1(%[dst_v])              \n"
     ".set pop                                                 \n"
      :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
       [tmp_t3] "=&r" (tmp_t3), [tmp_t4] "=&r" (tmp_t4),
       [tmp_t5] "=&r" (tmp_t5), [tmp_t6] "=&r" (tmp_t6),
       [tmp_t7] "=&r" (tmp_t7), [tmp_t8] "=&r" (tmp_t8),
       [src_rgb0] "+r" (src_rgb0), [src_rgb1] "+r" (src_rgb1),
       [dst_u] "+r" (dst_u), [dst_v] "+r" (dst_v)
      :[const1] "r" (const1), [const2] "r" (const2),
       [const3] "r" (const3), [const4] "r" (const4),
       [const5] "r" (const5)
      :"hi", "lo"
    );
  }
  if (width & 1){
    uint8 ab = (src_rgb0[1] + src_rgb1[1]) >> 1;
    uint8 ag = (src_rgb0[2] + src_rgb1[2]) >> 1;
    uint8 ar = (src_rgb0[3] + src_rgb1[3]) >> 1;
    dst_u[0] = (112 * ab - 74 * ag - 38 * ar + 0x8080) >> 8;
    dst_v[0] = (112 * ar - 94 * ag - 18 * ab + 0x8080) >> 8;
  }
}

void RGBAToYRow_DSPR2(const uint8* src_argb0, uint8* dst_y, int width){
  int x;
  int const1 = 0x00420081;
  int const2 = 0x00190000;
  int const5 = 0x40;
  for (x = 0; x < width; x+=4){
    int tmp_t1, tmp_t2, tmp_t3, tmp_t4, tmp_t5;
    int tmp_t6, tmp_t7, tmp_t8;
    __asm__ __volatile__ (
     ".set push                                                \n"
     ".set noreorder                                           \n"
     "lw                %[tmp_t1],   0(%[src_argb0])           \n"
     "lw                %[tmp_t2],   4(%[src_argb0])           \n"
     "lw                %[tmp_t3],   8(%[src_argb0])           \n"
     "lw                %[tmp_t4],   12(%[src_argb0])          \n"
     "preceu.ph.qbl     %[tmp_t5],   %[tmp_t1]                 \n"
     "preceu.ph.qbr     %[tmp_t1],   %[tmp_t1]                 \n"
     "preceu.ph.qbl     %[tmp_t6],   %[tmp_t2]                 \n"
     "preceu.ph.qbr     %[tmp_t2],   %[tmp_t2]                 \n"
     "preceu.ph.qbl     %[tmp_t7],   %[tmp_t3]                 \n"
     "preceu.ph.qbr     %[tmp_t3],   %[tmp_t3]                 \n"
     "preceu.ph.qbl     %[tmp_t8],   %[tmp_t4]                 \n"
     "preceu.ph.qbr     %[tmp_t4],   %[tmp_t4]                 \n"
     "mult              $ac0,        %[const5],     %[const5]  \n"
     "mult              $ac1,        %[const5],     %[const5]  \n"
     "mult              $ac2,        %[const5],     %[const5]  \n"
     "mult              $ac3,        %[const5],     %[const5]  \n"
     "dpa.w.ph          $ac0,        %[tmp_t5],     %[const1]  \n"
     "dpa.w.ph          $ac1,        %[tmp_t6],     %[const1]  \n"
     "dpa.w.ph          $ac2,        %[tmp_t7],     %[const1]  \n"
     "dpa.w.ph          $ac3,        %[tmp_t8],     %[const1]  \n"
     "dpa.w.ph          $ac0,        %[tmp_t1],     %[const2]  \n"
     "dpa.w.ph          $ac1,        %[tmp_t2],     %[const2]  \n"
     "dpa.w.ph          $ac2,        %[tmp_t3],     %[const2]  \n"
     "dpa.w.ph          $ac3,        %[tmp_t4],     %[const2]  \n"
     "extr_r.w          %[tmp_t1],   $ac0,          8          \n"
     "extr_r.w          %[tmp_t2],   $ac1,          8          \n"
     "extr_r.w          %[tmp_t3],   $ac2,          8          \n"
     "extr_r.w          %[tmp_t4],   $ac3,          8          \n"
     "addiu             %[dst_y],    %[dst_y],      4          \n"
     "addiu             %[src_argb0],%[src_argb0],  16         \n"
     "sb                %[tmp_t1],   -4(%[dst_y])              \n"
     "sb                %[tmp_t2],   -3(%[dst_y])              \n"
     "sb                %[tmp_t3],   -2(%[dst_y])              \n"
     "sb                %[tmp_t4],   -1(%[dst_y])              \n"
     ".set pop                                                 \n"
      :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
       [tmp_t3] "=&r" (tmp_t3), [tmp_t4] "=&r" (tmp_t4),
       [tmp_t5] "=&r" (tmp_t5), [tmp_t6] "=&r" (tmp_t6),
       [tmp_t7] "=&r" (tmp_t7), [tmp_t8] "=&r" (tmp_t8),
       [src_argb0] "+r" (src_argb0), [dst_y] "+r" (dst_y)
      :[const1] "r" (const1), [const2] "r" (const2),
       [const5] "r" (const5)
      :"hi", "lo"
    );
  }
  for (x = 0; x < (width & 3); ++x){
    uint8 ar = src_argb0[3];
    uint8 ag = src_argb0[2];
    uint8 ab = src_argb0[1];
    dst_y[0] = (66 * ar + 129 * ag +  25 * ab + 0x1080) >> 8;
    src_argb0 += 4;
    dst_y += 1;
  }
}

void ARGBToUVRow_DSPR2(const uint8* src_rgb0, int src_stride_rgb,
                       uint8* dst_u, uint8* dst_v, int width){
  const uint8* src_rgb1 = src_rgb0 + src_stride_rgb;
  int x;
  int const1 = 0xffb60070;
  int const2 = 0x0000ffda;
  int const3 = 0xffa2ffee;
  int const4 = 0x00000070;
  int const5 = 0x100;

  for (x = 0; x < width - 1; x += 2){
    int tmp_t1, tmp_t2, tmp_t3, tmp_t4, tmp_t5;
    int tmp_t6, tmp_t7, tmp_t8;
    __asm__ __volatile__ (
     ".set push                                                \n"
     ".set noreorder                                           \n"
     "lw                %[tmp_t1],   0(%[src_rgb0])            \n"
     "lw                %[tmp_t2],   4(%[src_rgb0])            \n"
     "lw                %[tmp_t3],   0(%[src_rgb1])            \n"
     "lw                %[tmp_t4],   4(%[src_rgb1])            \n"
     "preceu.ph.qbr     %[tmp_t5],   %[tmp_t1]                 \n"
     "preceu.ph.qbl     %[tmp_t1],   %[tmp_t1]                 \n"
     "preceu.ph.qbr     %[tmp_t6],   %[tmp_t2]                 \n"
     "preceu.ph.qbl     %[tmp_t2],   %[tmp_t2]                 \n"
     "preceu.ph.qbr     %[tmp_t7],   %[tmp_t3]                 \n"
     "preceu.ph.qbl     %[tmp_t3],   %[tmp_t3]                 \n"
     "preceu.ph.qbr     %[tmp_t8],   %[tmp_t4]                 \n"
     "preceu.ph.qbl     %[tmp_t4],   %[tmp_t4]                 \n"
     "addu.ph           %[tmp_t5],   %[tmp_t5],     %[tmp_t6]  \n"
     "addu.ph           %[tmp_t7],   %[tmp_t7],     %[tmp_t8]  \n"
     "addu.ph           %[tmp_t1],   %[tmp_t1],     %[tmp_t2]  \n"
     "addu.ph           %[tmp_t3],   %[tmp_t3],     %[tmp_t4]  \n"
     "addu.ph           %[tmp_t5],   %[tmp_t5],     %[tmp_t7]  \n"
     "addu.ph           %[tmp_t1],   %[tmp_t1],     %[tmp_t3]  \n"
     "shrl.ph           %[tmp_t5],   %[tmp_t5],     2          \n"
     "shrl.ph           %[tmp_t1],   %[tmp_t1],     2          \n"
     "mult              $ac0,        %[const5],     %[const5]  \n"
     "mult              $ac1,        %[const5],     %[const5]  \n"
     "dpaq_s.w.ph       $ac0,        %[tmp_t5],     %[const1]  \n"
     "dpaq_s.w.ph       $ac1,        %[tmp_t5],     %[const3]  \n"
     "dpaq_s.w.ph       $ac0,        %[tmp_t1],     %[const2]  \n"
     "dpaq_s.w.ph       $ac1,        %[tmp_t1],     %[const4]  \n"
     "extr_r.w          %[tmp_t7],   $ac0,          9          \n"
     "extr_r.w          %[tmp_t8],   $ac1,          9          \n"
     "addiu             %[src_rgb0], %[src_rgb0],   8          \n"
     "addiu             %[src_rgb1], %[src_rgb1],   8          \n"
     "addiu             %[dst_u],    %[dst_u],      1          \n"
     "addiu             %[dst_v],    %[dst_v],      1          \n"
     "sb                %[tmp_t7],   -1(%[dst_u])              \n"
     "sb                %[tmp_t8],   -1(%[dst_v])              \n"
     ".set pop                                                 \n"
      :[tmp_t1] "=&r" (tmp_t1), [tmp_t2] "=&r" (tmp_t2),
       [tmp_t3] "=&r" (tmp_t3), [tmp_t4] "=&r" (tmp_t4),
       [tmp_t5] "=&r" (tmp_t5), [tmp_t6] "=&r" (tmp_t6),
       [tmp_t7] "=&r" (tmp_t7), [tmp_t8] "=&r" (tmp_t8),
       [src_rgb0] "+r" (src_rgb0), [src_rgb1] "+r" (src_rgb1),
       [dst_u] "+r" (dst_u), [dst_v] "+r" (dst_v)
      :[const1] "r" (const1), [const2] "r" (const2),
       [const3] "r" (const3), [const4] "r" (const4),
       [const5] "r" (const5)
      :"hi", "lo"
    );
  }
  if (width & 1){
    uint8 ab = (src_rgb0[0] + src_rgb1[0]) >> 1;
    uint8 ag = (src_rgb0[1] + src_rgb1[1]) >> 1;
    uint8 ar = (src_rgb0[2] + src_rgb1[2]) >> 1;
    dst_u[0] = (112 * ab - 74 * ag - 38 * ar + 0x8080) >> 8;
    dst_v[0] = (112 * ar - 94 * ag - 18 * ab + 0x8080) >> 8;
  }
}

#endif  // defined(LIBYUV_DSPR2)

#ifdef __cplusplus
}  // extern "C"
}  // namespace libyuv
#endif
