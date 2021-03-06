#pragma once

#include <QtWidgets/QListWidget>

#include <qrkernel/ids.h>

#include "qrgui/mainWindow/error.h"

namespace qReal {
	class MainWindow;
}

class ErrorListWidget : public QListWidget
{
	Q_OBJECT

public:
	explicit ErrorListWidget(QWidget *parent = nullptr);
	void init(qReal::MainWindow* mainWindow);

signals:
	void clearRequested();

private slots:
	void highlightElement(QListWidgetItem * const item);
	void copyCurrentItem();
	void showContextMenu(const QPoint &pos);

private:

	void initContextMenu();

	qReal::MainWindow* mMainWindow;
	QMenu *mContextMenu;
};

