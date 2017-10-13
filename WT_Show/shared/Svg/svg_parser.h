#ifndef SVGPARSER_H
#define SVGPARSER_H

#include "shared_global.h"
#include "svg_parse_result.h"

#include <QObject>

class SHAREDSHARED_EXPORT SvgParser
{
public:
    SvgParser();

    static ContoursParseResult parseContours(QByteArray& bytes);
};

#endif // SVGPARSER_H
