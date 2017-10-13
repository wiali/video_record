#ifndef SVGXMLHANDLER_H
#define SVGXMLHANDLER_H

#include "contours_parser.h"

#include <qtxml/QXmlSimpleReader>

class SvgXmlHandler : public QXmlContentHandler, public QXmlErrorHandler
{
public:
    SvgXmlHandler(ContoursParser& contoursParser);

    // QXmlErrorHandler interface
public:
    bool warning(const QXmlParseException &exception);
    bool error(const QXmlParseException &exception);
    bool fatalError(const QXmlParseException &exception);
    QString errorString() const;

    // QXmlContentHandler interface
public:
    void setDocumentLocator(QXmlLocator *locator);
    bool startDocument();
    bool endDocument();
    bool startPrefixMapping(const QString &prefix, const QString &uri);
    bool endPrefixMapping(const QString &prefix);
    bool startElement(const QString &namespaceURI, const QString &localName, const QString &qName, const QXmlAttributes &atts);
    bool endElement(const QString &namespaceURI, const QString &localName, const QString &qName);
    bool characters(const QString &ch);
    bool ignorableWhitespace(const QString &ch);
    bool processingInstruction(const QString &target, const QString &data);
    bool skippedEntity(const QString &name);

protected:
    ContoursParser& contoursParser;
};

#endif // SVGXMLHANDLER_H
