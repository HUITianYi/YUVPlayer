#ifndef YUV_READER_H
#define YUV_READER_H

#include "yuv_common.h"

#ifdef __cplusplus
extern "C"
{
#endif

    YUVReader *yuv_reader_create(const char *filename, int width, int height);
    int yuv_reader_next_frame(YUVReader *reader);
    int yuv_reader_seek(YUVReader *reader, size_t frame_idx);
    void yuv_reader_destroy(YUVReader *reader);

#ifdef __cplusplus
}
#endif

#endif // YUV_READER_H
