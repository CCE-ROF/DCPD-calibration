#include "PageAbout.h"
#include "ui_PageAbout.h"
#include "VFTLicense.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

PageAbout::PageAbout(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PageAbout)
{
    ui->setupUi(this);

    auto license = VFTLicense::instance();
    QString computer_id = license->getCPUID();
    ui->edtCPU_ID->setText(computer_id);
    ui->lblVersion->setText(tr("Version-number: ") + QApplication::applicationVersion());

    QString strWebsite = "Website: <a href='https://www.vectorforce.de/'>https://www.vectorforce.de/</a>";
    ui->lblWebsite->setText(strWebsite);
    QObject::connect(ui->lblWebsite, SIGNAL(linkActivated(const QString)), this, SLOT(openLink(const QString)));
}

void PageAbout::openLink(const QString url)
{
    QDesktopServices::openUrl(QUrl(url));
}

PageAbout::~PageAbout()
{
    delete ui;
}
