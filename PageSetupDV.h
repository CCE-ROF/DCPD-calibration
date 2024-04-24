#pragma once

#include <QWidget>
#include "CommunicationModule.h"
#include <QDateTime>

namespace Ui {
    class PageSetupDV;
}

struct SettingsDV {
    quint16 calibLab_id;
    QString calibLabSN;
    QDate calibLabDate;
};

class PageSetupDV : public QWidget
{
    Q_OBJECT
public:
    static PageSetupDV *instance()
    {
        return m_instance;
    }

    explicit PageSetupDV(QWidget *parent = nullptr);
    void updateSettings();
    void showCalibDataSection(bool value);
    ~PageSetupDV();

    SettingsDV currentSettings;

private:
    Ui::PageSetupDV *ui;
    static PageSetupDV *m_instance;

    void loadData();

public slots:
    void selectCalibration(int index);
    void selectFile();

signals:
    void changed();
};
