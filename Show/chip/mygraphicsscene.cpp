#include "mygraphicsscene.h"
#include <QPaintEngine>
#include <Windows.h>
#include <qopengl.h>
#include <QMatrix4x4>
#include <QOpenGLFunctions>


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

myGraphicsscene::myGraphicsscene(QWidget* parent)
    : QGraphicsScene(parent)
    , mTextItem(0)
{
    mTextItem = new QGraphicsTextItem("Sample Text");
    mTextItem->setTextInteractionFlags(Qt::TextEditorInteraction);
    addItem(mTextItem);
    mTextItem->setPos(sceneRect().width()/2, sceneRect().height()/2);
}

myGraphicsscene::~myGraphicsscene()
{

}

void myGraphicsscene::drawBackground(QPainter *painter,
                                     const QRectF &)
{
    if (painter->paintEngine()->type() != QPaintEngine::OpenGL2)
    {
        qWarning("OpenGLScene: drawBackground needs a "
                 "QGLWidget to be set as viewport on the "
                 "graphics view");
        return;
    }

    auto f = QOpenGLContext::currentContext()->functions();

    f->glClearColor(1, 0, 0, 1);
    f->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //f->glDrawArrays(GL_QUADS, 0.4f, 1.0f, 0.0f);
    //f->glRectf(-0.75f,0.75f, 0.75f, -0.75f);
}
