#pragma once
#ifndef PROJECTNAMEVALIDATOR_H
#define PROJECTNAMEVALIDATOR_H

#include <QValidator>

namespace capture {
namespace components {

class ProjectNameValidator : public QValidator
{
    Q_OBJECT
public:
    ProjectNameValidator(QObject * parent = nullptr);

    virtual State validate(QString &input, int &pos) const override;

signals:

    void invalidInputDetected() const;

private:
    QSet<QChar> m_invalidCharacters;
};

} // namespace components
} // namespace capture

#endif // PROJECTNAMEVALIDATOR_H
