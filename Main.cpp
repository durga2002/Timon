#include <QtWidgets/QApplication>
#include <QObject>
#include <QFile>
#include <QMessageBox>

#include "MaxiView.h"
#include "MiniView.h"
#include "Logic.h"

int main(int argc, char *argv[])
{   
    QApplication app(argc, argv);

    const QString sLockFileName = "prevent_multiple_instances.lock";
    QFile fLockFile(sLockFileName);
    if (fLockFile.exists())
    {
        QWidget fictive;
        QMessageBox::critical(&fictive, "ERROR", sLockFileName + " is detected.\nDon't run another instance or delete the lock file.");
        return 1;
    }
    fLockFile.open(QIODeviceBase::WriteOnly); // leave it open to protect from deletion by user  

    Logic logic(&app);
    MaxiView maxi;
    MiniView mini;

    app.connect(&mini, &MiniView::commandSent, &logic, &Logic::userCommand);
    app.connect(&logic, &Logic::exitRequested, [&app, &fLockFile]() { 
        fLockFile.close();
        fLockFile.remove(); 
        app.quit(); });

    app.connect(&logic, &Logic::miniViewRequested, &mini, &MiniView::show);
    app.connect(&logic, &Logic::miniViewRequested, &maxi, &MaxiView::hide);

    app.connect(&logic, &Logic::maxiViewRequested, &maxi, &MaxiView::show);
    app.connect(&logic, &Logic::maxiViewRequested, &mini, &MiniView::hide);

    app.connect(&mini, &MiniView::userCame, &logic, &Logic::userCame);
    app.connect(&maxi, &MaxiView::userLeft, &logic, &Logic::userLeft);

    app.connect(&maxi, &MaxiView::activityChanged, &logic, &Logic::updateActivity);
    app.connect(&logic, &Logic::statePublished, &mini, &MiniView::updateState);
    app.connect(&logic, &Logic::statePublished, &maxi, &MaxiView::updateState);

    mini.updateScreen(app.primaryScreen()); 
    maxi.updateScreen(app.primaryScreen());

    logic.updateActivity(IDLE);
    logic.miniViewRequested();

    return app.exec();
}

