#pragma once

#include <QWidget>
#include "CommunicationModule.h"
#include <QDateTime>

namespace Ui {
    class PageSetupApp;
}

class PageSetupApp : public QWidget
{
    Q_OBJECT

public:
    static PageSetupApp *instance()
    {
        return m_instance;
    }

    explicit PageSetupApp(QWidget *parent = nullptr);
    void loadSettings();
    void updateSettings();
    void fillPortsParameters();
    void showCalibDataSection(bool value);
    ~PageSetupApp();

private:
    Ui::PageSetupApp *ui;
    static PageSetupApp *m_instance;

    void loadData();

public slots:
    void fillPortsInfo();
    void showPortInfo_1(int idx);
    void showPortInfo_2(int idx);

signals:
    void changed();

};
