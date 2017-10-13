#ifndef FRAME_COUNTING_H
#define FRAME_COUNTING_H

#include <QWidget>
#include <QVector>

class FrameCounting : public QWidget
{
    Q_OBJECT

    struct Counting
    {
        Counting(FrameCounting* fc) : m_fc(fc) { m_fc->start(); }
        ~Counting() { m_fc->end(); }
        FrameCounting* m_fc;
    }; 

public:
    FrameCounting(bool showFps=false, QWidget *parent = nullptr);
    QSharedPointer<Counting> count();    

protected:
    void paintEvent(QPaintEvent *event) override;

private slots:
    void onTimerOut();

private:
    void start();
    void end();

private: 
    QTimer m_fpsTimer;
    int m_paint_time_avg;
    QVector<int> m_arr_paint_time;
    QTime m_paint_time;
    int m_frame_counts;
    int m_frame_counts_avg;

    bool m_showFps;
};


#endif // FRAME_COUNTING_H
