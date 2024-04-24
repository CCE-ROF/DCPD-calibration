#include "CalCheckInfo.h"
#include "ui_CalCheckInfo.h"
#include "Reports.h"
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QMessageBox>
#include <QDebug>
#include "FlukeHelper.h"
#include "MainWindow.h"

CalCheckInfo::CalCheckInfo(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CalCheckInfo)
{
    ui->setupUi(this);
    calib_id = 0;
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);

    QObject::connect(ui->btnOK, &QPushButton::clicked, this, &CalCheckInfo::OK);
}

void CalCheckInfo::showThis()
{
    QDateTime dt = QDateTime::currentDateTime();
    ui->lblDate->setText(dt.toString());

    if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::PWCalibration)
        ui->lblCalibEq->setText("VFT Shunt Device, S/N");
    else if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::AMPCalibration)
        ui->lblCalibEq->setText("CS-01, SN");

    if (exec())
    {
        if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::PWCalibration)
        {
            QString sqlStr = "insert into SVCalCheckHeader (date_calib, operator, model, sn, temp, FlukeSN, DividerSN, file_name, runs) "
                             "values (:date_calib, :operator, :model, :sn, :temp, :FlukeSN, :DividerSN, :file_name, :runs)";
            QSqlQuery query;
            query.prepare(sqlStr);
            query.bindValue(":date_calib", dt.date());
            query.bindValue(":operator", ui->edtUser->text());
            query.bindValue(":model", ui->edtModel->text());
            query.bindValue(":sn", ui->edtSN->text());
            query.bindValue(":temp", ui->edtTemp->text());
            query.bindValue(":FlukeSN", ui->edtCalib->text());
            query.bindValue(":DividerSN", ui->edtDivider->text());
            query.bindValue(":file_name", "");
            query.bindValue(":runs", FlukeHelper::getParameter("runs"));
            query.exec();

            sqlStr = "select max(id) as calib_id from SVCalCheckHeader";
            query.clear();
            query.exec(sqlStr);
            query.first();
            calib_id = query.value(0).toInt();
        }
        else if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::AMPCalibration)
        {
            QString sqlStr = "insert into DVCalCheckHeader (date_calib, operator, model, sn, temp, FlukeSN, EqSN, file_name, runs) "
                             "values (:date_calib, :operator, :model, :sn, :temp, :FlukeSN, :EqSN, :file_name, :runs)";
            QSqlQuery query;
            query.prepare(sqlStr);
            query.bindValue(":date_calib", dt.date());
            query.bindValue(":operator", ui->edtUser->text());
            query.bindValue(":model", ui->edtModel->text());
            query.bindValue(":sn", ui->edtSN->text());
            query.bindValue(":temp", ui->edtTemp->text());
            query.bindValue(":FlukeSN", ui->edtCalib->text());
            query.bindValue(":EqSN", ui->edtDivider->text());
            query.bindValue(":file_name", "");
            query.bindValue(":runs", FlukeHelper::getParameter("runs"));
            query.exec();

            sqlStr = "select max(id) as calib_id from SVCalCheckHeader";
            query.clear();
            query.exec(sqlStr);
            query.first();
            calib_id = query.value(0).toInt();
        }
    }
}

void CalCheckInfo::OK()
{
    if (
            ui->edtCalib->text().isEmpty() ||
            ui->edtDivider->text().isEmpty() ||
            ui->edtModel->text().isEmpty() ||
            ui->edtSN->text().isEmpty() ||
            ui->edtTemp->text().isEmpty() ||
            ui->edtUser->text().isEmpty()
       )
    {
        QMessageBox::warning(this, tr("Error"), tr("Please fill all fields"));
        return;
    }
    this->accept();
}

CalCheckInfo::~CalCheckInfo()
{
    delete ui;
}
