#include "export_widget.h"
#include "ui_export_widget.h"

#include <QDebug>
#include <QDir>
#include <QFont>
#include <QFontMetrics>

#include <global_utilities.h>

#include "common/utilities.h"
#include "model/export_image_model.h"
#include "event/export_projects_event.h"

namespace capture {
namespace monitor {

ExportWidget::ExportWidget(QWidget *parent)
    : RightSubMenuBaseWidget(parent)
    , ui(new Ui::ExportWidget) {
    ui->setupUi(this);

    const int maxTextWidth = calculateMaxTextWidth();
    QSize newSize(size());

    if (maxTextWidth > 170) { // 170 pixels is max text width in ui file.
        qDebug() << "The export text is too long, resize the width, the old text size is" << maxTextWidth;
        newSize.setWidth(80 + maxTextWidth); // 80 = 30 + 35 + 15; 30 for border, 35 for left pedding, 15 for right pedding
    }

    if (!GlobalUtilities::applicationSettings("ocr")->value("is_enabled", false).toBool()) {
        newSize.setHeight(height() - ui->ocrToPDFButton->height());
        ui->ocrToPDFButton->hide();
        ui->frame->resize(ui->frame->width(), ui->frame->height() - ui->ocrToPDFButton->height());
    }

    resize(newSize);
    setContent(ui->frame);
}

void ExportWidget::setModel(QSharedPointer<model::ApplicationStateModel> model) { m_model = model; }
int ExportWidget::getThisArrowTop() const { return ui->ocrToPDFButton->isVisible() ? 355 : 275; }

void ExportWidget::moveToTarget() {
    QPoint targetInGlobal = m_target->parentWidget()->mapToGlobal(m_target->pos());
    QPoint targetInParent = parentWidget()->mapFromGlobal(targetInGlobal);

    const int arrowPosX = targetInParent.x() + m_target->width() - (ARROW_WIDTH + BORDER_SIZE) - 322;
    const int arrowPosY = (targetInParent.y() - getThisArrowTop()) + (ARROW_HEIGHT / 2) - 10;

    move(arrowPosX, arrowPosY);
    update();
}

void ExportWidget::paintBorderWithArrow(QPaintEvent*) {
    if(!m_content) {
        qWarning() << "No content was set in the left submenu" << this;
        return;
    }

    QPen pen;
    pen.setColor(BORDER_COLOR);
    pen.setWidth(BORDER_SIZE);
    pen.setCapStyle(Qt::SquareCap);
    pen.setJoinStyle(Qt::MiterJoin);
    pen.setCosmetic(true);
    QPainter painter(this);
    painter.setRenderHints(QPainter::HighQualityAntialiasing);
    painter.setPen(pen);

    QPoint topLeft(m_content->geometry().topLeft().x() - BORDER_SIZE, m_content->geometry().topLeft().y() - BORDER_SIZE);
    QPoint topRight(m_content->geometry().topRight().x() + BORDER_SIZE, topLeft.y());
    QPoint bottomLeft(topLeft.x(), topLeft.y() + m_content->height() + (BORDER_SIZE * 2));
    QPoint bottomRight(topRight.x(), bottomLeft.y());

    painter.drawLine(topRight, bottomRight);
    painter.drawLine(topLeft, topRight);
    painter.drawLine(bottomLeft, bottomRight);

    QPoint leftLineToArrow(topLeft.x(), topLeft.y() + this->getThisArrowTop());
    painter.drawLine(topLeft, leftLineToArrow);

    QPoint leftLineFromArrow( topLeft.x(), this->getThisArrowTop() + ARROW_HEIGHT);
    painter.drawLine(leftLineFromArrow, bottomLeft);

    painter.drawLine(leftLineToArrow, QPoint(leftLineToArrow.x() - ARROW_WIDTH, (leftLineToArrow.y() + leftLineFromArrow.y()) / 2));
    painter.drawLine(QPoint(leftLineToArrow.x() - ARROW_WIDTH, (leftLineToArrow.y() + leftLineFromArrow.y()) / 2), leftLineFromArrow);
}

void ExportWidget::showEvent(QShowEvent* event) {
    setFocus();
    return RightSubMenuBaseWidget::showEvent(event);
}

void ExportWidget::focusOutEvent(QFocusEvent *event) {
    hide();

    QTimer::singleShot(500, this, [ this ] {
        if(m_target->property("checked").toBool())
        {
            m_target->setProperty("checked", false);
        }
    });

    return RightSubMenuBaseWidget::focusOutEvent(event);
}

void ExportWidget::exportAll(QString location, event::ExportProjectItemsEvent::Format format) {
    QVector<QSharedPointer<model::ExportImageModel>> items;

    foreach(auto project, m_model->projects()->items()) {
        items << model::ExportImageModel::fromStageProject(project);
    }

    auto exportImageEvent = new event::ExportProjectItemsEvent(format, location, items);
    exportImageEvent->dispatch();
}

void ExportWidget::on_saveAsPNGButton_clicked() {
    QString location = common::Utilities::saveDialog(common::Utilities::Image, tr("Save as PNG"), false);
    QDir dir(location);

    if (dir.exists() && location != "" && m_model->projects()->count() > 0) {
        exportAll(location, event::ExportProjectItemsEvent::PNG);
    }
    else {
        // ToDo: Error
    }
}

void ExportWidget::on_saveAsJPGButton_clicked() {
    QString location = common::Utilities::saveDialog(common::Utilities::Image, tr("Save as JPG"), false);
    QDir dir(location);

    if (dir.exists() && location != "" && m_model->projects()->count() > 0) {
        exportAll(location, event::ExportProjectItemsEvent::JPG);
    }
}

void ExportWidget::on_exportToPDFButton_clicked() {
    auto now = QDateTime::currentDateTime();
    auto filename = QString(tr("HP_DOC_%1_%2.pdf")).arg(now.toString("yyyyMMdd")).arg(now.toString("HHmmss"));

    QString fileName = common::Utilities::saveDialog(common::Utilities::PDF , tr("Export all images to PDF"), true,
                                                     tr("PDF Files(*.pdf)"), nullptr, filename);

    if (fileName != "" && m_model->projects()->count() > 0) {
        exportAll(fileName, event::ExportProjectItemsEvent::PDF);
    }
}

void ExportWidget::on_ocrToPDFButton_clicked() {
    auto now = QDateTime::currentDateTime();
    auto filename = QString(tr("HP_DOC_%1_%2.pdf")).arg(now.toString("yyyyMMdd")).arg(now.toString("HHmmss"));

    QString fileName = common::Utilities::saveDialog(common::Utilities::PDF , tr("Export current document with OCR to PDF"), true,
                                                     tr("PDF Files(*.pdf)"), nullptr, filename);

    if (fileName != "" && m_model->projects()->count() > 0) {
        exportAll(fileName, event::ExportProjectItemsEvent::OCR);
    }
}

void ExportWidget::on_sendAllToStageButton_clicked() {
    qDebug() << "Send all to stage click";

    auto exportEvent = new event::ExportProjectsEvent(m_model->projects()->items());
    exportEvent->dispatch();
}

int ExportWidget::calculateMaxTextWidth() {
    int maxWidth = -1;

    for(auto child : ui->frame->children()) {
        if (auto button = qobject_cast<QAbstractButton*>(child)) {
            if (button->isVisible()) {
                const auto width = common::Utilities::measureButtonTextWidth(button);
                maxWidth = width > maxWidth ? width : maxWidth;
            }
        }
    }

    return maxWidth;
}

} // namespace monitor
} // namespace capture
