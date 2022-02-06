//
// Created by Sergey Fadeev on 05.04.2021.
//

#pragma once

#include <QRunnable>
#include <vector>
#include <QSemaphore>

#include <QObject>
#include <complex>

class render_task : public QObject,  public QRunnable {
    Q_OBJECT
public:
    render_task(unsigned char* data,
                size_t stride,
                size_t h,
                size_t w,
                double scale,
                const std::complex<double>& center_offset,
                int threads_count,
                int thread_number,
                QSemaphore&);

    void run() override;
    void set_values(unsigned char*, size_t, size_t, size_t, double, std::complex<double>);
    void stop_task();

signals:
    void resultReady();

private:
    [[nodiscard]] unsigned char pixel_value(double x, double y) const;
    void render();

    std::vector<unsigned char> temp_data;
    unsigned char* data;

    size_t stride;
    size_t h;
    size_t w;

    double scale;
    std::complex<double> center_offset;

    int thread_number;
    int threads_count;

    bool restart = false;
    QSemaphore ready_semaphore;
    QSemaphore& memory_semaphore;
    std::atomic_bool need_stop = false;
};


