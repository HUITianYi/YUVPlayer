#ifndef YUV_CONVERTER_H
#define YUV_CONVERTER_H

#include "yuv_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    // Convert YUV420P to RGB (Scalar implementation)
    void yuv420_to_rgb_scalar(const uint8_t *y, const uint8_t *u, const uint8_t *v,
                              uint8_t *rgb, int width, int height);

    // Convert YUV420P to RGB (SSE implementation)
    void yuv420_to_rgb_sse(const uint8_t *y, const uint8_t *u, const uint8_t *v,
                           uint8_t *rgb, int width, int height);

    // Convert YUV420P to RGB (Multi-threaded wrapper)
    void yuv420_to_rgb_mt(const uint8_t *y, const uint8_t *u, const uint8_t *v,
                          uint8_t *rgb, int width, int height, int thread_count);

#ifdef __cplusplus
}
#endif

#endif // YUV_CONVERTER_H
