#ifndef PAGEFACTORYCALIBRATIONSV_H
#define PAGEFACTORYCALIBRATIONSV_H

#include <QWidget>

namespace Ui {
class PageFactoryCalibrationSV;
}

class PageFactoryCalibrationSV : public QWidget
{
    Q_OBJECT

public:
    explicit PageFactoryCalibrationSV(QWidget *parent = nullptr);
    ~PageFactoryCalibrationSV();

private:
    void doPause(quint32 pauseMsec);

    Ui::PageFactoryCalibrationSV *ui;
    bool m_isRunning;

public slots:
    void run();
    void stop();

private slots:
    void btnAction();
};

#endif // PAGEFACTORYCALIBRATIONSV_H
