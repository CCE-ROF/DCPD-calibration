#include "PageSetupSV.h"
#include "ui_PageSetupSV.h"
#include <QDateTime>
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include <QtSerialPort/QSerialPortInfo>
#include "FlukeHelper.h"

static const char blankString[] = QT_TRANSLATE_NOOP("SettingsDlg", "N/A");

PageSetupSV *PageSetupSV::m_instance;

PageSetupSV::PageSetupSV(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageSetupSV)
{
    ui->setupUi(this);
    m_instance = this;

    QObject::connect(ui->btnSelectFile, SIGNAL(clicked()), this, SLOT(selectFile()));
    QObject::connect(ui->btnApply, &QPushButton::clicked, this, &PageSetupSV::updateSettings);
    QObject::connect(ui->cbCalibration, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PageSetupSV::selectCalibration);
    QObject::connect(ui->btnToday, &QToolButton::clicked,  [=]()
    {
        ui->dtpCalibDate->setDate(QDate::currentDate());
    });

    loadData();

    ui->cbCalibration->setCurrentIndex(0);
    updateSettings();
}

void PageSetupSV::updateSettings()
{
    QString sqlStr = "UPDATE SVLabHeader SET "
                     "shunt_sn = :shunt_sn, "
                     "calibDate = :calibDate, "
                     "resistor = :resistor, "
                     "linearsation = :linearsation "
                     "WHERE id = :id";
    QSqlQuery query;
    query.prepare(sqlStr);
    query.bindValue(":shunt_sn", ui->edtShutCalSN->text());
    query.bindValue(":calibDate", ui->dtpCalibDate->date());
    query.bindValue(":resistor", ui->spnResistor->value());
    query.bindValue(":linearsation", ui->edtLinerization->text());
    query.bindValue(":id", ui->cbCalibration->currentData().toInt());
    query.exec();

    currentSettings.calibLab_id = ui->cbCalibration->currentData().toInt();
    currentSettings.calibLabSN = ui->edtShutCalSN->text();
    currentSettings.calibLabDate = ui->dtpCalibDate->date();
    currentSettings.calibLabResistor = ui->spnResistor->value();
    currentSettings.calibLabLinearsation = ui->edtLinerization->text();
}

void PageSetupSV::selectFile()
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
    int calib_id = 0;

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
        if (lineNo == 2)
            currentSettings.calibLabResistor = list[1].replace("\r\n", "").toDouble();
        if (lineNo == 3)
            currentSettings.calibLabLinearsation = list[1].replace("\r\n", "");

        if (lineNo == 3)
        {
            sqlStr = "INSERT INTO SVLabHeader (shunt_sn, calibDate, resistor, linearsation) "
                     "VALUES (:shunt_sn, :calibDate, :resistor, :linearsation);";
            query.prepare(sqlStr);
            query.bindValue(":shunt_sn", currentSettings.calibLabSN);
            query.bindValue(":calibDate", currentSettings.calibLabDate);
            query.bindValue(":resistor", currentSettings.calibLabResistor);
            query.bindValue(":linearsation", currentSettings.calibLabLinearsation);
            query.exec();

            if (query.lastError().type() != QSqlError::NoError)
                qDebug() << query.lastError().text();

            sqlStr = "select max(id) as calib_id from SVLabHeader";
            query.clear();
            query.exec(sqlStr);

            if (query.lastError().type() != QSqlError::NoError)
                qDebug() << query.lastError().text();

            query.first();

            calib_id = query.value(0).toInt();
        }

        if (lineNo > 3 && calib_id != 0)
        {
            sqlStr = "INSERT INTO SVLabData (header_id, resistor, ampere)"
                     "VALUES (:header_id, :resistor, :ampere)";
            query.prepare(sqlStr);
            query.bindValue(":header_id", calib_id);
            query.bindValue(":resistor", list.at(1));
            query.bindValue(":ampere", list.at(0));
            query.exec();

            if (query.lastError().type() != QSqlError::NoError)
                qDebug() << query.lastError().text();
        }

        lineNo++;
    }

    loadData();
    ui->cbCalibration->setCurrentIndex(0);
}

void PageSetupSV::selectCalibration(int index)
{
    if (index == -1)
        return;

    int id = ui->cbCalibration->currentData().toInt();

    QSqlQuery query;
    query.exec("SELECT * FROM SVLabHeader WHERE id = " + QString::number(id));
    query.first();

    int idx = query.record().indexOf("shunt_sn");
    ui->edtShutCalSN->setText(query.value(idx).toString());

    idx = query.record().indexOf("calibDate");
    ui->dtpCalibDate->setDate(query.value(idx).toDate());

    idx = query.record().indexOf("resistor");
    ui->spnResistor->setValue(query.value(idx).toDouble());

    idx = query.record().indexOf("linearsation");
    ui->edtLinerization->setText(query.value(idx).toString());

    currentSettings.calibLab_id = ui->cbCalibration->currentData().toInt();
    currentSettings.calibLabSN = ui->edtShutCalSN->text();
    currentSettings.calibLabDate = ui->dtpCalibDate->date();
    currentSettings.calibLabResistor = ui->spnResistor->value();
    currentSettings.calibLabLinearsation = ui->edtLinerization->text();
}

void PageSetupSV::loadData()
{
    ui->cbCalibration->clear();

    QSqlQuery query;
    query.exec("SELECT * FROM SVLabHeader ORDER BY id DESC");
    while (query.next())
    {
        int idx = query.record().indexOf("id");
        int id = query.value(idx).toInt();

        idx = query.record().indexOf("shunt_sn");
        QString str = query.value(idx).toString();

        ui->cbCalibration->addItem(str, id);
    }
}

void PageSetupSV::showCalibDataSection(bool value)
{
    ui->grpCalibrationData->setVisible(value);
}

QList<QPair<double, double>> PageSetupSV::loadSVCalibLabData(int header_id)
{
    QList<QPair<double, double>> list;
    QSqlQuery query;
    QString sqlStr = "SELECT * FROM SVLabData WHERE header_id = " + QString::number(header_id);
    query.exec(sqlStr);
    while (query.next())
    {
        QPair<double, double> pair;

        int idx = query.record().indexOf("resistor");
        pair.first = query.value(idx).toDouble();

        idx = query.record().indexOf("ampere");
        pair.second = query.value(idx).toDouble();

        list << pair;
    }

    return list;
}

PageSetupSV::~PageSetupSV()
{
    delete ui;
}
