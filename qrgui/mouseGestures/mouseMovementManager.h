#pragma once

#include <qrkernel/ids.h>

#include <QtCore/QMap>
#include <QtWidgets/QWidget>

#include "qrgui/mouseGestures/mouseGesturesDeclSpec.h"

#include "private/keyManager.h"
#include "private/mixedgesturesmanager.h"
#include "private/geometricForms.h"

#include "plugins/pluginManager/editorManagerInterface.h"

namespace qReal {
namespace gestures {

class QRGUI_MOUSE_GESTURES_EXPORT MouseMovementManager : public QObject
{
	Q_OBJECT

public:
	/// Specifies the type of the result of the gesture made by user: regular element creation or some metagesture
	/// (e.g. element deletion).
	enum GestureResultType
	{
		invalidGesture
		, createElementGesture
		, deleteGesture
	};

	/// Specifies the result of the gesture made.
	struct QRGUI_MOUSE_GESTURES_EXPORT GestureResult
	{
	public:
		GestureResult();
		explicit GestureResult(GestureResultType type, const Id &id = Id());

		/// Returns the resulting type of the gesture.
		GestureResultType type() const;

		/// Returns the type of the element requested by the gesture.
		Id elementType() const;

		/// Returns the resulting type of the gesture.
		void setType(GestureResultType type);

		/// Sets the type of the element requested by the gesture.
		void setElementType(const Id &id);

	private:
		GestureResultType mType;
		Id mId;
	};

	MouseMovementManager(const Id &diagram, const EditorManagerInterface &editorManagerInterface);

	/// Creates and returns the widget that shows available for this manager mouse gestures.
	/// Transfers ownership.
	QWidget *producePainter() const;

	void initializeGestures();
	void mousePress(const QPointF &point);
	void mouseMove(const QPointF &point);
	void clear();
	GestureResult result();
	static PathVector stringToPath(const QString &str);
	QPointF pos();
	QPointF firstPoint();
	QPointF lastPoint();
	QLineF newLine();
	bool wasMoving();
	bool isEdgeCandidate();
	bool pathIsEmpty();

private:
	static QPoint parsePoint(const QString &str);
	void recountCentre();
	void drawIdealPath();

	const Id mDiagram;
	const EditorManagerInterface &mEditorManagerInterface;
	PathVector mPath;
	QPointF mCenter;
	QSet<Id> mInitializedGestures;
	QScopedPointer<KeyManager> mKeyStringManager;
	QScopedPointer<GesturesManager> mGesturesManager;
};

}
}
