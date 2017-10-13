#include "svg_xml_handler.h"

SvgXmlHandler::SvgXmlHandler(ContoursParser &contoursParser)
    : contoursParser(contoursParser)
{
}

void SvgXmlHandler::setDocumentLocator(QXmlLocator *locator)
{
    Q_UNUSED(locator);
}

bool SvgXmlHandler::startDocument()
{
    return true;
}

bool SvgXmlHandler::endDocument()
{
    return true;
}

bool SvgXmlHandler::startPrefixMapping(const QString &prefix, const QString &uri)
{
    Q_UNUSED(prefix);
    Q_UNUSED(uri);
    return true;

}

bool SvgXmlHandler::endPrefixMapping(const QString &prefix)
{
    Q_UNUSED(prefix);
    return true;
}

bool SvgXmlHandler::startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);
    if (qName == "svg")
    {
        contoursParser.parseSvgSize(atts);
    }
    else if (qName == "polyline")
    {
        contoursParser.parsePolyline(atts);
    }
    return true;
}


bool SvgXmlHandler::endElement(const QString &namespaceURI, const QString &localName, const QString &qName)
{
    Q_UNUSED(namespaceURI);
    Q_UNUSED(localName);
    Q_UNUSED(qName);
    return true;
}

bool SvgXmlHandler::characters(const QString &ch)
{
    Q_UNUSED(ch);
    return true;
}

bool SvgXmlHandler::ignorableWhitespace(const QString &ch)
{
    Q_UNUSED(ch);
    return true;
}

bool SvgXmlHandler::processingInstruction(const QString &target, const QString &data)
{
    Q_UNUSED(target);
    Q_UNUSED(data);
    return true;
}

bool SvgXmlHandler::skippedEntity(const QString &name)
{
    Q_UNUSED(name);
    return true;
}
QString SvgXmlHandler::errorString() const
{
    return "";
}

bool SvgXmlHandler::warning(const QXmlParseException &exception)
{
    Q_UNUSED(exception);
    return true;
}
bool SvgXmlHandler::error(const QXmlParseException &exception)
{
    Q_UNUSED(exception);
    return true;
}

bool SvgXmlHandler::fatalError(const QXmlParseException &exception)
{
    Q_UNUSED(exception);
    return true;
}
