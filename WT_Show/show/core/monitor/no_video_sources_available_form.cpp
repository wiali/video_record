#include "no_video_sources_available_form.h"
#include "ui_no_video_sources_available_form.h"

namespace capture {
namespace monitor {

NoVideoSourcesAvailableForm::NoVideoSourcesAvailableForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NoVideoSourcesAvailableForm)
{
    ui->setupUi(this);
}

NoVideoSourcesAvailableForm::~NoVideoSourcesAvailableForm() {}

} // namespace monitor
} // namespace capture
