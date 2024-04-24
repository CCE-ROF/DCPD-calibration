#include "PageSetupApp.h"
#include "ui_PageSetupApp.h"
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

PageSetupApp *PageSetupApp::m_instance;

PageSetupApp::PageSetupApp(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageSetupApp)
{
    ui->setupUi(this);
    m_instance = this;

    QObject::connect(ui->btnScan_1, &QPushButton::clicked, this, &PageSetupApp::fillPortsInfo);
    QObject::connect(ui->btnScan_2, &QPushButton::clicked, this, &PageSetupApp::fillPortsInfo);
    QObject::connect(ui->btnApply, &QPushButton::clicked, this, &PageSetupApp::updateSettings);
    QObject::connect(ui->serialPortInfoListBox_1, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PageSetupApp::showPortInfo_1);
    QObject::connect(ui->serialPortInfoListBox_2, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &PageSetupApp::showPortInfo_2);
    fillPortsParameters();
    fillPortsInfo();

    loadSettings();
}

void PageSetupApp::showPortInfo_1(int idx)
{
    if (idx == -1)
        return;

    QStringList list = ui->serialPortInfoListBox_1->itemData(idx).toStringList();
    ui->descriptionLabel_1->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    ui->manufacturerLabel_1->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    ui->serialNumberLabel_1->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    ui->locationLabel_1->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    ui->vidLabel_1->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    ui->pidLabel_1->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void PageSetupApp::showPortInfo_2(int idx)
{
    if (idx == -1)
        return;

    QStringList list = ui->serialPortInfoListBox_2->itemData(idx).toStringList();
    ui->descriptionLabel_2->setText(tr("Description: %1").arg(list.count() > 1 ? list.at(1) : tr(blankString)));
    ui->manufacturerLabel_2->setText(tr("Manufacturer: %1").arg(list.count() > 2 ? list.at(2) : tr(blankString)));
    ui->serialNumberLabel_2->setText(tr("Serial number: %1").arg(list.count() > 3 ? list.at(3) : tr(blankString)));
    ui->locationLabel_2->setText(tr("Location: %1").arg(list.count() > 4 ? list.at(4) : tr(blankString)));
    ui->vidLabel_2->setText(tr("Vendor Identifier: %1").arg(list.count() > 5 ? list.at(5) : tr(blankString)));
    ui->pidLabel_2->setText(tr("Product Identifier: %1").arg(list.count() > 6 ? list.at(6) : tr(blankString)));
}

void PageSetupApp::loadSettings()
{
    ui->serialPortInfoListBox_1->setCurrentText(FlukeHelper::getParameter("port1"));
    ui->baudRateBox_1->setCurrentText(FlukeHelper::getParameter("baudRate1"));
    ui->dataBitsBox_1->setCurrentText(FlukeHelper::getParameter("dataBits1"));
    ui->stopBitsBox_1->setCurrentText(FlukeHelper::getParameter("stopBits1"));
    ui->parityBox_1->setCurrentText(FlukeHelper::getParameter("parity1"));
    ui->flowControlBox_1->setCurrentText(FlukeHelper::getParameter("flowControl1"));

    ui->serialPortInfoListBox_2->setCurrentText(FlukeHelper::getParameter("port2"));
    ui->baudRateBox_2->setCurrentText(FlukeHelper::getParameter("baudRate2"));
    ui->dataBitsBox_2->setCurrentText(FlukeHelper::getParameter("dataBits2"));
    ui->stopBitsBox_2->setCurrentText(FlukeHelper::getParameter("stopBits2"));
    ui->parityBox_2->setCurrentText(FlukeHelper::getParameter("parity2"));
    ui->flowControlBox_2->setCurrentText(FlukeHelper::getParameter("flowControl2"));

    updateSettings();
}

void PageSetupApp::fillPortsParameters()
{
    ui->baudRateBox_1->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox_1->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox_1->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox_1->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox_1->setCurrentIndex(0);

    ui->baudRateBox_2->addItem(QStringLiteral("9600"), QSerialPort::Baud9600);
    ui->baudRateBox_2->addItem(QStringLiteral("19200"), QSerialPort::Baud19200);
    ui->baudRateBox_2->addItem(QStringLiteral("38400"), QSerialPort::Baud38400);
    ui->baudRateBox_2->addItem(QStringLiteral("115200"), QSerialPort::Baud115200);
    ui->baudRateBox_2->setCurrentIndex(0);

    ui->dataBitsBox_1->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox_1->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox_1->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox_1->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsBox_1->setCurrentIndex(3);

    ui->dataBitsBox_2->addItem(QStringLiteral("5"), QSerialPort::Data5);
    ui->dataBitsBox_2->addItem(QStringLiteral("6"), QSerialPort::Data6);
    ui->dataBitsBox_2->addItem(QStringLiteral("7"), QSerialPort::Data7);
    ui->dataBitsBox_2->addItem(QStringLiteral("8"), QSerialPort::Data8);
    ui->dataBitsBox_2->setCurrentIndex(3);

    ui->parityBox_1->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox_1->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox_1->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox_1->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox_1->addItem(tr("Space"), QSerialPort::SpaceParity);
    ui->parityBox_1->setCurrentIndex(0);

    ui->parityBox_2->addItem(tr("None"), QSerialPort::NoParity);
    ui->parityBox_2->addItem(tr("Even"), QSerialPort::EvenParity);
    ui->parityBox_2->addItem(tr("Odd"), QSerialPort::OddParity);
    ui->parityBox_2->addItem(tr("Mark"), QSerialPort::MarkParity);
    ui->parityBox_2->addItem(tr("Space"), QSerialPort::SpaceParity);
    ui->parityBox_2->setCurrentIndex(0);

    ui->stopBitsBox_1->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox_1->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox_1->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    ui->stopBitsBox_1->setCurrentIndex(2);

    ui->stopBitsBox_2->addItem(QStringLiteral("1"), QSerialPort::OneStop);
#ifdef Q_OS_WIN
    ui->stopBitsBox_2->addItem(tr("1.5"), QSerialPort::OneAndHalfStop);
#endif
    ui->stopBitsBox_2->addItem(QStringLiteral("2"), QSerialPort::TwoStop);
    ui->stopBitsBox_2->setCurrentIndex(2);

    ui->flowControlBox_1->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox_1->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox_1->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
    ui->flowControlBox_1->setCurrentIndex(0);


    ui->flowControlBox_2->addItem(tr("None"), QSerialPort::NoFlowControl);
    ui->flowControlBox_2->addItem(tr("RTS/CTS"), QSerialPort::HardwareControl);
    ui->flowControlBox_2->addItem(tr("XON/XOFF"), QSerialPort::SoftwareControl);
    ui->flowControlBox_2->setCurrentIndex(0);
}

void PageSetupApp::fillPortsInfo()
{
    ui->serialPortInfoListBox_1->clear();
    ui->serialPortInfoListBox_2->clear();

    QString description;
    QString manufacturer;
    QString serialNumber;
    const auto infos = QSerialPortInfo::availablePorts();

    for (const QSerialPortInfo &info : infos) {
        QStringList list;
        description = info.description();
        manufacturer = info.manufacturer();
        serialNumber = info.serialNumber();
        list << info.portName()
             << (!description.isEmpty() ? description : blankString)
             << (!manufacturer.isEmpty() ? manufacturer : blankString)
             << (!serialNumber.isEmpty() ? serialNumber : blankString)
             << info.systemLocation()
             << (info.vendorIdentifier() ? QString::number(info.vendorIdentifier(), 16) : blankString)
             << (info.productIdentifier() ? QString::number(info.productIdentifier(), 16) : blankString);

        ui->serialPortInfoListBox_1->addItem(list.first(), list);
        ui->serialPortInfoListBox_2->addItem(list.first(), list);
    }
}

void PageSetupApp::updateSettings()
{
    auto baudRate_1 = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox_1->itemData(ui->baudRateBox_1->currentIndex()).toInt());

    FlukeHelper::setParameter("port1", ui->serialPortInfoListBox_1->currentText());
    FlukeHelper::setParameter("baudRate1", QString::number(baudRate_1));
    FlukeHelper::setParameter("dataBits1", ui->dataBitsBox_1->currentText());
    FlukeHelper::setParameter("stopBits1", ui->stopBitsBox_1->currentText());
    FlukeHelper::setParameter("parity1", ui->parityBox_1->currentText());
    FlukeHelper::setParameter("flowControl1", ui->flowControlBox_1->currentText());

    auto baudRate_2 = static_cast<QSerialPort::BaudRate>(
                    ui->baudRateBox_2->itemData(ui->baudRateBox_2->currentIndex()).toInt());

    FlukeHelper::setParameter("port2", ui->serialPortInfoListBox_2->currentText());
    FlukeHelper::setParameter("baudRate2", QString::number(baudRate_2));
    FlukeHelper::setParameter("dataBits2", ui->dataBitsBox_2->currentText());
    FlukeHelper::setParameter("stopBits2", ui->stopBitsBox_2->currentText());
    FlukeHelper::setParameter("parity2", ui->parityBox_2->currentText());
    FlukeHelper::setParameter("flowControl2", ui->flowControlBox_2->currentText());
}

PageSetupApp::~PageSetupApp()
{
    delete ui;
}
