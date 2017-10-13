#ifndef CONTOURSPARSER_H
#define CONTOURSPARSER_H

#include "svg_parse_result.h"

#include <QPolygon>
#include <qtxml/QXmlSimpleReader>

class ContoursParser
{
public:
    ContoursParser();

    void parsePoints(const QString &points);
    void parseSvgSize(const QXmlAttributes &atts);
    void parsePolyline(const QXmlAttributes &atts);


    ContoursParseResult result() const;

protected:
    // helper functions
    int parseNumber(QString s);

    ContoursParseResult m_result;

};

#endif // CONTOURSPARSER_H
