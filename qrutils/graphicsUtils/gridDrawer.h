#pragma once

#include <QtGui/QPainter>

#include "qrutils/utilsDeclSpec.h"

namespace graphicsUtils {

class QRUTILS_EXPORT GridDrawer
{
public:
	GridDrawer();
	void drawGrid(QPainter *painter, const QRectF &rect, const int indexGrid);
};

}
