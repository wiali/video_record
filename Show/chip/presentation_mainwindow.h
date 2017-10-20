/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef PRESENTATION_MAINWINDOW_H
#define PRESENTATION_MAINWINDOW_H

#include <QMainWindow>

#include <opencv2/opencv.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp> // for camera

#include <opencv2/core/ocl.hpp>

#include "opencv2/imgproc.hpp"
#include <Windows.h>

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QSplitter;
class DownCam;
QT_END_NAMESPACE

enum ScreenType
{
    MonitorScreen = 0,
    MatScreen = 1,
    PresentScreen = 2
};

namespace Ui {
class PresentMainWindow;
}

class PresentMainWindow : public QMainWindow
{
    Q_OBJECT
public:


    PresentMainWindow(QWidget *parent = 0);
    ~PresentMainWindow();

    QImage m_image_downCam;
    QImage m_image_webCam;
    QImage m_image_monitor;


    static QScreen* findScreen(const ScreenType& type, int* index = nullptr);

    static QRect findScreenGeometry(const ScreenType &type);

    static void setHardwareIds(QStringList monitorHardwareIds, QStringList matHardwareIds);

    static QStringList monitorHardwareIds() { return m_monitorHardwareIds; }

    static QStringList matHardwareIds() { return m_matHardwareIds; }


    static QMap<QString, QString> getScreenNames();
protected:
    void paintEvent(QPaintEvent *event) override;

private slots:

private:    
    cv::Mat hwnd2mat(HWND hwnd);
    QImage mat2Image_shared(const cv::Mat &mat, QImage::Format formatHint);

    Ui::PresentMainWindow *ui;

    QGraphicsScene* m_scene;

    cv::VideoCapture m_downCam;
    cv::VideoCapture m_webCam;

    DownCam* m_downCam_item;
    DownCam* m_webCam_item;
    DownCam* m_monitor_item;

    cv::Mat m_frame_downCam;
    cv::Mat m_frame_webCam;
    cv::Mat m_frame_monitor;

    static QStringList m_monitorHardwareIds;
    static QStringList m_matHardwareIds;
};

#endif // PRESENTATION_MAINWINDOW_H
