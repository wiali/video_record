#include "mygraphicsscene.h"
#include <QPaintEngine>
#include <Windows.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <qopengl.h>
#include <QMatrix4x4>
#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
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
    if (painter->paintEngine()->type()
            != QPaintEngine::OpenGL2) {
        qWarning("OpenGLScene: drawBackground needs a "
                 "QGLWidget to be set as viewport on the "
                 "graphics view");
        return;
    }


    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
           glColor3f(0.4f, 1.0f, 0.0f);
           glRectf(-0.75f,0.75f, 0.75f, -0.75f);


}
