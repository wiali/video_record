#pragma once
#ifndef INK_BUTTON_H
#define INK_BUTTON_H

#include <QWidget>
#include <QPropertyAnimation>
#include <QPushButton>

#include "styled_color_dialog.h"
#include "color_button.h"

namespace Ui {
class InkButton;
}

namespace capture {
namespace mat {

class InkButton : public QWidget
{
    Q_OBJECT

public:
    explicit InkButton(QWidget* parent = 0);
    ~InkButton();

    bool rollState();
    QPushButton* inkBtn();
    ColorButton* colorBtn();
    QSharedPointer<StyledColorDialog> colorPicker();

public slots:
    void roll();
    void setInkClick(bool InkClick);
    void setModel(QSharedPointer<InkWidget> model);

protected slots:
    void moveEvent(QMoveEvent *event) override;
    void hideEvent(QHideEvent *event) override;

private slots:
    void setupAnimation();

    void on_ink_clicked();
    void on_handon_clicked(bool checked);  
    void on_brush_clicked();
    void on_eraser_clicked();
    void on_color_button_clicked(bool checked);

    void on_color_changed(const QColor &color);
    void on_size_changed(int size);

private:
    int  m_rollSpeed;
    bool m_inkClick;
    bool m_rolled;
    QSharedPointer<StyledColorDialog> m_colorPicker;
    QSharedPointer<InkWidget> m_inkCanvas;
    QPropertyAnimation* m_rollAnimations;
    QPropertyAnimation* m_unrollAnimations;
    Ui::InkButton *ui;
};

} // namespace mat
} // namespace capture

#endif // INK_BUTTON_H
