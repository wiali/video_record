#include "contours_parser.h"

ContoursParser::ContoursParser()
{
}

int ContoursParser::parseNumber(QString s)
{
    bool ok;
    int n = s.toInt(&ok);
    if (!ok)
    {
        qWarning() << "Unable to convert string to integer" + s;
        m_result.warnings++;
    }
    return n;
}

void ContoursParser::parsePoints(const QString& points)
{
    m_result.contours.clear();

    QStringList list = points.split(' ', QString::SkipEmptyParts);
    for (QString point : list)
    {
        QStringList coords = point.split(',');
        if (coords.size() != 2)
        {
            qWarning() << "Failed to parse SVG point" << point;
            m_result.warnings++;
        }

        bool okX, okY;
        QPoint p;
        p.setX(coords[0].toInt(&okX));
        p.setY(coords[1].toInt(&okY));
        if (!okX || !okY)
        {
            qWarning() << "Failed to parse SVG point" << point;
            m_result.warnings++;
        }
        m_result.contours.append(p);
    }
}

void ContoursParser::parseSvgSize(const QXmlAttributes& atts)
{
    // clear previous result
    m_result.size = QSize();

    // local variables
    QSize size;
    bool okH = false;
    bool okW = false;

    for (int i=0; i < atts.count(); i++)
    {
        if (atts.qName(i) == "height")
        {
            size.setHeight(parseNumber(atts.value(i)));
            okH = true;
        }
        else if (atts.qName(i) == "width")
        {
            size.setWidth(parseNumber(atts.value(i)));
            okW = true;
        }
    }

    if (okH && okW)
    {
        m_result.size = size;
    }
    else
    {
        qWarning() << "Failed to parse SVG element and extract size from it";
        m_result.warnings++;
    }
}

void ContoursParser::parsePolyline(const QXmlAttributes &atts)
{
    for (int i=0; i < atts.count(); i++)
    {
        if (atts.qName(i) == "points")
        {
            parsePoints(atts.value(i));
        }
    }
}


ContoursParseResult ContoursParser::result() const
{
    return m_result;
}
