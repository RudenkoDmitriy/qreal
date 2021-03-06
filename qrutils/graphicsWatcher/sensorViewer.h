#pragma once

#include <QtWidgets/QGraphicsView>
#include <QtCore/QTimer>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QToolTip>

#include "keyPoint.h"
#include "pointsQueueProcessor.h"

namespace utils {
namespace sensorsGraph {

/// An area with real time plotting
class SensorViewer : public QGraphicsView
{
	Q_OBJECT

public:
	explicit SensorViewer(QWidget *parent = 0);
	~SensorViewer();

	/// @note All intervals in ms
	void configureUserOptions(const int &fpsDelay, const int &autoScaleDelay, const int &textInfoUpdateDelay);

	void resizeEvent(QResizeEvent *event = 0);

public slots:
	void setNextValue(const qreal newValue);
	void startJob();
	void stopJob();
	void clear();
	void zoomIn();
	void zoomOut();
	void onSensorChange();

	/// Save sensor's values history into the ".csv" file.
	void exportHistory();

protected:
	void drawNextFrame();
	void drawBackground(QPainter *painter, const QRectF &rect);
	/// Renders hint with value under cursor
	void mouseMoveEvent(QMouseEvent *event);
	void leaveEvent(QEvent *);
	void mouseDoubleClickEvent(QMouseEvent *event);

	void initGraphicsOutput();

private slots:
	void visualTimerEvent();

private:
	QGraphicsScene *mScene;
	QTimer mVisualTimer;
	KeyPoint *mMainPoint;
	KeyPoint *mMarker;
	PointsQueueProcessor *mPointsDataProcessor;
	QBrush mPenBrush;

	/// Output update delay in ms
	int mFpsInterval;
	/// Time in milliseconds
	int mAutoScaleInterval;
	/// Update interval of current value
	int mUpdateTextInfoInterval;
	/// Shift in pixels each frame
	static const int stepSize = 2;
	/// Zoom coefficent
	static const int zoomRate = 2;

	int mScaleCoefficient;
	int mAutoScaleTimer;
	int mUpdateCurrValueTimer;
	qreal mOutputValue;
};

}
}

