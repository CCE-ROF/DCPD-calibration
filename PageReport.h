#ifndef PAGEREPORT_H
#define PAGEREPORT_H

#include <QWidget>

namespace Ui {
class PageReport;
}

class PageReport : public QWidget
{
    Q_OBJECT

public:
    explicit PageReport(QWidget *parent = nullptr);
    ~PageReport();

private:
    Ui::PageReport *ui;
    void loadData();

private slots:
    void printPreview();
    void exportResult();
    void deleteRecord();

};

#endif // PAGEREPORT_H
