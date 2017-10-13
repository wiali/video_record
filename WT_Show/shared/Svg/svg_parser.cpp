#include "svg_parser.h"
#include "svg_xml_handler.h"

#include <qtxml/QXmlSimpleReader>

SvgParser::SvgParser()
{
}

ContoursParseResult SvgParser::parseContours(QByteArray &bytes)
{
    QXmlInputSource *source = new QXmlInputSource();
    source->setData(bytes);

    QXmlSimpleReader xmlReader;
    ContoursParser parser;
    SvgXmlHandler handler(parser);

    xmlReader.setContentHandler(&handler);
    xmlReader.setErrorHandler(&handler);
    bool xmlOk = xmlReader.parse(source);

    auto result = parser.result();
    if (!xmlOk || result.contours.size() == 0|| result.size.isValid())
    {
        qWarning() << "Failed to parse contours";
        result.success = false;
    }
    else
    {
        result.success = true;
    }

    return result;
}
