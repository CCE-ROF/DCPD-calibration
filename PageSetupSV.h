#pragma once

#include <QWidget>
#include <QDateTime>

namespace Ui {
    class PageSetupSV;
}

struct SettingsSV {
    quint16 calibLab_id;
    QString calibLabSN;
    QDate calibLabDate;
    double calibLabResistor;
    QString calibLabLinearsation;
};

class PageSetupSV : public QWidget
{
    Q_OBJECT
public:
    static PageSetupSV *instance()
    {
        return m_instance;
    }

    explicit PageSetupSV(QWidget *parent = nullptr);
    void updateSettings();
    void showCalibDataSection(bool value);
    QList<QPair<double, double> > loadSVCalibLabData(int header_id);
    ~PageSetupSV();

    SettingsSV currentSettings;

private:
    Ui::PageSetupSV *ui;
    static PageSetupSV *m_instance;

    void loadData();

public slots:
    void selectCalibration(int index);
    void selectFile();

signals:
    void changed();
};
