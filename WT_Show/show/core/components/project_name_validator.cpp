#include "project_name_validator.h"

#include <global_utilities.h>

namespace capture {
namespace components {

ProjectNameValidator::ProjectNameValidator(QObject *parent)
    : QValidator(parent)
{
    // Based on https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247(v=vs.85).aspx#naming_conventions
    auto invalidCharacters = GlobalUtilities::applicationSettings()->value("project_name_invalid_characters", "<>:\"\\/|?*").toString();

    for(int i = 0; i < invalidCharacters.count(); i++)
    {
        m_invalidCharacters << invalidCharacters.at(i);
    }
}

QValidator::State ProjectNameValidator::validate(QString &input, int &pos) const
{
    Q_UNUSED(pos)
    State result = State::Acceptable;

    for(auto character: m_invalidCharacters)
    {
        result = input.contains(character) ? State::Invalid : result;
    }

    if (result != State::Acceptable)
    {
        emit invalidInputDetected();
    }

    return result;
}

} // namespace components
} // namespace capture
