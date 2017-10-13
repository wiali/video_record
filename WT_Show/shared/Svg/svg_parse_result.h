#ifndef SVGPARSERESULT_H
#define SVGPARSERESULT_H

#include <qpolygon.h>
#include <qsize.h>

struct ContoursParseResult
{
    QSize size;
    QPolygon contours;
    bool success = false;
    int warnings = 0;
};

#endif // SVGPARSERESULT_H
