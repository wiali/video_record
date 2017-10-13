#pragma once
#ifndef COMMANDLINEPARAMETERS_H
#define COMMANDLINEPARAMETERS_H

#include <QString>

namespace capture {
namespace model {

struct CommandLineParameters {
public:
    QString launchedFrom;
    QString userString;
};

} // namespace model
} // namespace capture

#endif // COMMANDLINEPARAMETERS_H
