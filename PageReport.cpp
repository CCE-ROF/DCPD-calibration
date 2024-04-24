#include "PageReport.h"
#include "ui_PageReport.h"
#include "Reports.h"

PageReport::PageReport(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageReport)
{
    ui->setupUi(this);

    QPalette palette = ui->btnExport->palette();
    palette.setColor(QPalette::Button, QColor(0,255,0));
    //ui->btnExport->setPalette(palette);
    //ui->btnPreview->setPalette(palette);

    palette.setColor(QPalette::Button, QColor(255,0,0));
    ui->btnDelete->setPalette(palette);

    loadData();

    QObject::connect(ui->btnDelete, &QPushButton::clicked, this, &PageReport::deleteRecord);
    QObject::connect(ui->btnExport, &QPushButton::clicked, this, &PageReport::exportResult);
    QObject::connect(ui->btnPreview, &QPushButton::clicked, this, &PageReport::printPreview);
}

void PageReport::loadData()
{
    ui->tblHeaders->setRowCount(0);

    QSqlQuery query;
    query.exec("SELECT * FROM SVCalCheckHeader ORDER BY id DESC");
    while (query.next())
    {
        ui->tblHeaders->setRowCount(ui->tblHeaders->rowCount() + 1);

        int idx = query.record().indexOf("id");
        int id = query.value(idx).toInt();

        idx = query.record().indexOf("date_calib");
        QString date = query.value(idx).toString();

        idx = query.record().indexOf("model");
        QString model = query.value(idx).toString();

        idx = query.record().indexOf("sn");
        QString sn = query.value(idx).toString();

        idx = query.record().indexOf("temp");
        QString temp = query.value(idx).toString();

        idx = query.record().indexOf("operator");
        QString technician = query.value(idx).toString();

        idx = query.record().indexOf("FlukeSN");
        QString fluke = query.value(idx).toString();

        idx = query.record().indexOf("DividerSN");
        QString shunt = query.value(idx).toString();

        auto item = new QTableWidgetItem(date);
        item->setData(Qt::UserRole, id);
        ui->tblHeaders->setItem(ui->tblHeaders->rowCount()-1, 0, item);

        item = new QTableWidgetItem(model);
        ui->tblHeaders->setItem(ui->tblHeaders->rowCount()-1, 1, item);

        item = new QTableWidgetItem(sn);
        ui->tblHeaders->setItem(ui->tblHeaders->rowCount()-1, 2, item);

        item = new QTableWidgetItem(temp);
        ui->tblHeaders->setItem(ui->tblHeaders->rowCount()-1, 3, item);

        item = new QTableWidgetItem(technician);
        ui->tblHeaders->setItem(ui->tblHeaders->rowCount()-1, 4, item);

        item = new QTableWidgetItem(fluke);
        ui->tblHeaders->setItem(ui->tblHeaders->rowCount()-1, 5, item);

        item = new QTableWidgetItem(shunt);
        ui->tblHeaders->setItem(ui->tblHeaders->rowCount()-1, 6, item);
    }
}

void PageReport::deleteRecord()
{
    if (ui->tblHeaders->currentRow() == -1)
        return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, tr("Deleting"),tr("The data will be permanent deleted.\nAre you sure?"),
                                     QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes)
    {
        int id = ui->tblHeaders->item(ui->tblHeaders->currentRow(), 0)->data(Qt::UserRole).toInt();
        QSqlQuery query;
        query.exec("DELETE FROM SVCalCheckHeader WHERE id = " + QString::number(id));
        if (query.lastError().type() != QSqlError::NoError)
            qDebug() << query.lastError().text();

        loadData();
    }
}

void PageReport::printPreview()
{
    if (ui->tblHeaders->currentRow() == -1)
        return;

    auto reports = Reports::instance();
    reports->printPreview(ui->tblHeaders->item(ui->tblHeaders->currentRow(), 0)->data(Qt::UserRole).toInt());
}

void PageReport::exportResult()
{
    if (ui->tblHeaders->currentRow() == -1)
        return;

    auto reports = Reports::instance();
    reports->exportData(ui->tblHeaders->item(ui->tblHeaders->currentRow(), 0)->data(Qt::UserRole).toInt());
}

PageReport::~PageReport()
{
    delete ui;
}
