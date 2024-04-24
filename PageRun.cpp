#include "PageRun.h"
#include "ui_PageRun.h"
#include "CommunicationModule.h"
#include "CalCheckInfo.h"
#include "FlukeHelper.h"
#include "Reports.h"
#include "MainWindow.h"
#include <QPushButton>

PageRun::PageRun(QWidget *parent) :
    QWidget(parent),  ui(new Ui::PageRun)
{
    ui->setupUi(this);

    QPalette palette = ui->btnStop->palette();
    palette.setColor(QPalette::Button, QColor(255,0,0));
    ui->btnStop->setPalette(palette);

    m_isRunning = false;

	QObject::connect(ui->btnStop, &QPushButton::clicked, this, &PageRun::stop);
    QObject::connect(ui->btnExport, &QPushButton::clicked, this, &PageRun::exportResult);
    QObject::connect(ui->btnPreview, &QPushButton::clicked, this, &PageRun::printPreview);
}

void PageRun::setPageCheck(PageCheck *page)
{
    pageCheck = page;
}

void PageRun::stop()
{
    if (m_isRunning)
    {
        m_isRunning = false;
        qApp->processEvents();
        QMessageBox::warning(this, tr("Info"), tr("Interruption of getting data"));

        auto mw = MainWindow::instance();
        mw->setCurrectPage("pageCheck");
    }
}

void PageRun::run()
{
    ui->btnStop->setText("STOP");

    ui->btnExport->setEnabled(false);
    ui->btnPreview->setEnabled(false);

    m_isRunning = true;
    ui->tableWidget->clear();
    ui->tableWidget->setColumnCount(pageCheck->parameters().runs + 1);

    if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::PWCalibration)
    {
        runPWCalibration();
    }
    else if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::AMPCalibration)
    {
        runAMPCalibration();
    }

    m_isRunning = false;
    ui->btnExport->setEnabled(true);
    ui->btnPreview->setEnabled(true);

    ui->btnStop->setText("RUN");
}

