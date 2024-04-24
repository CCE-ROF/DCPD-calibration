#ifndef PAGECHECK_H
#define PAGECHECK_H

#include <QWidget>
#include <QPushButton>
#include <QtSql>

namespace Ui {
    class PageCheck;
}

struct Parameters {
    quint16 runs;
    quint16 range;
    quint16 delay;
};

class PageCheck : public QWidget
{
    Q_OBJECT

public:
    explicit PageCheck(QWidget *parent = nullptr);
    QPushButton *btnRun;
    Parameters parameters();
    void loadData();
    ~PageCheck();

private:
    Ui::PageCheck *ui;
    Parameters params;

private slots:


};

#endif // PAGECHECK_H
