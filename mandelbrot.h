//
// Created by Sergey Fadeev on 30.03.2021.
//

#pragma once

#include "render_task.h"

#include <QWidget>
#include <QMainWindow>
#include <QThreadPool>

#include <complex>
#include <memory>

QT_BEGIN_NAMESPACE
namespace Ui { class mandelbrot; }
QT_END_NAMESPACE

class mandelbrot : public QMainWindow {
Q_OBJECT

public:
    explicit mandelbrot(QWidget *parent = nullptr);

    ~mandelbrot() override;

    void paintEvent(QPaintEvent*) override;
    void wheelEvent(QWheelEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void keyPressEvent(QKeyEvent*) override;
    void resizeEvent(QResizeEvent*) override;

    void threadIsReady();
    void restart_threads(bool);

private:
    std::unique_ptr<Ui::mandelbrot> ui;
    QImage main_image;

    QPoint cur_mouse_move_start_pos;
    std::complex<double> center_offset{0, 0};
    double scale = 0.005;
    double sizee = 1;

    std::vector<render_task*> tasks;
    std::unique_ptr<QThreadPool> pool;
    int threads_count;
    QSemaphore memory_semaphore;
};