void PageRun::runPWCalibration()
{
    ui->lbl1->setText(tr("Measurement No: %1").arg(0));
    ui->lbl2->setText(tr("Before start"));
    ui->lblVoltage->setVisible(true);
    ui->lblAmpere->setVisible(true);
    ui->lblVoltage->setText(tr("The current Voltage: %1, ").arg(0));
    ui->lblAmpere->setText(tr("Ampere: %1").arg(0));
    ui->tableWidget->setRowCount(pageCheck->parameters().range);

    // Load lab's results
    labValueList = PageSetupSV::instance()->loadSVCalibLabData(PageSetupSV::instance()->currentSettings.calibLab_id);

    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(ui->tableWidget->width() / ui->tableWidget->columnCount());
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem);
    ui->tableWidget->model()->setHeaderData(0, Qt::Horizontal, "Value, A");

    for (int col = 1; col < ui->tableWidget->columnCount(); col++)
    {
        ui->tableWidget->setHorizontalHeaderItem(col, new QTableWidgetItem);
        ui->tableWidget->model()->setHeaderData(col, Qt::Horizontal, QString("Run %1, A").arg(col));
    }

    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        QString val;
        if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::PWCalibration)
            val = QString::number(row + 1, 'f', 3);
        else if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::AMPCalibration)
            val = QString("%1 %").arg(-(100 - row * 10));

        auto item = new QTableWidgetItem(val, row);
        ui->tableWidget->setItem(row, 0, item);
    }

    calib_id = 0;

    QScopedPointer<CalCheckInfo> dlg(new CalCheckInfo(this));
    dlg->showThis();
    if (dlg->result() == QDialog::Accepted)
    {
        calib_id = dlg->calib_id;
    }
    else
    {
        auto mw = MainWindow::instance();
        mw->setCurrectPage("pageCheck");
        return;
    }

    ui->lbl2->setText(tr("In progress"));

    // Do measurement
    /*
    The command will be :
    CS:FS_SET <value>
    CS:mV_SET <value>
    Etc..

    CH_SEL is not used in the CS. This will be on the DV SCPI
    */

    auto comPS = MainWindow::instance()->connectionCS;
    comPS->writeData("SV:1 A 00.000");       // Set A = 0
    doPause(500);
    comPS->writeData("SV:1 OUTPUT ON");      // Switch On power supply

    auto comFluke = MainWindow::instance()->connectionFluke;
    comFluke->writeCmd(QString("SYSTEM:REMOTE").toUtf8() + '\n', false);    // Connect Fluke
    doPause(500);


    int measNo = 0;
    // Before start
    QString strValue = comFluke->writeCmd(QString("CONF:VOLT:DC %1").arg(1).toUtf8() + '\n', false);   // Switch Flucke to Volt DC
    doPause(500);

    for (int col = 1; col < ui->tableWidget->columnCount(); col++)
    {
        for (int row = 0; row < ui->tableWidget->rowCount(); row++)   // Iterate over rows (1A increment)
        {
            qApp->processEvents();

            if (!m_isRunning)
            {
                ui->lbl2->setText(tr("Interrupted"));
                comFluke->writeCmd(QString("SYSTEM:LOCAL").toUtf8() + '\n', false);

                qApp->processEvents();

                QSqlQuery query;
                query.exec(QString("DELETE FROM SVCalCheckHeader WHERE id=%1").arg(calib_id));

                qApp->processEvents();
                return;
            }

            ui->lbl2->setText(tr("In progress"));

            QString str1 = ui->tableWidget->item(row, 0)->text();
            if (str1.toDouble() < 10)
                str1 = "0" + str1;
            qDebug() << QString("SV:1 A %1").arg(str1).toLocal8Bit();

            comPS->writeData("SV:1 OUTPUT OFF");      // Switch Off power supply
            doPause(500);
            comPS->writeData(QString("SV:1 A %1").arg(str1).toUtf8());   // Set Power Supply to row number (increment 1A)
            doPause(500);
            comPS->writeData("SV:1 OUTPUT ON");      // Switch On power supply
            doPause(2000);

            strValue = comFluke->writeCmd(QString("MEAS:volt:dc? 1").toUtf8() + '\n', true);   // Do voltage measuremet on Fluke

            qDebug() << "   PS" << str1;
            qDebug() << "   Fluke" << strValue;

            // Do calculation, we checked it at the last time
            cuurentRow = row;
            double voltage = qAbs(strValue.toDouble());
            double ampere1 = voltage / PageSetupSV::instance()->currentSettings.calibLabResistor * 1000;
            double ampere2 = FlukeHelper::ampereFromResolution(&labValueList, voltage, row + 1);// * 1000;

            strValue = QString::number(ampere2);

            auto item = ui->tableWidget->item(row, col);
            if (item)
            {
                item->setText(strValue);
            }
            else
            {
                item = new QTableWidgetItem(strValue);
                ui->tableWidget->setItem(row, col, item);
            }
            ui->tableWidget->scrollToItem(item);

            ui->lbl1->setText(tr("Measurement No: %1").arg(measNo+1));

            insertIntoDB(row, col, voltage, ampere1, ampere2);
            doInfoPause(pageCheck->parameters().delay * 1000);
            measNo++;

            qApp->processEvents();
        }
    }

    comFluke->writeCmd(QString("SYSTEM:LOCAL").toUtf8() + '\n', false);
    comPS->writeData("SV:1 OUTPUT OFF");
}

