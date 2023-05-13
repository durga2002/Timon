#pragma once

#include <QObject>
#include <QMutex>
#include "Common.h"

class Logic  : public QObject
{
	Q_OBJECT

public:
	Logic(QObject *parent);
	~Logic();

signals:
	void miniViewRequested();
	void maxiViewRequested();
	void statePublished(const QList<long>& timeCounters, int currentActivity, bool autoIdleEnabled);
	void exitRequested();

public slots:
	void userCame();
	void userLeft();
	void updateActivity(int activity);
	void userCommand(UserCommand cmd);

protected:
	virtual void timerEvent(QTimerEvent* e);

private:
	const QString m_sLogFileBaseName = "records.log";
	const int m_mouseActivityCheckIntervalMs = 3000;
	const int m_autoIdleTimeoutMinutes = 5;

	QList<long> m_timeCounters;
	int m_currentActivity;
	std::chrono::time_point<std::chrono::system_clock> m_lastTimeUpdated;
	QMutex m_mutex;
	bool m_autoIdleEnabled;

	void updateLog();
	void loadFromLog();
	void showMessage(QString msg);
	bool dayChanged(const std::tm& prev, const std::tm& now);
	std::tm midnight(const std::chrono::system_clock::time_point& tp);
};
