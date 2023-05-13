#include <QFile>
#include <QDate>
#include <QMutexLocker>
#include <QGuiApplication>
#include <QScreen>
#include <QMessageBox>

#include "Logic.h"

Logic::Logic(QObject *parent) : QObject(parent),
	m_timeCounters(NUM_ACTIVITIES, 0),
	m_currentActivity(IDLE),
    m_autoIdleEnabled(true)
{
	m_lastTimeUpdated = std::chrono::system_clock::now();
	loadFromLog();
    startTimer(m_mouseActivityCheckIntervalMs);
}

Logic::~Logic()
{
    updateLog();
}

bool Logic::dayChanged(const std::tm & prev, const std::tm & now)
{
    return prev.tm_year != now.tm_year
        || prev.tm_mon != now.tm_mon
        || prev.tm_mday != now.tm_mday;
}

std::tm Logic::midnight(const std::chrono::system_clock::time_point& tp)
{
    const std::time_t t = std::chrono::system_clock::to_time_t(tp);
    std::tm* pLocalTime = localtime(&t);
    pLocalTime->tm_hour = 0;
    pLocalTime->tm_min = 0;
    pLocalTime->tm_sec = 0;
    return std::tm(*pLocalTime);
}

void Logic::updateActivity(int activity)
{
    auto now = std::chrono::system_clock::now();

    // Zero all time counters if day changed
    std::tm lastMidnight = midnight(now);
    const std::tm lastUpdateMidnight = midnight(m_lastTimeUpdated);
    if (dayChanged(lastUpdateMidnight, lastMidnight))
    {
        // The known feature/glitch/bug: supposed IDLE state from m_lastTimeUpdated to the midnight, 
        // so this time interval can be ignored.
        m_lastTimeUpdated = std::chrono::system_clock::from_time_t(mktime(&lastMidnight));
        for (auto& timer : m_timeCounters)
        {
            timer = 0;
        }
    }

    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_lastTimeUpdated);
    m_timeCounters[m_currentActivity] += duration.count();
    m_lastTimeUpdated = now;
    m_currentActivity = activity;
    updateLog();
    emit statePublished(m_timeCounters, m_currentActivity, m_autoIdleEnabled);
}

void Logic::userCame()
{
    updateActivity(m_currentActivity);
    emit maxiViewRequested();
}

void Logic::userLeft()
{
    updateActivity(m_currentActivity);
    emit miniViewRequested();
}
void Logic::showMessage(QString msg)
{
    static QWidget fictiveParent;

    QMessageBox *m_messageBox = new QMessageBox(&fictiveParent);
    m_messageBox->setWindowFlags(m_messageBox->windowFlags() | Qt::WindowStaysOnTopHint);
    m_messageBox->setIcon(QMessageBox::Information);
    m_messageBox->setWindowTitle("Give me a name!");
    m_messageBox->setText(msg);
    m_messageBox->setModal(false);
    m_messageBox->setAttribute(Qt::WA_DeleteOnClose);
    m_messageBox->show();
 }

void Logic::userCommand(UserCommand cmd)
{
    switch (cmd)
    {
    case ENABLE_AUTO_IDLE:
        m_autoIdleEnabled = true;
        break;
    case DISABLE_AUTO_IDLE:
        m_autoIdleEnabled = false;
        break;
    case EXIT:
        emit exitRequested();
        break;
    default:
        ;
    }
    updateActivity(m_currentActivity);
}

// Auto idle check
void Logic::timerEvent(QTimerEvent* e)
{
    if (!m_autoIdleEnabled || m_currentActivity == IDLE)
    {
        return;
    }

    static QPoint lastPosition;
    static auto lastTimeMouseMoved = std::chrono::system_clock::now();

    QPoint newPosition = QCursor::pos();

    auto now = std::chrono::system_clock::now();
    bool outOfScreen = QGuiApplication::screenAt(newPosition) == nullptr;
    if (lastPosition == newPosition || outOfScreen)
    {
         if (std::chrono::duration_cast<std::chrono::minutes>(now - lastTimeMouseMoved) > std::chrono::minutes(m_autoIdleTimeoutMinutes))
         {
            m_lastTimeUpdated = lastTimeMouseMoved;
            updateActivity(IDLE);
            showMessage("Auto IDLE occured");            
        }
    }
    else
    {
        lastTimeMouseMoved = now;
        lastPosition = newPosition;
    }
}

void Logic::updateLog()
{
    QMutexLocker<QMutex> locker(&m_mutex); // who knows, maybe it's mutlithreaded app...

    QFile fCurrentfLog(m_sLogFileBaseName);
    if (!fCurrentfLog.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showMessage("Can't open current log. New log file will be created.");

    }

    QTextStream input(&fCurrentfLog);
    QString sLine = input.readLine();
    if (!sLine.isEmpty())
    {
        QStringList records = sLine.split(u' ');
        QDate date = QDate::fromString(records.takeFirst(), Qt::ISODate);
        // If day changed get ready to append the old content to the new log
        if (date != QDate::currentDate())
        {
            input.seek(0);
        }
    }

    QString sNewLogFile = m_sLogFileBaseName + ".tmpnew";
    QFile fNewLog(sNewLogFile);
    if (!fNewLog.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        showMessage("Can't update log file.");
        return;
    }
    QTextStream output(&fNewLog);
    output << QDate::currentDate().toString(Qt::ISODate);
    for (auto count : m_timeCounters)
    {
        output << " " << count;
    }
    // append old records to the log
    output << "\n" << input.readAll() << Qt::endl;

    fNewLog.close();
    fCurrentfLog.close();

    QString sOldLogFile = m_sLogFileBaseName + ".bak" ;
    QFile::remove(sOldLogFile);
    fCurrentfLog.rename(sOldLogFile);
    fNewLog.rename(m_sLogFileBaseName);
}

void Logic::loadFromLog()
{
    QFile fLog(m_sLogFileBaseName);
    if (!fLog.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        showMessage("Can't open log file.");
        return;
    }

    QTextStream input(&fLog);
    QString sLine = input.readLine();
    QStringList records = sLine.split(u' ');

    QDate date = QDate::fromString(records.takeFirst(), Qt::ISODate);
    if (date == QDate::currentDate())
    {
        QList<long> times;
        while (!records.empty())
        {
            times.push_back(records.takeFirst().toLong());
        }
        if (times.size() != NUM_ACTIVITIES)
        {
            showMessage("Incosistent log file: unexpected number of activities.");
            return;
        }
        for (size_t i = 0; i < m_timeCounters.size(); i++)
        {
            m_timeCounters[i] += times[i];
        }
    }

    fLog.close();
}
