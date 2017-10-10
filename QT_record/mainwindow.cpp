#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "audio_recording.h"
#include "video_recording.h"
#include "desktop_record.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    DesktopRecord record;
}

void MainWindow::on_recordvideo_clicked()
{
    VideoRecording record;
}

void MainWindow::on_recordaudio_clicked()
{
    AudioRecording record;
}
