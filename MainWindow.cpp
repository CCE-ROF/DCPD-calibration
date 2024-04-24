#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "Reports.h"
#include "FlukeHelper.h"
#include "InfoDlg.h"
#include <QStyleFactory>

MainWindow *MainWindow::m_instance;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_instance = this;

    auto m_Db = QSqlDatabase::addDatabase("QSQLITE");
    m_Db.setDatabaseName(qApp->applicationDirPath()+"/Multimeter.db");
    if (!m_Db.open())
        qWarning() << m_Db.lastError().text();

    vftPalette = new VFTPalette(this);
    vftPalette->setPalette("Grey");

    pageReport = new PageReport(ui->stackedMain);
    pageReport->setObjectName("pageReport");
    pageCheck = new PageCheck(ui->stackedMain);
    pageCheck->setObjectName("pageCheck");
    pageSetupApp = new PageSetupApp(ui->stackedMain);
    pageSetupApp->setObjectName("pageSetupApp");
    pageSetupSV = new PageSetupSV(ui->stackedMain);
    pageSetupSV->setObjectName("pageSetupSV");
    pageSetupDV = new PageSetupDV(ui->stackedMain);
    pageSetupDV->setObjectName("pageSetupDV");
    pageRun = new PageRun(ui->stackedMain);
    pageRun->setObjectName("pageRun");
    pageAbout = new PageAbout(ui->stackedMain);
    pageAbout->setObjectName("pageAbout");
    pageFactoryCalibSV = new PageFactoryCalibrationSV(ui->stackedMain);
    pageFactoryCalibSV->setObjectName("pageFactoryCalibSV");

    pageRun->setPageCheck(pageCheck);

    ui->stackedMain->addWidget(pageSetupApp);
    ui->stackedMain->addWidget(pageSetupSV);
    ui->stackedMain->addWidget(pageSetupDV);
    ui->stackedMain->addWidget(pageCheck);
    ui->stackedMain->addWidget(pageRun);
    ui->stackedMain->addWidget(pageReport);
    ui->stackedMain->addWidget(pageAbout);
    ui->stackedMain->addWidget(pageFactoryCalibSV);

    connectionFluke = new CommunicationModule(this);
    connectionFluke->setObjectName("conFluke");
    connectionCS = new CommunicationModule(this);
    connectionCS->setObjectName("conCS");
    settingChanged();

    auto reports = new Reports(this);
    vftLicense = new VFTLicense(this);

    QObject::connect(ui->cmbCmdMode, QOverload<int>::of(&QComboBox::currentIndexChanged), [=](int index)
    {
        if (index == 0)
            ui->btnRemoteMode->setText("Fluke Remote mode / DC measurement");
        if (index == 1)
            ui->btnRemoteMode->setText("Power suuplye ON");
    });

    QObject::connect(ui->btnRemoteMode, &QPushButton::clicked, [=]()
    {
        if (ui->cmbCmdMode->currentIndex() == 0)
        {
            // Enter into Remote mode
            connectionFluke->writeData("SYSTEM:REMOTE\n");

            QTime dieTime = QTime::currentTime().addMSecs( 500 );
            while( QTime::currentTime() < dieTime )
            {
                QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
            }

            // Go into VOLT DC
            connectionFluke->writeData("CONF:VOLT:DC 1\n");

            dieTime = QTime::currentTime().addMSecs( 500 );
            while( QTime::currentTime() < dieTime )
            {
                QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
            }

            // Do VOLT DC measurement
            ui->edtCmd->setText("MEAS:volt:dc? 1");
            connectionFluke->writeData("MEAS:volt:dc? 1\n");
        }
        else
        {
            ui->edtCmd->setText("SV:1 OUTPUT ON");
            connectionCS->writeData("SV:1 OUTPUT ON");
        }
    });

    QObject::connect(pageSetupApp, SIGNAL(changed()), this, SLOT(settingChanged()));
    QObject::connect(ui->btnSend, SIGNAL(clicked()), this, SLOT(sendCmd()));
    QObject::connect(connectionFluke, SIGNAL(gotData(QByteArray)), this, SLOT(getData(QByteArray)));
    QObject::connect(connectionCS, SIGNAL(gotData(QByteArray)), this, SLOT(getData(QByteArray)));
    QObject::connect(ui->btnFactoryCalib, &QPushButton::clicked, [=]()
    {
        QScopedPointer<InfoDlg> dlg(new InfoDlg(QMessageBox::Information, this));
        dlg->userLogin(FlukeHelper::getParameter("password"));
        if (dlg->result() == QDialog::Accepted)
        {
            qDebug() << "------";
            ui->stackedMain->setCurrentWidget(pageFactoryCalibSV);
        }
    });
    QObject::connect(ui->btnSetupApp, &QPushButton::clicked, [=]()
    {
        setWidgetVisibility(Screen::SetupApp);
    });
    QObject::connect(ui->btnSetup, &QPushButton::clicked, [=]()
    {
        if (applicationMode == ApplicationMode::PWCalibration)
            ui->stackedMain->setCurrentWidget(pageSetupSV);
        else if (applicationMode == ApplicationMode::AMPCalibration)
            ui->stackedMain->setCurrentWidget(pageSetupDV);
    });
    QObject::connect(ui->btnCheck, &QPushButton::clicked, [=]()
    {
        ui->stackedMain->setCurrentWidget(pageCheck);
        pageCheck->loadData();
    });
    QObject::connect(pageCheck->btnRun, &QPushButton::clicked, [=]()
    {
        ui->stackedMain->setCurrentWidget(pageRun);
        pageRun->run();
    });
    QObject::connect(ui->btnReport, &QPushButton::clicked, [=]()
    {
        ui->stackedMain->setCurrentWidget(pageReport);
    });
    QObject::connect(ui->btnMain, &QPushButton::clicked, [=]()
    {
        setWidgetVisibility(Screen::Main);
    });
    QObject::connect(ui->btnExit, &QPushButton::clicked, [=]()
    {
        pageRun->stop();
        this->close();
    });
    QObject::connect(ui->btnSVCalibration, &QPushButton::clicked, [=]()
    {
        setWidgetVisibility(Screen::PWCalibrationMain);
    });
    QObject::connect(ui->btnAmpCalibration, &QPushButton::clicked, [=]()
    {
        setWidgetVisibility(Screen::AmpCalibrationMain);
    });
    QObject::connect(ui->btnAbout, &QPushButton::clicked, [=]()
    {
        ui->stackedMain->setCurrentWidget(pageAbout);
    });
    QObject::connect(ui->btnDebug, &QPushButton::clicked, [=]()
    {
        ui->stackedMain->setCurrentWidget(ui->pageSendTest);
    });

    setWidgetVisibility(Screen::Main);
}

