#pragma once

#include <QWidget>
#include "ui_MaxiView.h"


class MaxiView : public QFrame
{
	Q_OBJECT

public:
	MaxiView(QWidget *parent = nullptr);
	~MaxiView() {};

protected:
	virtual void leaveEvent(QEvent* event) override;
	virtual void showEvent(QShowEvent* event) override;
	virtual void timerEvent(QTimerEvent* e) override;

public slots:
	void updateState(const QList<long>& timeCounters, int newActivity, bool autoIdleEnabled);
	void updateScreen(QScreen* pScr);
	void updatePosition();

signals:
	void userLeft();
	void activityChanged(int activity);

private:
	Ui::MaxiViewClass m_ui;
	QScreen* m_screen = nullptr;
	int m_timer;
};
