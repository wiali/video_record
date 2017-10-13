#ifndef CAPTURE_WORKTOOL_H
#define CAPTURE_WORKTOOL_H

#include <QSharedDataPointer>
#include "model/application_state_model.h"

namespace capture {

class CaptureWorktool
{
public:
    CaptureWorktool();
    int exec(int argc, char **argv);

    inline QSharedPointer<model::ApplicationStateModel> model() const { return m_model; }

protected:
    virtual QSharedPointer<model::ApplicationStateModel> createModel();

protected:
    virtual bool singleScreenMode();

private:
    bool parseCommandLine(int argc, char *argv[], int* exitCode);    

     QSharedPointer<model::ApplicationStateModel> m_model;
};

} // namespace capture

#endif // CAPTURE_WORKTOOL_H
