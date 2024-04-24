#include "PageCheck.h"
#include "ui_PageCheck.h"
#include "FlukeHelper.h"
#include "MainWindow.h"
#include <QFileDialog>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

PageCheck::PageCheck(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageCheck)
{
    ui->setupUi(this);
    btnRun = ui->btnRun;

    QObject::connect(ui->btnRun, &QPushButton::clicked, [=]()
    {
        auto helper = FlukeHelper::instance();
        helper->setParameter("delay", QString("%1").arg(ui->spnDelay->value()));
        helper->setParameter("runs", QString("%1").arg(ui->cbRuns->currentText()));
        helper->setParameter("range", QString("%1").arg(ui->cbRange->currentText()));
    });
}

Parameters PageCheck::parameters()
{
    params.delay = ui->spnDelay->value();
    params.runs = ui->cbRuns->currentText().toInt();
    params.range = ui->cbRange->currentData().toInt();

    return params;
}

void PageCheck::loadData()
{
    ui->cbRange->clear();
    if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::PWCalibration)
    {
        ui->cbRange->addItem("10A", 10);
        ui->cbRange->addItem("20A", 20);
        ui->cbRange->addItem("40A", 40);
        ui->cbRange->addItem("100A", 100);
    }
    else if (MainWindow::instance()->appMode() == MainWindow::ApplicationMode::AMPCalibration)
    {
        ui->cbRange->addItem("ALL", 0);
        ui->cbRange->addItem("500", 500);
        ui->cbRange->addItem("1000", 1000);
        ui->cbRange->addItem("2000", 2000);
        ui->cbRange->addItem("5000", 5000);
        ui->cbRange->addItem("10000", 10000);
        ui->cbRange->addItem("20000", 20000);
        ui->cbRange->addItem("40000", 40000);
    }

    ui->cbRange->setCurrentIndex(1);
    ui->cbRuns->setCurrentIndex(2);

    ui->spnDelay->setValue(FlukeHelper::getParameter("delay").toDouble());
    ui->cbRuns->setCurrentText(FlukeHelper::getParameter("runs"));
    ui->cbRange->setCurrentText(FlukeHelper::getParameter("range"));
}

PageCheck::~PageCheck()
{
    delete ui;
}
