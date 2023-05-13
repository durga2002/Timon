#pragma once

#include <QWidget>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QEnterEvent>
#include <iostream>
#include "ui_MiniView.h"
#include "Common.h"

class MiniView : public QWidget
{
	Q_OBJECT

public:
	MiniView(QWidget *parent = nullptr);
	~MiniView();

protected:
	virtual void enterEvent(QEnterEvent* event);

	// Periodically pop up the widget as it can be fully obscured by a taskbar (which is also always-on-top).
	virtual void timerEvent(QTimerEvent* e);

public slots:
	void updateState(const QList<long>& timeCounters, int currentActivity, bool autoIdleEnabled);
	void updateScreen(QScreen* pScr);
	void updatePosition();


signals:
	void userCame();
	void commandSent(UserCommand cmd);

private:
	Ui::MiniViewClass m_ui;
	QString m_ledStyle;
	QStringList m_activeLEDStyles;
	QStringList m_inactiveLEDStyles;
	QList<QWidget*> m_LEDs;
	QSystemTrayIcon* m_trayIcon;
	QMenu* m_trayIconMenu;
	QScreen* m_screen = nullptr;
};
