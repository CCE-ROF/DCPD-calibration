#pragma once

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlRecord>

#include "VFTPalette.h"
#include "PageReport.h"
#include "PageCheck.h"
#include "PageSetupApp.h"
#include "PageSetupSV.h"
#include "PageSetupDV.h"
#include "PageRun.h"
#include "PageAbout.h"
#include "PageFactoryCalibrationSV.h"
#include "VFTLicense.h"

QT_BEGIN_NAMESPACE
    namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    static MainWindow *instance()
    {
        return m_instance;
    }
    enum Screen {
        Main,
        SetupApp,
        PWCalibrationMain,
        AmpCalibrationMain
    };
    enum ApplicationMode {
        Undef,
        PWCalibration,
        AMPCalibration
    };

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    ApplicationMode appMode();
    void setCurrectPage(QString pageName);

    CommunicationModule *connectionFluke;
    CommunicationModule *connectionCS;

private:
    void closeEvent(QCloseEvent *event);
    Ui::MainWindow *ui;
    static MainWindow * m_instance;
    VFTLicense *vftLicense;
    VFTPalette *vftPalette;
    ApplicationMode applicationMode;
    PageCheck *pageCheck;
    PageSetupApp *pageSetupApp;
    PageSetupSV *pageSetupSV;
    PageSetupDV *pageSetupDV;
    PageRun *pageRun;
    PageReport *pageReport;
    PageAbout *pageAbout;
    PageFactoryCalibrationSV *pageFactoryCalibSV;

private slots:
    void settingChanged();
    void sendCmd();
    void getData(QByteArray data);
    void setWidgetVisibility(Screen screen);

};
