#include <QGuiApplication>
#include <QScreen>
#include "Common.h"
#include "MiniView.h"

extern QList<QColor> styles;

MiniView::MiniView(QWidget *parent)
	: QWidget(parent)
{
	m_ui.setupUi(this);
	setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::SubWindow);

	m_LEDs.push_back(m_ui.led1);
	m_LEDs.push_back(m_ui.led2);
	for (int activity = 0; activity < NUM_ACTIVITIES; activity++)
	{
		if (activity == IDLE)
		{
			continue;
		}

		QColor color = styles[activity];
		
		m_activeLEDStyles.append(
			QString("background-color: %1;").arg(color.name(QColor::HexArgb))
		);

		color.setAlphaF(deemFactor);

		m_inactiveLEDStyles.append(
			QString("background-color: %1; border: 1px solid %2;")
			.arg(color.name(QColor::HexArgb))
			.arg(backgroundColor.name())
		);
	}	

	setMouseTracking(true);

	const QString originalBGColor = palette().color(QPalette::Window).name();
	setStyleSheet(QString("background-color: %1;").arg(backgroundColor.name()));

	m_trayIconMenu = new QMenu(this);	
	m_trayIconMenu->setStyleSheet(QString("\
		QMenu\
	    {\
			background-color : %1;\
			color : black;\
		}\
		QMenu:selected\
		{\
			background-color : lightgray;\
			color : black;\
		}").arg(originalBGColor));	

	QAction* autoIdleAction = new QAction("&Auto-IDLE", this);
	autoIdleAction->setCheckable(true);
	autoIdleAction->setChecked(true);
	m_trayIconMenu->addAction(autoIdleAction);
	connect(autoIdleAction, &QAction::toggled, [this](bool checked) {emit this->commandSent(checked ? ENABLE_AUTO_IDLE : DISABLE_AUTO_IDLE); });

	QAction* quitAction = new QAction("&Quit", this);
	m_trayIconMenu->addAction(quitAction);
	connect(quitAction, &QAction::triggered, [this]() {emit this->commandSent(EXIT);});

	m_trayIcon = new QSystemTrayIcon(QIcon("icon16.png"), this);
	m_trayIcon->setContextMenu(m_trayIconMenu);
	m_trayIcon->show();

	// Periodically pop up the widget as it can be fully obscured by a taskbar (which is also always-on-top).
	const int popUpIntervalMs = 2000;
	startTimer(popUpIntervalMs);

	connect(qApp, &QApplication::primaryScreenChanged, this, &MiniView::updateScreen);
}

MiniView::~MiniView()
{
	m_trayIcon->hide();
}

void MiniView::enterEvent(QEnterEvent* event)
{
	emit userCame();
}

// Periodically pop up the widget as it can be fully obscured by a taskbar (which is also always-on-top).
void MiniView::timerEvent(QTimerEvent* e)
{
	if (isVisible())
	{
		raise();
	}
}

void MiniView::updateState(const QList<long>& notUsed, int currentActivity, bool autoIdleEnabled)
{
	for (int act = 0; act < NUM_ACTIVITIES; act++)
	{
		if (act != IDLE)
		{
			m_LEDs[act]->setStyleSheet(act == currentActivity ? m_activeLEDStyles[act] : m_inactiveLEDStyles[act]);
		}
	}
}

void MiniView::updateScreen(QScreen* pScr)
{
	qDebug() << "updateScreen: " << pScr;
	disconnect(m_screen, &QScreen::geometryChanged, this, &MiniView::updatePosition);
	m_screen = pScr;
	connect(m_screen, &QScreen::geometryChanged, this, &MiniView::updatePosition);
	updatePosition();
}

void MiniView::updatePosition()
{

	int taskbarHeight = m_screen->size().height() - m_screen->availableGeometry().height() - 1;
	if (taskbarHeight < 10)
	{
		taskbarHeight = 39;
	}
	const QSize panelSize = QSize(8, taskbarHeight);
	const QSize ledSize = QSize(6, taskbarHeight / 2 - 3);

	m_ui.led1->setMinimumSize(ledSize);
	m_ui.led1->setMaximumSize(ledSize);
	m_ui.led2->setMinimumSize(ledSize);
	m_ui.led2->setMaximumSize(ledSize);
	resize(panelSize);

	const QSize screenRect = m_screen->size();
	move(screenRect.width() - frameGeometry().width(), screenRect.height() - frameGeometry().height());
}