void MainWindow::setWidgetVisibility(Screen screen)
{
    ui->btnMain->setVisible(false);
    ui->btnCheck->setVisible(false);
    ui->btnSetupApp->setVisible(false);
    ui->btnFactoryCalib->setVisible(false);
    ui->btnReport->setVisible(false);

    switch (screen)
    {
    case SetupApp: {
        ui->stackedMain->setCurrentWidget(pageSetupApp);
        ui->btnMain->setVisible(true);
        break;
    }
    case PWCalibrationMain: {
        applicationMode = ApplicationMode::PWCalibration;
        ui->lblHeader->setText("VFT SV Calibration");
        ui->stackedMain->setCurrentWidget(pageSetupSV);
        ui->btnSetupApp->setVisible(false);
        ui->btnSetup->setVisible(true);
        ui->btnMain->setVisible(true);

        ui->btnCheck->setVisible(true);
        ui->btnFactoryCalib->setVisible(true);
        ui->btnReport->setVisible(true);
        break;
    }
    case AmpCalibrationMain: {
        applicationMode = ApplicationMode::AMPCalibration;
        ui->lblHeader->setText("VFT DV Calibration");
        ui->stackedMain->setCurrentWidget(pageSetupDV);
        ui->btnSetupApp->setVisible(false);
        ui->btnSetup->setVisible(true);
        ui->btnMain->setVisible(true);

        ui->btnCheck->setVisible(true);
        break;
    }
    default: {
        applicationMode = ApplicationMode::Undef;
        ui->lblHeader->setText("VFT Calibration");
        ui->stackedMain->setCurrentWidget(ui->pageMain);
        ui->btnSetup->setVisible(false);
        ui->btnSetupApp->setVisible(true);
        ui->btnExit->setVisible(true);
        ui->btnMain->setVisible(false);
    }
    }
}

MainWindow::ApplicationMode MainWindow::appMode()
{
    return applicationMode;
}

void MainWindow::settingChanged()
{
    connectionFluke->setConnectionMode(CommunicationModule::ConnectionMode::Serial);
    connectionFluke->setSerialParam(FlukeHelper::getParameter("port1"),
                                FlukeHelper::getParameter("baudRate1").toInt(),
                                FlukeHelper::getParameter("dataBits1").toInt(),
                                FlukeHelper::getParameter("parity1"),
                                FlukeHelper::getParameter("stopBits1"),
                                FlukeHelper::getParameter("flowControl1"));
    connectionFluke->connect();

    connectionCS->setConnectionMode(CommunicationModule::ConnectionMode::Serial);
    connectionCS->setSerialParam(FlukeHelper::getParameter("port2"),
                                FlukeHelper::getParameter("baudRate2").toInt(),
                                FlukeHelper::getParameter("dataBits2").toInt(),
                                FlukeHelper::getParameter("parity2"),
                                FlukeHelper::getParameter("stopBits2"),
                                FlukeHelper::getParameter("flowControl2"));
    connectionCS->connect();
}

void MainWindow::setCurrectPage(QString pageName)
{
    auto page = ui->stackedMain->findChild<QWidget*>(pageName);
    if (page)
        ui->stackedMain->setCurrentWidget(page);
}

void MainWindow::sendCmd()
{
    if (ui->cmbCmdMode->currentIndex() == 0)
        connectionFluke->writeData(ui->edtCmd->text().toUtf8() + '\n', true);
    else
        connectionCS->writeData(ui->edtCmd->text().toUtf8(), true);
}

void MainWindow::getData(QByteArray data)
{
    ui->plainTextEdit->appendPlainText(data);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    Q_UNUSED(event);

    if (pageRun)
        pageRun->stop();
}

MainWindow::~MainWindow()
{
    delete ui;
}