void PageRun::runAMPCalibration()
{
    // ---For AmpCalibration---
    double maxGain = 0;
    double curGain = 0;
    double FS_SET = 0;
    double mV_SET = 0;

    if (pageCheck->parameters().range == 0)
    {
        maxGain = 40000;
        curGain = 500;
    }
    else
    {
        maxGain = pageCheck->parameters().range;
        curGain = pageCheck->parameters().range;
    }
    // ---For AmpCalibration---

    ui->lbl1->setText(tr("Gain: %1 %").arg(-100));
    ui->lbl2->setText(tr("Before start"));
    ui->lblAmpere->setVisible(false);
    ui->lblVoltage->setText(tr("Gain set: %1, ").arg(curGain));
    ui->tableWidget->setRowCount(21);

    // Load lab's results
    loadDVCalibLabData();

    ui->tableWidget->horizontalHeader()->setDefaultSectionSize(ui->tableWidget->width() / ui->tableWidget->columnCount());
    ui->tableWidget->setHorizontalHeaderItem(0, new QTableWidgetItem);
    ui->tableWidget->model()->setHeaderData(0, Qt::Horizontal, "Value, A");

    for (int col = 1; col < ui->tableWidget->columnCount(); col++)
    {
        ui->tableWidget->setHorizontalHeaderItem(col, new QTableWidgetItem);
        ui->tableWidget->model()->setHeaderData(col, Qt::Horizontal, QString("Run %1, Gain").arg(col));
    }

    for (int row = 0; row < ui->tableWidget->rowCount(); row++)
    {
        QString val;
        if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::PWCalibration)
            val = QString::number(row + 1, 'f', 3);
        else if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::AMPCalibration)
            val = QString("%1 %").arg(-(100 - row * 10));

        auto item = new QTableWidgetItem(val, row);
        ui->tableWidget->setItem(row, 0, item);
    }

    calib_id = 0;

    QScopedPointer<CalCheckInfo> dlg(new CalCheckInfo(this));
    dlg->showThis();
    if (dlg->result() == QDialog::Accepted)
    {
        calib_id = dlg->calib_id;
    }
    else
    {
        auto mw = MainWindow::instance();
        mw->setCurrectPage("pageCheck");
        return;
    }

    ui->lbl2->setText(tr("In progress"));

    // Do measurement
    /*
    The command will be :
    CS:FS_SET <value>
    CS:mV_SET <value>
    Etc..

    CH_SEL is not used in the CS. This will be on the DV SCPI
    */

    auto comFluke = MainWindow::instance()->connectionFluke;
    auto comCS    = MainWindow::instance()->connectionCS;
    comFluke->writeCmd(QString("SYSTEM:REMOTE").toUtf8() + '\n', false);
    doPause(500);
    QString strValue;

    while (curGain <= maxGain)
    {
        // Before start
        strValue = comFluke->writeCmd(QString("CONF:VOLT:DC %1").arg(1).toUtf8() + '\n', false);  //???

        if (curGain == 500)
            FS_SET = 20;
        else if (curGain == 1000)
            FS_SET = 10;
        else if (curGain == 2000)
            FS_SET = 5;
        else if (curGain == 5000)
            FS_SET = 2;
        else if (curGain == 10000)
            FS_SET = 1;
        else if (curGain == 20000)
            FS_SET = 0.5;
        else if (curGain == 40000)
            FS_SET = 0.25;



        // G_SET
        comCS->writeCmd(QString("DV:01:G %1").arg(curGain).toUtf8() + '\n', true);        //???  set the gain?
        doPause(500);
        // FS_SET
        comCS->writeCmd(QString("CS:FS_SET %1").arg(FS_SET).toUtf8() + '\n', true);       //???  what value we send here?
        doPause(500);
        // mV_SET
        comCS->writeCmd(QString("CS:mV_SET %1").arg(-100).toUtf8() + '\n', true);         //???  percent from gain?
        doPause(500);

//        comCS->writeCmd(QString("DV:01:Z").toUtf8() + '\n', true);       // Auto Zero    //??? FS_SEL=ZERO
//        comCS->writeCmd(QString("DV:01:CH:ON").toUtf8() + '\n', true);                   //??? CH_SEL=1

//        strValue = comFluke->writeCmd(QString("MEAS:volt:dc:rat? 1").toUtf8() + '\n', true);   //??? Read Fluke voltage
//        //OR
//        strValue = comFluke->writeCmd(QString("MEAS:volt:dc? 1").toUtf8() + '\n', true);

        for (int col = 1; col < ui->tableWidget->columnCount(); col++)
        {
            for (int row = 0; row < ui->tableWidget->rowCount(); row++)
            {
                qApp->processEvents();

                if (!m_isRunning)
                {
                    ui->lbl2->setText(tr("Interrupted"));
                    comFluke->writeCmd(QString("SYSTEM:LOCAL").toUtf8() + '\n', false);

                    qApp->processEvents();

                    QSqlQuery query;
                    query.exec(QString("DELETE FROM DVCalCheckHeader WHERE id=%1").arg(calib_id));

                    qApp->processEvents();
                    return;
                }

                auto item = ui->tableWidget->item(row, 0);
                double mvPercent = item->text().replace(" %", "").toDouble();
                double mvK = mvPercent/100;

                mV_SET = FS_SET * mvK;
                strValue = QString("GAIN: %1    FS_SET: %2    mV_SET: %3     %4%       mvK: %5")
                                    .arg(curGain)
                                    .arg(FS_SET)
                                    .arg(mV_SET)
                                    .arg(mvPercent)
                                    .arg(mvK);

                //comCS->writeCmd(QString("CS:mV_SET %1").arg(mvK).toUtf8() + '\n', true);              //???  percent from gain?
                //todo
                //strValue = comFluke->writeCmd(QString("MEAS:volt:dc? 1").toUtf8() + '\n', true);              //??? OR
                //strValue = comFluke->writeCmd(QString("MEAS:volt:dc:rat? 1").toUtf8() + '\n', true);          //???
                //comCS->writeCmd(QString("SV:GAIN %1").arg(gain).toUtf8() + '\n', true);   //??? I did not find this command in the manual

                item = ui->tableWidget->item(row, col);
                if (item)
                {
                    item->setText(strValue);
                }
                else
                {
                    item = new QTableWidgetItem(strValue);
                    ui->tableWidget->setItem(row, col, item);
                }
                ui->tableWidget->scrollToItem(item);

                ui->lbl1->setText(tr("Gain: %1 %").arg(mvPercent));

                doInfoPause(pageCheck->parameters().delay * 1000);


                qApp->processEvents();
            }
        }

        // Check, when loop should be break
        // Increment Gain
        if (curGain == 500)
            curGain = 1000;
        else if (curGain == 1000)
            curGain = 2000;
        else if (curGain == 2000)
            curGain = 5000;
        else if (curGain == 5000)
            curGain = 10000;
        else if (curGain == 10000)
            curGain = 20000;
        else if (curGain == 20000)
            curGain = 40000;
        else
            break;
    }

    comFluke->writeCmd(QString("SYSTEM:LOCAL").toUtf8() + '\n', false);
}

