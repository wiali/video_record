#ifndef MYGRAPHICSSCENE_H
#define MYGRAPHICSSCENE_H

#include <QGraphicsScene>
#include <QWidget>
#include <QPainter>
#include <QRectF>
#include  <QGraphicsTextItem>

class myGraphicsscene : public QGraphicsScene
{
    Q_OBJECT
public:
    myGraphicsscene(QWidget* parent = 0);
    virtual ~myGraphicsscene();
protected:
    void drawBackground(QPainter *painter,
                                         const QRectF &);
private:
    QGraphicsTextItem* mTextItem;
};

#endif // MYGRAPHICSSCENE_H
