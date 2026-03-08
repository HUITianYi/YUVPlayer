#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstring>
#include "core/yuv_converter.h"

// 计时辅助函数
template <typename Func>
double measure_time(const char* name, Func func, int iterations) {
    std::cout << "正在测试 " << name << "..." << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        func();
        if (i % 10 == 0) std::cout << "." << std::flush;
    }
    std::cout << " 完成。" << std::endl;
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> duration = end - start;
    return duration.count() / iterations;
}

int main() {
    // 基准测试配置
    const int width = 1920;
    const int height = 1080;
    const int iterations = 100; // 确保测量准确性
    const int thread_count = 4;

    std::cout << "正在运行 YUV 转 RGB 性能基准测试..." << std::endl;
    std::cout << "分辨率: " << width << "x" << height << std::endl;
    std::cout << "循环次数: " << iterations << std::endl;
    std::cout << "多线程数: " << thread_count << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    // 为 YUV 和 RGB 分配内存
    size_t y_size = width * height;
    size_t uv_size = (width / 2) * (height / 2);
    size_t rgb_size = width * height * 3;

    std::cout << "正在分配内存..." << std::endl;
    std::vector<uint8_t> y_data(y_size, 128); // 填充灰色
    std::vector<uint8_t> u_data(uv_size, 128);
    std::vector<uint8_t> v_data(uv_size, 128);
    std::vector<uint8_t> rgb_data(rgb_size);
    std::cout << "内存分配完成。" << std::endl;

    // 获取原始指针
    const uint8_t* y_ptr = y_data.data();
    const uint8_t* u_ptr = u_data.data();
    const uint8_t* v_ptr = v_data.data();
    uint8_t* rgb_ptr = rgb_data.data();

    // 1. 标量版本 (基准)
    double scalar_time = measure_time("标量 (Scalar)", [&]() {
        yuv420_to_rgb_scalar(y_ptr, u_ptr, v_ptr, rgb_ptr, width, height);
    }, iterations);
    std::cout << "标量 (C) 平均耗时: " << std::fixed << std::setprecision(2) << scalar_time << " ms" << std::endl;

    // 2. SSE 版本 (SIMD)
    double sse_time = measure_time("SSE (SIMD)", [&]() {
        yuv420_to_rgb_sse(y_ptr, u_ptr, v_ptr, rgb_ptr, width, height);
    }, iterations);
    std::cout << "SSE (SIMD) 平均耗时: " << sse_time << " ms" << std::endl;

    // 3. 多线程版本 (SSE + 线程)
    double mt_time = measure_time("多线程 (MT)", [&]() {
        yuv420_to_rgb_mt(y_ptr, u_ptr, v_ptr, rgb_ptr, width, height, thread_count);
    }, iterations);
    std::cout << "多线程平均耗时: " << mt_time << " ms" << std::endl;

    std::cout << "--------------------------------------------------" << std::endl;
    
    // 计算加速比
    double sse_speedup = scalar_time / sse_time;
    double mt_speedup = scalar_time / mt_time;

    std::cout << "SSE 相对标量加速比: " << sse_speedup << "x" << std::endl;
    std::cout << "多线程相对标量加速比:  " << mt_speedup << "x" << std::endl;

    return 0;
}
