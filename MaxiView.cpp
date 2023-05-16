#include "MaxiView.h"
#include <QtWidgets/QApplication>
#include <QScreen>
#include <QGraphicsLayout>
#include <QSizePolicy>
#include <QLabel>
#include "Common.h"

MaxiView::MaxiView(QWidget *parent)
	: QFrame(parent),
    m_timer(0)
{
	m_ui.setupUi(this);
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::SubWindow);

    const QString sActButtonStyle = "\
        QPushButton \
        { \
            background-color: %1;\
            color: white;\
            font: bold;\
        } \
        QPushButton:checked \
        { \
            background-color: %2;\
            border: 2px solid white;\
            color: white;\
        }";

    m_ui.act1Button->setStyleSheet(sActButtonStyle.arg(styles[ACTIVITY_1].darker().name()).arg(styles[ACTIVITY_1].name()));
    m_ui.act2Button->setStyleSheet(sActButtonStyle.arg(styles[ACTIVITY_2].darker().name()).arg(styles[ACTIVITY_2].name()));
    m_ui.idleButton->setStyleSheet(sActButtonStyle.arg(styles[IDLE].darker().name()).arg(styles[IDLE].name()));

    m_ui.activitiesButtonsGroup->setId(m_ui.act1Button, ACTIVITY_1);
    m_ui.activitiesButtonsGroup->setId(m_ui.act2Button, ACTIVITY_2);
    m_ui.activitiesButtonsGroup->setId(m_ui.idleButton, IDLE);

    m_ui.idleButton->setText(activityNames[IDLE]);    

    m_ui.idleButton->click(); 
    connect(m_ui.activitiesButtonsGroup, &QButtonGroup::idClicked, this, &MaxiView::activityChanged);
    connect(qApp, &QApplication::primaryScreenChanged, this, &MaxiView::updateScreen);
}

void MaxiView::showEvent(QShowEvent* event)
{
    // automatically minimize after timeout
    const int timeoutMs = 6000;
    m_timer = startTimer(timeoutMs);

    QWidget::showEvent(event);
}

void MaxiView::timerEvent(QTimerEvent* e)
{
    leave();
}

void MaxiView::leaveEvent(QEvent* event)
{
    leave();
}

void MaxiView::leave()
{
    if (m_timer)
    {
        killTimer(m_timer);
        m_timer = 0;
    }
    emit userLeft();
}

void MaxiView::updateState(const QList<long>& timeCounters, int newActivity, bool autoIdleEnabled)
{
    if (m_ui.activitiesButtonsGroup->checkedId() != newActivity)
    {
        m_ui.activitiesButtonsGroup->buttons()[newActivity]->click();
    }

    m_ui.act1Button->setText(activityNames[ACTIVITY_1] + "\n" + QString::number(timeCounters[ACTIVITY_1] / 3600.0f, 'f', 1));
    m_ui.act2Button->setText(activityNames[ACTIVITY_2] + "\n" + QString::number(timeCounters[ACTIVITY_2] / 3600.0f,'f', 1));

    setStyleSheet(QString("QFrame {background-color: %1; border: 1px solid %2;}")
        .arg(backgroundColor.lighter().name())
        .arg(autoIdleEnabled ? "white" : "red"));
}

void MaxiView::updateScreen(QScreen* pScr)
{
    qDebug() << "updateScreen: " << pScr;
    disconnect(m_screen, &QScreen::geometryChanged, this, &MaxiView::updatePosition);
    m_screen = pScr;
    connect(m_screen, &QScreen::geometryChanged, this, &MaxiView::updatePosition);
    updatePosition();
}
void MaxiView::updatePosition()
{
    qDebug() << "updatePosition " << m_screen->size() << m_screen->availableGeometry();

    const QSize screenRect = m_screen->size();
    move(screenRect.width() - frameGeometry().width(), screenRect.height() - frameGeometry().height());
}
