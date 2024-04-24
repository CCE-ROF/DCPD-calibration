#pragma once

#include "PageCheck.h"
#include <QWidget>

namespace Ui {
    class PageRun;
}

class PageRun : public QWidget
{
    Q_OBJECT

public:
    explicit PageRun(QWidget *parent = nullptr);
    void setPageCheck(PageCheck *page);
    void run();
    ~PageRun();

private:
    Ui::PageRun *ui;
    PageCheck *pageCheck;
    bool m_isRunning;
    int cuurentRow = 0;

    int calib_id = 0;
    QList<QPair<double, double>> labValueList;

    void runPWCalibration();
    void runAMPCalibration();
    void doPause(quint32 pauseMsec);
    void doInfoPause(quint32 pauseMsec);
    void insertIntoDB(int row, int runNo, double voltage, double ampere1, double ampere2);
    void loadDVCalibLabData();

private slots:
    void exportResult();
    void printPreview();


public slots:
    void stop();

};
