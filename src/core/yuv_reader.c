#include "yuv_common.h"
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#endif

// 初始化 YUV 读取器
YUVReader *yuv_reader_create(const char *filename, int width, int height)
{
    YUVReader *reader = (YUVReader *)malloc(sizeof(YUVReader));
    if (!reader)
        return NULL;

    reader->width = width;
    reader->height = height;
    reader->frame_index = 0;

    // 以二进制模式打开文件
    reader->file = fopen(filename, "rb");
    if (!reader->file)
    {
        free(reader);
        return NULL;
    }

    reader->is_y4m = 0;
    reader->data_start_offset = 0;

    // 检查 YUV4MPEG2 签名 (Y4M)
    char signature[10];
    if (fread(signature, 1, 10, reader->file) == 10 && strncmp(signature, "YUV4MPEG2", 9) == 0)
    {
        reader->is_y4m = 1;
        // 读取头信息以解析分辨率（宽和高）
        // 示例头: YUV4MPEG2 W1920 H1080 F50:1 Ip A1:1
        int c;
        char buffer[256];
        int buf_idx = 0;
        
        // 读取直到换行符
        while ((c = fgetc(reader->file)) != '\n' && c != EOF && buf_idx < 255) {
            buffer[buf_idx++] = (char)c;
        }
        buffer[buf_idx] = '\0';
        reader->data_start_offset = ftell(reader->file);

        // 解析 W 和 H
        char *w_ptr = strchr(buffer, 'W');
        char *h_ptr = strchr(buffer, 'H');
        
        if (w_ptr) reader->width = atoi(w_ptr + 1);
        if (h_ptr) reader->height = atoi(h_ptr + 1);
        
        // 更新 frame_size，因为宽/高可能变了
        // 这里需要重新计算 size 并可能重新分配（虽然 create 函数还没分配 buffer，但在之后分配）
        // 注意：reader->current_frame.width/height 在 create 函数末尾才赋值，所以这里只需更新 reader->width/height
    }
    else
    {
        rewind(reader->file); // 不是 Y4M 格式，回到文件开头
    }

    // 计算总帧数
    fseek(reader->file, 0, SEEK_END);
    size_t file_size = ftell(reader->file);
    fseek(reader->file, reader->data_start_offset, SEEK_SET);

    size_t y_size = width * height;
    size_t u_size = (width / 2) * (height / 2);
    size_t v_size = u_size;
    reader->current_frame.frame_size = y_size + u_size + v_size;

    // 分配帧缓冲区
    // 使用对齐分配以提高 SIMD 效率（16 或 32 字节）
    // 注意：如果是 Y4M，这里的 width/height 可能已经被更新为正确的值
    // 但是 y_size 等变量是基于传入的 width/height 计算的，可能不正确
    // 重新计算 size
    y_size = reader->width * reader->height;
    u_size = (reader->width / 2) * (reader->height / 2);
    v_size = u_size;
    reader->current_frame.frame_size = y_size + u_size + v_size;

    // 注意：对于 Y4M，文件大小包含 'FRAME\n' 头，所以这里的总帧数是一个近似值（上限）
    // 但对于基本的进度条检查已经足够了
    reader->total_frames = (file_size - reader->data_start_offset) / reader->current_frame.frame_size;

// 分配帧缓冲区
// 使用对齐分配以提高 SIMD 效率（16 或 32 字节）
#ifdef _WIN32
    reader->current_frame.y_data = (uint8_t *)_aligned_malloc(y_size, 32);
    reader->current_frame.u_data = (uint8_t *)_aligned_malloc(u_size, 32);
    reader->current_frame.v_data = (uint8_t *)_aligned_malloc(v_size, 32);
#else
    posix_memalign((void **)&reader->current_frame.y_data, 32, y_size);
    posix_memalign((void **)&reader->current_frame.u_data, 32, u_size);
    posix_memalign((void **)&reader->current_frame.v_data, 32, v_size);
#endif

    reader->current_frame.width = reader->width;
    reader->current_frame.height = reader->height;

    return reader;
}

// 读取下一帧
int yuv_reader_next_frame(YUVReader *reader)
{
    if (!reader || !reader->file)
        return -1;

    size_t y_size = reader->width * reader->height;
    size_t uv_size = (reader->width / 2) * (reader->height / 2);

    // Y4M 特有：跳过 FRAME 帧头
    if (reader->is_y4m)
    {
        int c;
        // 帧头以 "FRAME" 开头。可能包含参数，以 0x0A 结束。
        // 我们只需扫描直到换行符。
        // 首先检查是否真的是 'F'（或 EOF）
        long current_pos = ftell(reader->file);
        c = fgetc(reader->file);
        if (c == 'F')
        {
            // 是帧头，消耗直到换行符
            while ((c = fgetc(reader->file)) != '\n' && c != EOF)
                ;
        }
        else if (c == EOF)
        {
            return 0; // 文件结束
        }
        else
        {
            // 意外情况，也许不是头？尝试回退并读取数据？
            // 或者可能只是恰好不以 F 开头的数据？
            // 在 Y4M 规范中，每一帧必须以 FRAME 开头。
            // 如果到了这里，说明有问题或者是 EOF。
            ungetc(c, reader->file);
        }
    }

    // 读取 Y 分量
    size_t read_bytes = fread(reader->current_frame.y_data, 1, y_size, reader->file);
    if (read_bytes != y_size)
        return 0; // 文件结束或出错

    // 读取 U 分量
    read_bytes = fread(reader->current_frame.u_data, 1, uv_size, reader->file);
    if (read_bytes != uv_size)
        return 0;

    // 读取 V 分量
    read_bytes = fread(reader->current_frame.v_data, 1, uv_size, reader->file);
    if (read_bytes != uv_size)
        return 0;

    reader->frame_index++;
    return 1; // 成功
}

// 跳转到指定帧
int yuv_reader_seek(YUVReader *reader, size_t frame_idx)
{
    if (!reader || !reader->file)
        return -1;
    if (frame_idx >= reader->total_frames)
        return -1;

    // 对于 Y4M，由于变长头，跳转比较复杂。
    // 我们假定每帧头都是固定长度的 "FRAME\n" (6字节)
    if (reader->is_y4m)
    {
        size_t y4m_frame_size = 6 + reader->current_frame.frame_size;
        size_t offset = reader->data_start_offset + frame_idx * y4m_frame_size;
        if (fseek(reader->file, (long)offset, SEEK_SET) == 0)
        {
            reader->frame_index = frame_idx;
            return 1;
        }
        return 0;
    }

    size_t offset = frame_idx * reader->current_frame.frame_size;
    if (fseek(reader->file, (long)offset, SEEK_SET) == 0)
    {
        reader->frame_index = frame_idx;
        return 1;
    }
    return 0;
}

// 清理资源
void yuv_reader_destroy(YUVReader *reader)
{
    if (reader)
    {
        if (reader->file)
            fclose(reader->file);

#ifdef _WIN32
        if (reader->current_frame.y_data)
            _aligned_free(reader->current_frame.y_data);
        if (reader->current_frame.u_data)
            _aligned_free(reader->current_frame.u_data);
        if (reader->current_frame.v_data)
            _aligned_free(reader->current_frame.v_data);
#else
        if (reader->current_frame.y_data)
            free(reader->current_frame.y_data);
        if (reader->current_frame.u_data)
            free(reader->current_frame.u_data);
        if (reader->current_frame.v_data)
            free(reader->current_frame.v_data);
#endif

        free(reader);
    }
}
