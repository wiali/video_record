#pragma once
#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QOpenGLWidget>
#include <QTouchEvent>
#include <QWidget>
#include <QSharedPointer>
#include <QGestureEvent>
#include <QOpenGLFramebufferObject>
#include <QTransform>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QOpenGLTexture>

enum InterpolationType
{
  Nearest = 1,
  Linear = 2
};

struct CombinedFilterData
{
    QScopedPointer<QOpenGLVertexArrayObject> vertexArrayObject;

    QSize imageSize;

    QSize keystoneCorrectedSize;

    QSharedPointer<QOpenGLShaderProgram> filtersProgram;

    GLuint renderBuffer;

    QMatrix3x3 textureMatrix;

    QMatrix4x4 projectionMatrix;

    QSharedPointer<QOpenGLBuffer> vertexBuffer;

    QOpenGLTexture::PixelFormat pixelFormat;

    InterpolationType interpolationType;
};

class VideoWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    VideoWidget(QWidget *parent = 0);

public slots:

public slots:
    
private slots:

signals:

protected:
//    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

private:    
    bool convert(uchar* sourceImageData, QSize imageSize, QOpenGLFramebufferObject* frameBuffer);
    QSharedPointer<CombinedFilterData> combinedInit(const QSize &imageSize);

    void combined(uchar* sourceImageData, QSize sourceImageSize, QOpenGLFramebufferObject *frameBuffer,
                  QSharedPointer<CombinedFilterData> &data);
    void runCorrectionFiltersProgram(QOpenGLFramebufferObject *targetFrameBuffer,
                                       QSharedPointer<CombinedFilterData> &data,
                                       const void * inputBuffer);
    void buildVertexBuffer(QSharedPointer<CombinedFilterData> data);
    QSharedPointer<QOpenGLShaderProgram> createAndCompileShaderProgram(QString programName);
    void readAndCompileShaderFile(QOpenGLShader *shader, QString shaderFileName);
    QSharedPointer<QOpenGLTexture> createTexture(QSize size, InterpolationType interpolationType, QOpenGLTexture::TextureFormat format = QOpenGLTexture::RGB8_UNorm);

    QMap<QOpenGLContext*, QSharedPointer<QOpenGLFramebufferObject>> m_frameBuffers;
    QSharedPointer<CombinedFilterData> m_contextData;
};

#endif