void PageRun::exportResult()
{
    auto reports = Reports::instance();
    reports->exportData(calib_id);
}

void PageRun::printPreview()
{
    auto reports = Reports::instance();
    reports->printPreview(calib_id);
}

void PageRun::insertIntoDB(int row, int runNo, double voltage, double ampere1, double ampere2)
{
    if (runNo == 1)
    {
        QString sqlStr = "INSERT INTO SVCalCheckData (header_id, RowNo)"
                         "VALUES (:header_id, :RowNo)";
        QSqlQuery query;
        query.prepare(sqlStr);
        query.bindValue(":header_id", calib_id);
        query.bindValue(":RowNo", row + 1);
        query.exec();
    }

    QString colName_a  = QString("a%1").arg(runNo);
    QString colName_ao = QString("ao%1").arg(runNo);
    QString colName_v  = QString("v%1").arg(runNo);

    QString sqlStr = "UPDATE SVCalCheckData SET " +
                     colName_v  + " = :" + colName_v + ", " +
                     colName_a  + " = :" + colName_a + ", " +
                     colName_ao + " = :" + colName_ao +
                     " WHERE header_id = :header_id AND RowNo = :RowNo";
    QSqlQuery query;
    query.prepare(sqlStr);
    query.bindValue(":header_id", calib_id);
    query.bindValue(":RowNo", row + 1);
    query.bindValue(":" + colName_a, ampere1);
    query.bindValue(":" + colName_ao, ampere2);
    query.bindValue(":" + colName_v, voltage);
    query.exec();

    //qDebug() << query.lastError().text();
}

void PageRun::loadDVCalibLabData()
{
    //todo
}

void PageRun::doInfoPause(quint32 pauseMsec)
{
    QElapsedTimer timer;
    timer.start();

    //qDebug() << "===InfoPause===";
    QScopedPointer<QTimer> infoTimer(new QTimer(this));
    QObject::connect(infoTimer.get(), &QTimer::timeout, this, [=]()
    {
        if (!m_isRunning)
            return;

        auto comFluke = MainWindow::instance()->connectionFluke;
        auto comCS    = MainWindow::instance()->connectionCS;

        if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::PWCalibration)
        {
            QString strValue = comFluke->writeCmd(QString("MEAS:volt:dc? 1").toUtf8() + '\n', true);
            double voltage = qAbs(strValue.toDouble());
            double ampere = FlukeHelper::ampereFromResolution(&labValueList, voltage, cuurentRow + 1);
            ampere = ampere * 1000;

            ui->lblVoltage->setText(tr("The current Voltage: %1, ").arg(voltage));
            ui->lblAmpere->setText(tr("Ampere: %1").arg(QString::number(ampere, 'f', 5)));
        }
        else if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::AMPCalibration)
        {

        }

        double remainingTime = pauseMsec - timer.elapsed();
        double tSec = remainingTime / 1000;
        //qDebug() << "    " << pauseMsec << timer.elapsed() << remainingTime << "tSec" << tSec;
        ui->lbl2->setText(tr("For the next measurement %1 seconds").arg(tSec));
    });
    infoTimer.get()->start(500);

    while(!timer.hasExpired(pauseMsec))
    {
        qApp->processEvents();
    }

    infoTimer->stop();
}

void PageRun::doPause(quint32 pauseMsec)
{
    if (!m_isRunning)
        return;

    QTime dieTime = QTime::currentTime().addMSecs(pauseMsec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents( QEventLoop::AllEvents, 100 );
}

PageRun::~PageRun()
{
    delete ui;
}
