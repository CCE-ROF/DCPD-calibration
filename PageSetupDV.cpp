#include "PageSetupDV.h"
#include "ui_PageSetupDV.h"
#include "MainWindow.h"
#include <QDateTime>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>
#include "FlukeHelper.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDlg", "N/A");

PageSetupDV *PageSetupDV::m_instance;

PageSetupDV::PageSetupDV(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageSetupDV)
{
    ui->setupUi(this);
    m_instance = this;

    QObject::connect(ui->btnSelectFile, SIGNAL(clicked()), this, SLOT(selectFile()));
    QObject::connect(ui->btnApply, &QPushButton::clicked, this, &PageSetupDV::updateSettings);
    QObject::connect(ui->cbCalibration, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PageSetupDV::selectCalibration);
    QObject::connect(ui->btnToday, &QToolButton::clicked,  [=]()
    {
        ui->dtpCalibDate->setDate(QDate::currentDate());
    });

    loadData();

    ui->cbCalibration->setCurrentIndex(0);
    updateSettings();
}

void PageSetupDV::updateSettings()
{
    QString sqlStr = "UPDATE DVLabHeader SET "
                     "cs_sn = :cs_sn, "
                     "calibDate = :calibDate "
                     "WHERE id = :id";
    QSqlQuery query;
    query.prepare(sqlStr);
    query.bindValue(":ca_sn", ui->edtShutCalSN->text());
    query.bindValue(":calibDate", ui->dtpCalibDate->date());
    query.bindValue(":id", ui->cbCalibration->currentData().toInt());
    query.exec();

    currentSettings.calibLab_id = ui->cbCalibration->currentData().toInt();
    currentSettings.calibLabSN = ui->edtShutCalSN->text();
    currentSettings.calibLabDate = ui->dtpCalibDate->date();
}

void PageSetupDV::selectFile()
{
    QString folderPath = QApplication::applicationDirPath();
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select File"), folderPath, "CAL (*.cal)");
    if (fileName.isEmpty())
        return;

    ui->lblFileName->setText(fileName);
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "Could not open file";
        return;
    }

    QString sqlStr;
    int lineNo = 0;
    int header_id = 0;

    while (!file.atEnd())
    {
        QByteArray line = file.readLine();
        QString str(line);
        QStringList list = str.split(";");

        QSqlQuery query;

        if (lineNo == 0)
            currentSettings.calibLabSN = list[1].replace("\r\n", "");
        if (lineNo == 1)
            currentSettings.calibLabDate = QDate::fromString(list[1].replace("\r\n", ""), "yyyy-MM-dd");

        if (lineNo == 3)
        {
            sqlStr = "INSERT INTO DVLabHeader (cs_sn, calibDate) "
                     "VALUES (:shunt_sn, :calibDate);";
            query.prepare(sqlStr);
            query.bindValue(":shunt_sn", currentSettings.calibLabSN);
            query.bindValue(":calibDate", currentSettings.calibLabDate);
            query.exec();

            if (query.lastError().type() != QSqlError::NoError)
                qDebug() << query.lastError().text();

            sqlStr = "select max(id) as calib_id from DVLabHeader";
            query.clear();
            query.exec(sqlStr);

            if (query.lastError().type() != QSqlError::NoError)
                qDebug() << query.lastError().text();

            query.first();

            header_id = query.value(0).toInt();
        }

        //todo
//        if (lineNo > 3 && calib_id != 0)
//        {
//            sqlStr = "INSERT INTO DVLabData (header_id, voltage, ampere)"
//                     "VALUES (:header_id, :voltage, :ampere)";
//            query.prepare(sqlStr);
            query.bindValue(":header_id", header_id);
//            query.bindValue(":voltage", list.at(1));
//            query.bindValue(":ampere", list.at(0));
//            query.exec();

//            if (query.lastError().type() != QSqlError::NoError)
//                qDebug() << query.lastError().text();
//        }

        lineNo++;
    }

    loadData();
    ui->cbCalibration->setCurrentIndex(0);
}

void PageSetupDV::selectCalibration(int index)
{
    if (index == -1)
        return;

    int id = ui->cbCalibration->currentData().toInt();

    QSqlQuery query;
    query.exec("SELECT * FROM DVLabHeader WHERE id = " + QString::number(id));
    query.first();

    int idx = query.record().indexOf("cs_sn");
    ui->edtShutCalSN->setText(query.value(idx).toString());

    idx = query.record().indexOf("calibDate");
    ui->dtpCalibDate->setDate(query.value(idx).toDate());
}

void PageSetupDV::loadData()
{
    ui->cbCalibration->clear();

    QSqlQuery query;
    query.exec("SELECT * FROM DVLabHeader ORDER BY id DESC");
    while (query.next())
    {
        int idx = query.record().indexOf("id");
        int id = query.value(idx).toInt();

        idx = query.record().indexOf("cs_sn");
        QString str = query.value(idx).toString();

        ui->cbCalibration->addItem(str, id);
    }
}

void PageSetupDV::showCalibDataSection(bool value)
{
    ui->grpCalibrationData->setVisible(value);
}

PageSetupDV::~PageSetupDV()
{
    delete ui;
}
