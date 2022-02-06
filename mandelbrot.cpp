//
// Created by Sergey Fadeev on 30.03.2021.
//

#include "ui_main_view.h"

#include "mandelbrot.h"
#include "render_task.h"

#include <QPaintEngine>
#include <QWheelEvent>
#include <QScreen>
#include <QThread>

#include <complex>

mandelbrot::mandelbrot(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::mandelbrot) {
    ui->setupUi(this);
    resize(QApplication::screens().at(0)->availableSize());

    threads_count = std::max(QThread::idealThreadCount() - 1, 1);

    pool = std::make_unique<QThreadPool>(this);
    pool->setMaxThreadCount(QThread::idealThreadCount());
    memory_semaphore.release(1);

    for(int i = 0; i < threads_count; i++) {
        auto* task = new render_task(main_image.bits(),
                                     main_image.bytesPerLine(),
                                     main_image.height(),
                                     main_image.width(),
                                     scale,
                                     center_offset,
                                     threads_count,
                                     i,
                                     memory_semaphore);
        tasks.push_back(task);
        connect(task, &render_task::resultReady, this, &mandelbrot::threadIsReady);
        pool->start(task);
    }
}

mandelbrot::~mandelbrot() {
    for(auto* task: tasks) {
        task->stop_task();
    }
    pool->clear();
}

void mandelbrot::paintEvent(QPaintEvent* ev) {
    QMainWindow::paintEvent(ev);
    QPainter p(this);
    p.drawImage(0, 0, main_image);
}

void mandelbrot::wheelEvent(QWheelEvent* event) {
    if (event->angleDelta().y() == 0) {
        return;
    }
    scale *= (1 - std::min(event->angleDelta().y(), 500) / 1000.);
    sizee *= (1 - std::min(event->angleDelta().y(), 500) / 1000.);

    restart_threads(false);
}

void mandelbrot::mousePressEvent(QMouseEvent* event) {
    cur_mouse_move_start_pos = event->pos();
}

void mandelbrot::mouseMoveEvent(QMouseEvent* event) {
    auto x_delta = cur_mouse_move_start_pos.x() - event->pos().x();
    auto y_delta = cur_mouse_move_start_pos.y() - event->pos().y();

    cur_mouse_move_start_pos = event->pos();
    center_offset += std::complex<double>(x_delta * 0.005 * sizee, y_delta * 0.005 * sizee);

    restart_threads(false);
}

void mandelbrot::keyPressEvent(QKeyEvent* event) {
    bool need_update = true;
    if (event->key() == Qt::Key_Left) {
        center_offset += std::complex<double>(-0.01 * sizee, 0);
    } else if (event->key() == Qt::Key_Right) {
        center_offset += std::complex<double>(+0.01 * sizee, 0);
    } else if (event->key() == Qt::Key_Up) {
        center_offset += std::complex<double>(0, -0.01 * sizee);
    } else if (event->key() == Qt::Key_Down) {
        center_offset += std::complex<double>(0, +0.01 * sizee);
    } else if (event->key() == Qt::Key_Return) {
        scale *= (0.975);
        sizee *= (0.975);
    } else if (event->key() == Qt::Key_Space) {
        scale *= (1.025);
        sizee *= (1.025);
    } else {
        need_update = false;
    }

    if (need_update) {
        restart_threads(false);
    }
}

void mandelbrot::resizeEvent(QResizeEvent*) {
    memory_semaphore.acquire();
    main_image = QImage(size(), QImage::Format_RGB888);
    restart_threads(true);
}

void mandelbrot::threadIsReady() {
    update();
}

void mandelbrot::restart_threads(bool need_release_memory_semaphore) {
    for(auto* i: tasks) {
        i->set_values(main_image.bits(),
                     main_image.bytesPerLine(),
                     main_image.height(),
                     main_image.width(),
                     scale,
                     center_offset);
    }

    if(need_release_memory_semaphore) {
        memory_semaphore.release(1);
    }
}
