#include <QColor>
#include <QList>

QList<QColor> styles = {
    QColor("green").lighter(),
    QColor("red"),
    QColor("gray")
};

QColor backgroundColor = QColor(28, 28, 28);

QStringList activityNames = {"GE", "Private", "IDLE"};

float deemFactor = 0.5f;