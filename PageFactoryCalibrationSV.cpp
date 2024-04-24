#include "PageFactoryCalibrationSV.h"
#include "ui_PageFactoryCalibrationSV.h"
#include "PageSetupSV.h"
#include "FlukeHelper.h"
#include "MainWindow.h"
#include "InfoDlg.h"
#include <QMessageBox>

PageFactoryCalibrationSV::PageFactoryCalibrationSV(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageFactoryCalibrationSV)
{
    ui->setupUi(this);

    QPalette palette = ui->btnStop->palette();
    palette.setColor(QPalette::Button, QColor(0,155,0));
    ui->btnStop->setPalette(palette);
    ui->btnStop->setText("RUN");
    m_isRunning = false;

    ui->lblInfo->setText("Status: Stopped. ");
    ui->lblVoltage->setText(tr("The current Voltage: %1, ").arg(0));
    ui->lblAmpere->setText(tr("Ampere: %1").arg(0));

    QObject::connect(ui->btnStop, &QPushButton::clicked, this, &PageFactoryCalibrationSV::btnAction);
}

void PageFactoryCalibrationSV::btnAction()
{
    if (m_isRunning)
        stop();
    else
        run();
}

void PageFactoryCalibrationSV::run()
{
    //qDebug() <<  QString("SV:1 SET").toUtf8().toHex(' ')  ;



    // Load lab's results
    QList<QPair<double, double>> labValueList = PageSetupSV::instance()->loadSVCalibLabData(PageSetupSV::instance()->currentSettings.calibLab_id);
    if (labValueList.size() == 0)
    {
        QMessageBox::warning(this, tr("Error"), tr("There is no Linearization data of resistor"));
        return;
    }

    m_isRunning = true;
    ui->lblInfo->setText("Status: Running. ");
    ui->tableWidget->setRowCount(0);

    QPalette palette = ui->btnStop->palette();
    palette.setColor(QPalette::Button, QColor(255,0,0));
    ui->btnStop->setPalette(palette);
    ui->btnStop->setText("STOP");

    auto comFluke = MainWindow::instance()->connectionFluke;
    qDebug() << comFluke->status() <<  comFluke->statusString();


    comFluke->writeCmd(QString("SYSTEM:REMOTE").toUtf8() + '\n', false);  // 1 - connect to Fluke
    doPause(500);
    comFluke->writeCmd(QString("CONF:VOLT:DC %1").arg(1).toUtf8() + '\n', false);   // Switch Flucke to Volt DC
    doPause(500);

    auto comSV = MainWindow::instance()->connectionCS;

    // start calibration
    comSV->writeData(QString("SV:1_@=!KALIB").toUtf8());  // 2 - start calibration
    doPause(500);

    // enable output
    comSV->writeData(QString("SV:1 OUTPUT ON").toUtf8()); // 3 - output on
    doPause(500);

    double outputA = 0.1;  // do initial reading from Fluke
    while (outputA < 20.1)
    {
        if (!m_isRunning)
        {
            qApp->processEvents();
            comFluke->writeCmd(QString("SYSTEM:LOCAL").toUtf8() + '\n', false);  // disconnect
            return;
        }

        // So we measure mV from the Fluke and then the calculated value A = V/R
        // ( V is coming from the Fluke and R = the value from the calibration file we loaded.)

        ui->tableWidget->setRowCount(ui->tableWidget->rowCount()+1);

        QTableWidgetItem *item = new QTableWidgetItem("");
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 0, item);

        item = new QTableWidgetItem("");
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1, 1, item);

        // read value from Fluke
        double diffA = 0;
        double koefA = 0.001;  // difference between values
        do
        {
            if (!m_isRunning)
            {
                qApp->processEvents();
                comFluke->writeCmd(QString("SYSTEM:LOCAL").toUtf8() + '\n', false);  // disconnect
                return;
            }

            QString strVoltage = comFluke->writeCmd(QString("MEAS:volt:dc? 1").toUtf8() + '\n', true);
            double mesV = strVoltage.toDouble();
            double linerAmpere = FlukeHelper::ampereFromResolution(&labValueList, mesV, 0);

            diffA = outputA - linerAmpere;

            qDebug() << "Got from Flike" << strVoltage
                     << "linerAmpere" << linerAmpere
                     << "    DIFF" << diffA;

            ui->lblVoltage->setText(tr("The current Voltage: %1, ").arg(QString::number(mesV, 'f', 6)));
            ui->lblAmpere->setText(tr("Ampere: %1").arg(linerAmpere));

            // step of the UP/DOWN is 0.0003
            bool isOK = false;

            if ( qAbs(diffA) <= koefA * 3)   // 0.001 * 1.5 --IT IS OK and we can jump to the next iteration
            {
                qDebug() << "---------------";
                isOK = true;
            }
            else if (diffA < koefA)
            {
                comSV->writeData(QString("SV:1 DOWN").toUtf8());
                doPause(50);
                qDebug() << "Down";
            }
                                              //     0.099912
            else if (diffA > koefA)    // DIFF 0.0999087  koefA = 0.001;  // difference between values
            {
                comSV->writeData(QString("SV:1 UP").toUtf8());
                doPause(50);
                qDebug() << "Up";
            }

            ui->tableWidget->item(ui->tableWidget->rowCount()-1, 0)->setText(QString("%1").arg(outputA));
            ui->tableWidget->item(ui->tableWidget->rowCount()-1, 1)->setText(QString("V / I = %1 / %2").arg(mesV).arg(linerAmpere));
            ui->tableWidget->scrollToBottom();

            if (isOK)
            {
                break;
            }

            doPause(500);
            qApp->processEvents();
        }
        while (qAbs(diffA) > koefA);

        doPause(100);
        qApp->processEvents();

        // Go to next step
        qDebug() << "SET";
        comSV->writeData(QString("SV:1 OVP").toUtf8());
        //comSV->writeData(QString("SV:1 SET").toUtf8());   //53 56 3a 31 00 53 45 54
        doPause(500);
        outputA += 0.1;

        doPause(100);
        qApp->processEvents();
    }

    comFluke->writeCmd(QString("SYSTEM:LOCAL").toUtf8() + '\n', false);  // disconnect

    QScopedPointer<InfoDlg> dlg(new InfoDlg(QMessageBox::Information, this));
    dlg->showThis(tr("Calibration is done"));

    palette.setColor(QPalette::Button, QColor(0,155,0));
    ui->btnStop->setPalette(palette);
    ui->btnStop->setText("RUN");
}

void PageFactoryCalibrationSV::stop()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        qApp->processEvents();
        ui->lblInfo->setText("Status: Stopped. ");
        QMessageBox::warning(this, tr("Info"), tr("Interruption of getting data"));

        QPalette palette = ui->btnStop->palette();
        palette.setColor(QPalette::Button, QColor(0,155,0));
        ui->btnStop->setPalette(palette);
        ui->btnStop->setText("RUN");
    }
}

void PageFactoryCalibrationSV::doPause(quint32 pauseMsec)
{
    if (!m_isRunning)
        return;

    QTime dieTime = QTime::currentTime().addMSecs(pauseMsec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
}

PageFactoryCalibrationSV::~PageFactoryCalibrationSV()
{
    delete ui;
}
