#include "frame_counting.h"

#include <QDebug>

const int DEFAULT_FPS = 60;

FrameCounting::FrameCounting(bool showFps, QWidget *parent)
  : QWidget(parent), m_showFps(showFps)
{
    if (!m_showFps)
        return;

    setAutoFillBackground(false);
    // lets make sure we have a transparent background
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_PaintOnScreen);
    setAttribute(Qt::WA_TransparentForMouseEvents);

    // Set Object Name
    setObjectName("FrameCounting_Window");

    m_fpsTimer.setInterval(1000);
    m_fpsTimer.start();
    connect(&m_fpsTimer, SIGNAL(timeout()), this, SLOT(onTimerOut()));

    setGeometry(0, 0, 500, 500);
}

void FrameCounting::start()
{
    m_paint_time.restart();
}

void FrameCounting::end()
{
    m_frame_counts++;
    m_arr_paint_time.push_back(m_paint_time.elapsed());
}

void FrameCounting::paintEvent(QPaintEvent* paintEvent)
{
    if (!m_showFps)
        return;

    QPainter painter(this);

    QPen pen;
    pen.setColor(Qt::blue);
    QFont font;
    font.setFamily("Segoe UI");
    font.setBold(true);
    font.setPointSize(20);
    font.setUnderline(true);
    font.setOverline(true);
    painter.setFont(font);
    painter.setPen(pen);

    QString data = QString("%1ms").arg(m_paint_time_avg);
    painter.drawText(100, 50, data);

    pen.setColor(Qt::red);
    painter.setPen(pen);
    data = QString("%1fps").arg(m_frame_counts_avg);
    painter.drawText(20, 50, data);

    QWidget::paintEvent(paintEvent);
}

void FrameCounting::onTimerOut()
{
    m_frame_counts_avg = m_frame_counts;
    m_frame_counts = 0;

    if (m_arr_paint_time.empty())
    {
        m_frame_counts_avg = DEFAULT_FPS;
        m_paint_time_avg = 0;
        return;
    }

    float nTotalTime = 0.0;
    for (auto time : m_arr_paint_time)
    {
        nTotalTime += time;
    }
    m_paint_time_avg = nTotalTime / m_arr_paint_time.size();
    m_arr_paint_time.clear();
}

QSharedPointer<FrameCounting::Counting> FrameCounting::count()
{
    if (m_showFps)
    {
        return QSharedPointer<Counting>::create(this);
    }
    return QSharedPointer<Counting>();
}