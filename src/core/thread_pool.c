#include "yuv_converter.h"
#include <process.h>
#include <windows.h>

typedef struct
{
    const uint8_t *y;
    const uint8_t *u;
    const uint8_t *v;
    uint8_t *rgb;
    int width;
    int start_h;
    int end_h;
    int total_height;
} ThreadData;

unsigned __stdcall thread_func(void *arg)
{
    ThreadData *data = (ThreadData *)arg;

    // 调用标量或SSE函数处理该块
    // 我们需要一个修改后的版本，该版本接受起始/结束高度或偏移量
    // 但我们可以直接偏移指针！

    // 计算偏移量
    int y_offset = data->start_h * data->width;
    int uv_offset = (data->start_h / 2) * (data->width / 2);
    int rgb_offset = data->start_h * data->width * 3;

    int chunk_height = data->end_h - data->start_h;

    // 为了速度，调用SSE版本
    yuv420_to_rgb_sse(data->y + y_offset,
                      data->u + uv_offset,
                      data->v + uv_offset,
                      data->rgb + rgb_offset,
                      data->width,
                      chunk_height);

    return 0;
}

void yuv420_to_rgb_mt(const uint8_t *y, const uint8_t *u, const uint8_t *v,
                      uint8_t *rgb, int width, int height, int thread_count)
{

    HANDLE *threads = (HANDLE *)malloc(sizeof(HANDLE) * thread_count);
    ThreadData *args = (ThreadData *)malloc(sizeof(ThreadData) * thread_count);

    int chunk_size = height / thread_count;
    // 确保块大小为偶数以进行UV平面通过对齐
    if (chunk_size % 2 != 0)
        chunk_size--;

    for (int i = 0; i < thread_count; i++)
    {
        args[i].y = y;
        args[i].u = u;
        args[i].v = v;
        args[i].rgb = rgb;
        args[i].width = width;
        args[i].total_height = height;

        args[i].start_h = i * chunk_size;
        if (i == thread_count - 1)
        {
            args[i].end_h = height; // 最后一个线程处理剩余部分
        }
        else
        {
            args[i].end_h = (i + 1) * chunk_size;
        }

        threads[i] = (HANDLE)_beginthreadex(NULL, 0, thread_func, &args[i], 0, NULL);
    }

    WaitForMultipleObjects(thread_count, threads, TRUE, INFINITE);

    for (int i = 0; i < thread_count; i++)
    {
        CloseHandle(threads[i]);
    }

    free(threads);
    free(args);
}
