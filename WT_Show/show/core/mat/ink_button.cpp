#include "ink_button.h"
#include "ui_ink_button.h"

#include <QPainter>
#include <QRect>

namespace capture {
namespace mat {

InkButton::InkButton(QWidget* parent)
    : QWidget(parent)
    , m_rollSpeed(300)
    , m_inkClick(true)
    , m_rolled(true)
    , m_colorPicker(nullptr)
    , ui(new Ui::InkButton)
{
    ui->setupUi(this);

    this->resize(this->width(), 64);
}

InkButton::~InkButton()
{
    delete ui;
}

void InkButton::setModel(QSharedPointer<InkWidget> model)
{
    m_inkCanvas = model;
    m_colorPicker.reset(new StyledColorDialog(m_inkCanvas, ui->color, this->window()));
    m_colorPicker->hide();

    ui->brush->click();

    connect(ui->color, &ColorButton::toggled, this, &InkButton::on_color_button_clicked);
    connect(m_colorPicker.data(), &StyledColorDialog::colorChanged, this, &InkButton::on_color_changed);
    connect(m_colorPicker.data(), &StyledColorDialog::sizeChanged, this, &InkButton::on_size_changed);
}

void InkButton::setupAnimation()
{
    m_unrollAnimations = new QPropertyAnimation(this, "geometry");
    m_unrollAnimations->setDuration(m_rollSpeed);
    m_unrollAnimations->setStartValue(QRect(x(), y(), width(), ui->ink->height() * 5));
    m_unrollAnimations->setEndValue(QRect(x(), y(), width(), ui->ink->height()));

    m_rollAnimations = new QPropertyAnimation(this, "geometry");
    m_rollAnimations->setDuration(m_rollSpeed);
    m_rollAnimations->setStartValue(QRect(x(), y(), width(), ui->ink->height()));
    m_rollAnimations->setEndValue(QRect(x(), y(), width(), ui->ink->height() * 5));
}

void InkButton::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event);
    ui->color->setChecked(false);
}

void InkButton::hideEvent(QHideEvent *event)
{
    m_colorPicker->hide();
    QWidget::hideEvent(event);
}

QPushButton* InkButton::inkBtn()
{
    return ui->ink;
}

ColorButton* InkButton::colorBtn()
{
    return ui->color;
}

QSharedPointer<StyledColorDialog> InkButton::colorPicker()
{
    return m_colorPicker;
}

bool InkButton::rollState()
{
    return m_rolled;
}

void InkButton::setInkClick(bool InkClick)
{
    m_inkClick = InkClick;
}

void InkButton::roll()
{
    m_rolled = !m_rolled;
    if(m_rolled)
    {
        setupAnimation();
        ui->color->setChecked(false);
        m_unrollAnimations->start();
    }
    else
    {
        if(y() > (this->parentWidget()->height() - 320))
        {
            this->move(x(), this->parentWidget()->height() - 320);
        }
        setupAnimation();
        m_rollAnimations->start();
    }
}

void InkButton::on_ink_clicked()
{
    if(m_inkClick)
    {
        roll();
    }
    ui->ink->setCheckable(true);
    ui->ink->setChecked(!m_rolled);
}

void InkButton::on_handon_clicked(bool checked)
{
    // Should we enable the touchmat here?
    m_inkCanvas->setPenMode(!checked);
    ui->color->setChecked(false);
}

void InkButton::on_brush_clicked()
{
    ui->brush->setChecked(true);
    ui->eraser->setChecked(false);
    ui->color->setChecked(false);

    m_inkCanvas->enterDrawMode();
}

void InkButton::on_eraser_clicked()
{
    ui->brush->setChecked(false);
    ui->eraser->setChecked(true);
    ui->color->setChecked(false);

    m_inkCanvas->enterEraserMode();
}

void InkButton::on_color_button_clicked(bool checked)
{
    if(x() > m_colorPicker->width() - 20)
    {
        m_colorPicker->setPositionMode(true);
    }
    else
    {
        m_colorPicker->setPositionMode(false);
    }

    int limitHeight = height() + m_colorPicker->height() - 38 - 32; // 38 for intitail offsety, 32 for half of icon size

    if(y() > this->parentWidget()->height() - limitHeight)
    {
        int offset = limitHeight - (this->parentWidget()->height() - y()) + 38;
        m_colorPicker->setOffsetY(offset);
    }
    else
    {
        m_colorPicker->setOffsetY(38);
    }

    m_colorPicker->raise();
    m_colorPicker->setVisible(checked);
}

void InkButton::on_color_changed(const QColor &color)
{
    qDebug() << "Pen color = " << color;
    m_inkCanvas->colorChanged(color);
    m_inkCanvas->setPenPointColor(color);
    ui->color->setChecked(false);
    ui->color->setColor(color);
}

void InkButton::on_size_changed(int size)
{
    qDebug() << "Pen size = " << size;
    m_inkCanvas->penSizeChanged(size);
}

} // namespace mat
} // namespace capture
