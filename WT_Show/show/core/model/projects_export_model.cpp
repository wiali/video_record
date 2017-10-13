#include "projects_export_model.h"

#include <QDebug>

#include "ink_data.h"

namespace capture {
namespace model {

ProjectsExportModel::ProjectsExportModel(QObject *parent)
    : QObject(parent)
    , m_state(NotExporting)
    , m_count(0)
    , m_index(0) {
    // Needed for proper cross-thread signals
    static struct Initialize {
        Initialize() {
            qRegisterMetaType<capture::model::ProjectsExportModel::State>();
            qRegisterMetaType<capture::model::ProjectsExportModel::Format>();
        }
    } initialize;
}

ProjectsExportModel::State ProjectsExportModel::state() const { return m_state; }
int ProjectsExportModel::count() const { return m_count; }
int ProjectsExportModel::index() const { return m_index; }
ProjectsExportModel::Format ProjectsExportModel::format() const { return m_format; }

void ProjectsExportModel::setState(model::ProjectsExportModel::State state) {
    if (m_state != state) {
        m_state = state;

        qInfo() << this << "State changed to" << state;

        emit stateChanged(m_state);
    }
}

void ProjectsExportModel::setCount (int count) {
    if (m_count != count) {
        m_count = count;

        qInfo() << this << "Count changed to" << count;

        emit countChanged(m_count);
    }
}

void ProjectsExportModel::setIndex (int index) {
    if (m_index != index) {
        m_index = index;

        qInfo() << this << "Index changed to" << index;

        emit indexChanged(m_index);
    }
}

void ProjectsExportModel::setFormat(model::ProjectsExportModel::Format format) {
    if (format != m_format) {
        m_format = format;

        qInfo() << this << "Format changed to" << format;

        emit formatChanged(m_format);
    }
}

} // namespace model
} // namespace capture

