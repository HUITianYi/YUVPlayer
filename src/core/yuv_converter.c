#include "yuv_converter.h"
#include <math.h>

#define CLAMP(x) ((x) < 0 ? 0 : ((x) > 255 ? 255 : (x)))

// 标量实现
void yuv420_to_rgb_scalar(const uint8_t *y, const uint8_t *u, const uint8_t *v,
                          uint8_t *rgb, int width, int height)
{
    int i, j;
    int y_idx, uv_idx;
    int r, g, b;
    int Y, U, V;

    for (j = 0; j < height; j++)
    {
        for (i = 0; i < width; i++)
        {
            y_idx = j * width + i;
            uv_idx = (j / 2) * (width / 2) + (i / 2);

            Y = y[y_idx];
            U = u[uv_idx] - 128;
            V = v[uv_idx] - 128;

            // BT.601 的整数近似
            // R = Y + 1.402 * V
            // G = Y - 0.344 * U - 0.714 * V
            // B = Y + 1.772 * U

            // 使用整数运算（乘以 1024 并移位）
            // 1.402 * 1024 = 1436
            // 0.344 * 1024 = 352
            // 0.714 * 1024 = 731
            // 1.772 * 1024 = 1815

            r = Y + ((1436 * V) >> 10);
            g = Y - ((352 * U + 731 * V) >> 10);
            b = Y + ((1815 * U) >> 10);

            // 存入 RGB（打包，每像素 3 字节）
            // 或者 RGBA（每像素 4 字节，用于对齐/Qt QImage）
            // 这里假设 RGB（3 字节），但 Qt 倾向于 32 位对齐。
            // 不过，QImage Format_RGB888 是 24 位的。

            int rgb_idx = (j * width + i) * 3;
            rgb[rgb_idx] = CLAMP(r);
            rgb[rgb_idx + 1] = CLAMP(g);
            rgb[rgb_idx + 2] = CLAMP(b);
        }
    }
}
