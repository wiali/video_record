#include "video_widget.h"

#include <QApplication>

/*
[Resolutions]
ColorCamera4K="{'height':3312,'width':4416}"
ColorCamera2K="{'height':1656,'width':2208}"
ColorCamera1K="{'height':828,'width':1104}"
Screen4K="{'height':2800,'width':4200}"
Screen2K="{'height':1280,'width':1920}"
Screen1K="{'height':700,'width':1050}"
InfraredCamera4K="{'height':480,'width':640}"
ProjGen2="{'height':1280,'width':1920}"

            "DestinationDevice": "screen",
            "DestinationResolution": {
                "height": 2800,
                "width": 4200
            },
            "SourceDevice": "color camera",
            "SourceResolution": {
                "height": 3312,
                "width": 4416
            },
            "matrix": [
                0.99800744125081942,
                -0.12494066354552548,
                -58.492698385016908,
                0.0031995645556478947,
                0.96248397740829172,
                -333.29982033723468,
                2.3323995676436789e-07,
                -5.8578188344702079e-05,
                1
            ]
*/

VideoWidget::VideoWidget(QWidget* parent)
    : QOpenGLWidget(parent)
{
//    setAutoFillBackground(false);
//    setAttribute(Qt::WA_NoSystemBackground, true);

    grabGesture(Qt::PanGesture);
    grabGesture(Qt::PinchGesture);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Background, Qt::black);
    setPalette(palette);
}

//void VideoWidget::initializeGL()
//{
//    initializeOpenGLFunctions();

//    makeCurrent();

//    // fix vertical sync issue as found by Randy in NVIDIA forum
//    QSurfaceFormat format;
//    format.setSwapInterval(0);
//    QSurfaceFormat::setDefaultFormat(format);

//    glShadeModel(GL_SMOOTH);

//    doneCurrent();
//}

void VideoWidget::resizeGL(int width, int height)
{
    QOpenGLWidget::resizeGL(width, height);
}

void VideoWidget::paintGL()
{
    auto f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(0, 0, 0, 1);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    const auto context = QOpenGLContext::currentContext();
    QSize frameSize(4200, 2800);
    if ( !m_frameBuffers.contains(context))
    {
        m_frameBuffers.insert(context, QSharedPointer<QOpenGLFramebufferObject>::create(frameSize));
    }

    if (m_frameBuffers.contains(context))
    {
        uchar* sourceImageData = new uchar[4416*3312];
        QSize imageSize(4416, 3312);
        convert(sourceImageData, imageSize, m_frameBuffers[context].data());
    }

    // Blit to target framebuffer ...
    QRect source(0,0,4200,2800);
    QRect destination(0,0,1920,1080);

    QSharedPointer<QOpenGLFramebufferObject> frameBuffer =
            m_frameBuffers.contains(context) ? m_frameBuffers[context] :
                                               QSharedPointer<QOpenGLFramebufferObject>();

    QOpenGLFramebufferObject::blitFramebuffer(0, destination, frameBuffer.data(), source,
                                              // SPROUTSW-4416 - Using linear sampling instead of default nearest
                                              GL_COLOR_BUFFER_BIT, GL_LINEAR);

}

bool VideoWidget::convert(uchar* sourceImageData, QSize imageSize, QOpenGLFramebufferObject* glFrameBuffer)
{
    bool result = false;
    //const auto context = QOpenGLContext::currentContext();

    if (!m_contextData)
    {
        m_contextData = combinedInit(imageSize);
    }

    if (m_contextData)
    {
        combined(sourceImageData, imageSize, glFrameBuffer, m_contextData);

        result = true;
    }

    return result;
}

QSharedPointer<CombinedFilterData> VideoWidget::combinedInit(const QSize& imageSize)
{
    auto data = QSharedPointer<CombinedFilterData>::create();

    data->imageSize = imageSize;
    data->pixelFormat = QOpenGLTexture::PixelFormat::BGR;//QImage::Format_RGB888;

    data->filtersProgram = createAndCompileShaderProgram(":/filtersShader");

    buildVertexBuffer(data);

    data->projectionMatrix.ortho(0, 1, 0, 1, -1, 1);

    data->keystoneCorrectedSize = QSize(4200,2800);

    QTransform keystoneCorrectionTransformation = {
    0.99800744125081942,
    -0.12494066354552548,
    -58.492698385016908,
    0.0031995645556478947,
    0.96248397740829172,
    -333.29982033723468,
    2.3323995676436789e-07,
    -5.8578188344702079e-05,
    1};

    const auto invertedHomography = keystoneCorrectionTransformation.inverted();

    data->textureMatrix(0, 0) = static_cast<GLfloat>(invertedHomography.m11());
    data->textureMatrix(1, 0) = static_cast<GLfloat>(invertedHomography.m12());
    data->textureMatrix(2, 0) = static_cast<GLfloat>(invertedHomography.m13());
    data->textureMatrix(0, 1) = static_cast<GLfloat>(invertedHomography.m21());
    data->textureMatrix(1, 1) = static_cast<GLfloat>(invertedHomography.m22());
    data->textureMatrix(2, 1) = static_cast<GLfloat>(invertedHomography.m23());
    data->textureMatrix(0, 2) = static_cast<GLfloat>(invertedHomography.m31());
    data->textureMatrix(1, 2) = static_cast<GLfloat>(invertedHomography.m32());
    data->textureMatrix(2, 2) = static_cast<GLfloat>(invertedHomography.m33());

    return data;
}

