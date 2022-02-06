//
// Created by Sergey Fadeev on 05.04.2021.
//

#include "render_task.h"

render_task::render_task(unsigned char* data,
                                    size_t stride,
                                    size_t h,
                                    size_t w,
                                    double scale,
                                    const std::complex<double>& center_offset,
                                    int threads_count,
                                    int thread_number,
                                    QSemaphore& memory_semaphore)
    : data(data)
    , stride(stride)
    , h(h)
    , w(w)
    , scale(scale)
    , center_offset(center_offset)
    , threads_count(threads_count)
    , thread_number(thread_number)
    , memory_semaphore(memory_semaphore){
}

void render_task::run() {
    while(true) {
        render();
        ready_semaphore.acquire();
        if(need_stop.load()) {
            return;
        }
    }
}

void render_task::set_values(unsigned char* new_data,
                             size_t new_stride,
                             size_t new_h,
                             size_t new_w,
                             double new_scale,
                             std::complex<double> new_center_offset) {
    restart = true;
    data = new_data;
    stride = new_stride;
    h = new_h;
    w = new_w;
    scale = new_scale;
    center_offset = new_center_offset;
    ready_semaphore.release(1);
}

void render_task::stop_task() {
    restart = true;
    need_stop = true;
    ready_semaphore.release(1);
}

void render_task::render() {
    restart = false;
    size_t step = 64;
    size_t line_size = h / threads_count;
    size_t line_pos = h / threads_count * thread_number;

    if (thread_number == threads_count - 1) {
        line_size = h - (threads_count - 1) * line_size;
        line_pos = h - line_size;
    }

    temp_data.resize(line_size * stride);

    while (step != 0) {
        for (size_t i = 0; i < line_size && !restart; i += step) {
            for (size_t j = 0; j < w && !restart; j += step) {
                unsigned char cur_val = pixel_value(j, line_pos + i);
                for (size_t o = i; o < i + step &&
                                   o < line_size &&
                                   o + line_pos < h &&
                                   !restart; o++) {
                    for (size_t k = j; k < j + step && k < w && !restart; k++) {
                        temp_data[o * stride + k * 3 + 0] = cur_val / 2;
                        temp_data[o * stride + k * 3 + 1] = 0;
                        temp_data[o * stride + k * 3 + 2] = cur_val;
                    }
                }
            }
        }
        memory_semaphore.acquire();
        if(restart) {
            memory_semaphore.release(1);
            return;
        }
        memcpy(data + line_pos * stride, temp_data.data(), line_size * stride);
        memory_semaphore.release(1);
        resultReady();
        step /= 2;
    }
}

unsigned char render_task::pixel_value(double x, double y) const {
    std::complex<double> c(x - w / 2., y - h / 2.);
    c *= scale;
    c += center_offset;

    std::complex<double> z(0, 0);

    const size_t max_iteration = 100;
    size_t step = 0;
    while(true) {
        if (step >= max_iteration) {
            return 0;
        } else if (std::abs(z) >= 2.0) {
            return static_cast<unsigned char>(step * 2.56);
        }
        step++;
        z = z * z + c;
    }
}