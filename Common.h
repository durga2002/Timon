#pragma once

#include <QColor>
#include <QList>
#include <QStringList>

enum {
	ACTIVITY_1,
	ACTIVITY_2,
	IDLE,
	NUM_ACTIVITIES
};

enum UserCommand {
	ENABLE_AUTO_IDLE,
	DISABLE_AUTO_IDLE,
	EXIT,
	NUM_USER_COMMANDS
};

extern float deemFactor; 

extern QList<QColor> styles;
extern QColor backgroundColor;
extern QStringList activityNames;