void VideoWidget::combined(uchar* sourceImageData, QSize sourceImageSize,
                                  QOpenGLFramebufferObject *frameBuffer,
                                  QSharedPointer<CombinedFilterData>& data)
{
    const auto functions = QOpenGLContext::currentContext()->functions();

    GLint viewport[4];

    functions->glGetIntegerv(GL_VIEWPORT, viewport);
    functions->glViewport(0, 0, sourceImageSize.width(), sourceImageSize.height());

    QOpenGLFramebufferObject *finalFrameBuffer = frameBuffer;
    QScopedPointer<QOpenGLFramebufferObject> mirrorFrameBuffer;

    runCorrectionFiltersProgram(finalFrameBuffer, data, sourceImageData);

    data->vertexArrayObject->release();
    frameBuffer->release();

    functions->glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void VideoWidget::runCorrectionFiltersProgram(QOpenGLFramebufferObject *frameBuffer,
                                                     QSharedPointer<CombinedFilterData> &data,
                                                     const void* inputBuffer)
{
    const auto functions = QOpenGLContext::currentContext()->functions();

    auto inputTexture = createTexture(data->imageSize, data->interpolationType);

    auto const imageSize = data->imageSize;

    inputTexture->setData(data->pixelFormat, QOpenGLTexture::UInt8, inputBuffer);

    frameBuffer->bind();
    inputTexture->bind();

    const auto textureMatrix = data->textureMatrix;

    data->filtersProgram->bind();
    data->filtersProgram->setUniformValue("projectionMatrix", data->projectionMatrix);
    data->filtersProgram->setUniformValue("textureMatrix", textureMatrix);
    data->filtersProgram->setUniformValue("imageSize", QVector2D(imageSize.width(), imageSize.height()));
    data->filtersProgram->setUniformValue("tex", 0);

    data->vertexArrayObject->bind();
    functions->glDrawArrays(GL_QUADS, 0, 4);
    functions->glFlush();
}


QSharedPointer<QOpenGLShaderProgram> VideoWidget::createAndCompileShaderProgram(QString programName)
{
    QSharedPointer<QOpenGLShaderProgram> result(new QOpenGLShaderProgram);

    QOpenGLShader *vertexShader = new QOpenGLShader(QOpenGLShader::Vertex, result.data());
    readAndCompileShaderFile(vertexShader, programName + ".vert");

    QOpenGLShader *fragmentShader = new QOpenGLShader(QOpenGLShader::Fragment, result.data());
    readAndCompileShaderFile(fragmentShader, programName + ".frag");

    result->addShader(vertexShader);
    result->addShader(fragmentShader);

    if (!result->link()) {
        throw std::exception("Failed to link shader program");
    }

    qDebug() << this << "Shader program" << programName << "linked succesfully";

    return result;
}


void VideoWidget::buildVertexBuffer(QSharedPointer<CombinedFilterData> data)
{
    static const float coords[12] = { 0, 0, 0 ,
                                      1, 0, 0,
                                      1, 1, 0 ,
                                      0, 1, 0  };

    data->vertexArrayObject.reset(new QOpenGLVertexArrayObject);
    data->vertexArrayObject->bind();

    data->vertexBuffer = QSharedPointer<QOpenGLBuffer>::create(QOpenGLBuffer::VertexBuffer);
    data->vertexBuffer->create();
    data->vertexBuffer->bind();
    data->vertexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
    data->vertexBuffer->allocate(coords, 12 * sizeof(float));

    data->filtersProgram->setAttributeBuffer(0, GL_FLOAT, 0, 3);
    data->filtersProgram->enableAttributeArray(0);

    data->vertexArrayObject->release();
}

QSharedPointer<QOpenGLTexture> VideoWidget::createTexture(QSize size,
                                                                 InterpolationType interpolationType,
                                                                 QOpenGLTexture::TextureFormat format)
{

    auto result = QSharedPointer<QOpenGLTexture>::create(QOpenGLTexture::Target2D);
    result->setSize(size.width(), size.height());
    result->setFormat(format);

    result->setMagnificationFilter(interpolationType == InterpolationType::Nearest ? QOpenGLTexture::Nearest : QOpenGLTexture::Linear);
    result->setMinificationFilter(interpolationType == InterpolationType::Nearest ? QOpenGLTexture::Nearest : QOpenGLTexture::Linear);

    if (!result->create())
        throw new std::exception("Failed to create texture");

    result->allocateStorage();

    return result;
}

void VideoWidget::readAndCompileShaderFile(QOpenGLShader* shader, QString shaderFileName)
{
    QFile shaderFile(shaderFileName);
    if (!shaderFile.open(QIODevice::ReadOnly)) {
        throw new std::exception("Cannot open shader file");
    }

    if (!shader->compileSourceFile(shaderFileName))
    {
        qCritical() << this << "Failed to compile shader" << shaderFileName << "with following error:";
        qCritical() << this << shader->log();
        throw new std::exception(shader->log().toStdString().c_str());
    }
    else
    {
        qDebug() << this << "Shader" << shaderFileName << "compiled succesfully";
    }
}
