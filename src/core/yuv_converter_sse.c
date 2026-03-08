#include "yuv_converter.h"
#include <emmintrin.h> // SSE2
#include <tmmintrin.h> // SSSE3 (for _mm_shuffle_epi8 if needed, but we can do without)

// YUV 转 RGB 的系数（缩放 2^6 = 64 以适应 16 位精度）
// R = Y + 1.402 * (V-128)
// G = Y - 0.344 * (U-128) - 0.714 * (V-128)
// B = Y + 1.772 * (U-128)

// 缩放 64 倍：
// 1.402 * 64 = 89.7 -> 90
// 0.344 * 64 = 22.0 -> 22
// 0.714 * 64 = 45.7 -> 46
// 1.772 * 64 = 113.4 -> 113

void yuv420_to_rgb_sse(const uint8_t *y_ptr, const uint8_t *u_ptr, const uint8_t *v_ptr,
                       uint8_t *rgb_ptr, int width, int height)
{

    // 常量
    __m128i val_128 = _mm_set1_epi16(128);
    __m128i val_0 = _mm_setzero_si128();

    // RGB 转换系数（有符号 16 位）
    __m128i c_r_v = _mm_set1_epi16(90);
    __m128i c_g_u = _mm_set1_epi16(22);
    __m128i c_g_v = _mm_set1_epi16(46);
    __m128i c_b_u = _mm_set1_epi16(113);

    int i, j;

    // 一次处理 16 个像素
    for (j = 0; j < height; j++)
    {
        const uint8_t *y_line = y_ptr + j * width;
        const uint8_t *u_line = u_ptr + (j / 2) * (width / 2);
        const uint8_t *v_line = v_ptr + (j / 2) * (width / 2);
        uint8_t *rgb_line = rgb_ptr + j * width * 3;

        for (i = 0; i < width; i += 16)
        {
            // 加载 16 个 Y 字节
            __m128i y_16 = _mm_loadu_si128((const __m128i *)(y_line + i));

            // 加载 8 个 U 和 8 个 V 字节
            __m128i u_8 = _mm_loadl_epi64((const __m128i *)(u_line + i / 2));
            __m128i v_8 = _mm_loadl_epi64((const __m128i *)(v_line + i / 2));

            // 将 U 和 V 解包为 16 字节（复制每个字节）
            // u_8: u0 u1 u2 u3 u4 u5 u6 u7 00 00 ...
            // 我们需要: u0 u0 u1 u1 u2 u2 ...
            // 使用 unpacklo 与自身交错？不，标准 unpack 是与零或另一个寄存器交错。
            // 简单方法：先解包到 16 位，然后 shuffle？
            // 或者更简单：解包 8->16 (u0 00 u1 00...) 然后位移或？
            // 实际上，_mm_unpacklo_epi8(u_8, u_8) 会给出 u0 u0 u1 u1 ... 如果使用正确的掩码？不。

            // 先解包到 16 位 (u0 -> 00 u0)
            __m128i u_16_lo = _mm_unpacklo_epi8(u_8, u_8); // u0 u0 u1 u1 u2 u2 u3 u3 ...
            __m128i v_16_lo = _mm_unpacklo_epi8(v_8, v_8); // v0 v0 v1 v1 ...

            // 现在我们在 u_16_lo/v_16_lo 中有 16 个 U 和 16 个 V 字节（因为 u_8 在低 64 位）
            // 等等，unpacklo_epi8(a, a) 会交错 a 的低 64 位字节。
            // 如果 a = [u0 u1...], unpacklo(a, a) = [u0 u0 u1 u1...]. 正确！

            // 将 Y 分成两个 8 像素块（低位和高位）以进行 16 位运算
            __m128i y_lo = _mm_unpacklo_epi8(y_16, val_0); // 8 像素（16 位）
            __m128i y_hi = _mm_unpackhi_epi8(y_16, val_0); // 8 像素（16 位）

            // 将 U/V 分成两个 8 像素块
            __m128i u_lo = _mm_unpacklo_epi8(u_16_lo, val_0);
            __m128i u_hi = _mm_unpackhi_epi8(u_16_lo, val_0);

            __m128i v_lo = _mm_unpacklo_epi8(v_16_lo, val_0);
            __m128i v_hi = _mm_unpackhi_epi8(v_16_lo, val_0);

            // U 和 V 减去 128
            u_lo = _mm_sub_epi16(u_lo, val_128);
            u_hi = _mm_sub_epi16(u_hi, val_128);
            v_lo = _mm_sub_epi16(v_lo, val_128);
            v_hi = _mm_sub_epi16(v_hi, val_128);

            // 计算低 8 像素的 R, G, B
            // R = Y + 1.402 * V
            __m128i r_lo = _mm_add_epi16(y_lo, _mm_srai_epi16(_mm_mullo_epi16(v_lo, c_r_v), 6));

            // G = Y - 0.344 * U - 0.714 * V
            __m128i g_lo_part = _mm_add_epi16(_mm_mullo_epi16(u_lo, c_g_u), _mm_mullo_epi16(v_lo, c_g_v));
            __m128i g_lo = _mm_sub_epi16(y_lo, _mm_srai_epi16(g_lo_part, 6));

            // B = Y + 1.772 * U
            __m128i b_lo = _mm_add_epi16(y_lo, _mm_srai_epi16(_mm_mullo_epi16(u_lo, c_b_u), 6));

            // 计算高 8 像素的 R, G, B
            __m128i r_hi = _mm_add_epi16(y_hi, _mm_srai_epi16(_mm_mullo_epi16(v_hi, c_r_v), 6));

            __m128i g_hi_part = _mm_add_epi16(_mm_mullo_epi16(u_hi, c_g_u), _mm_mullo_epi16(v_hi, c_g_v));
            __m128i g_hi = _mm_sub_epi16(y_hi, _mm_srai_epi16(g_hi_part, 6));

            __m128i b_hi = _mm_add_epi16(y_hi, _mm_srai_epi16(_mm_mullo_epi16(u_hi, c_b_u), 6));

            // 打包回 8 位（饱和处理）
            // packus 将 16 位有符号数打包为 8 位无符号数并饱和
            __m128i r_8 = _mm_packus_epi16(r_lo, r_hi);
            __m128i g_8 = _mm_packus_epi16(g_lo, g_hi);
            __m128i b_8 = _mm_packus_epi16(b_lo, b_hi);

            // 现在我们有 16 个 R，16 个 G，16 个 B 字节。
            // 结构：R0 R1 ... R15, G0 G1 ... G15, B0 B1 ... B15
            // 需要交错排列为 RGB RGB ...
            // 这是 SSE 中比较棘手的部分。
            // 既然我们只需要写入内存，如果懒的话可以逐字节写，
            // 但这就违背了初衷。
            // 或者如果能 shuffle 的话使用 _mm_storeu_si128。

            // 用 SSE 存储 RGB（3 字节）很麻烦（16 字节寄存器）。
            // 16 像素 * 3 字节 = 48 字节。
            // 正好 3 个 XMM 寄存器。

            // 我们可以 shuffle 得到：
            // Reg1: R0 G0 B0 R1 G1 B1 ...
            // 这是复杂的 shuffling。

            // 存储的后备方案：直接提取并存储？不，太慢。
            // 让我们先实现一个简单的存储循环，或者使用已知的 shuffle 模式。
            // "数组的结构" (SOA) 到 "结构的数组" (AOS) 转换。

            // 优化：能不能直接线性存储？
            // 不行，显示需要打包的 RGB。

            // 让我们用临时缓冲区和标准 C 来打包，如果 shuffling 太难搞定。
            // 或者更好：实现打包。

            uint8_t r_buf[16], g_buf[16], b_buf[16];
            _mm_storeu_si128((__m128i *)r_buf, r_8);
            _mm_storeu_si128((__m128i *)g_buf, g_8);
            _mm_storeu_si128((__m128i *)b_buf, b_8);

            for (int k = 0; k < 16; k++)
            {
                rgb_line[(i + k) * 3 + 0] = r_buf[k];
                rgb_line[(i + k) * 3 + 1] = g_buf[k];
                rgb_line[(i + k) * 3 + 2] = b_buf[k];
            }
        }
    }
}
