#ifndef YUV_COMMON_H
#define YUV_COMMON_H

#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

    // 假设采用 YUV420P 格式
    typedef struct
    {
        int width;
        int height;
        uint8_t *y_data;
        uint8_t *u_data;
        uint8_t *v_data;
        size_t frame_size; // 每帧总字节数
    } YUVFrame;

    typedef struct
    {
        FILE *file;
        int width;
        int height;
        size_t frame_index;
        size_t total_frames;
        YUVFrame current_frame;
        int is_y4m;             // 如果文件是 YUV4MPEG2 (.y4m) 格式，则为 1
        long data_start_offset; // 第一帧数据的起始偏移量（跳过文件头）
    } YUVReader;

    // 用于显示的 RGB 结构体 (ARGB 或 RGB)
    typedef struct
    {
        uint8_t *data; // RGB 打包数据
        int width;
        int height;
        int stride; // 每行字节数
    } RGBFrame;

#ifdef __cplusplus
}
#endif

#endif // YUV_COMMON_H
