#pragma once

#include <QDialog>

namespace Ui {
    class CalCheckInfo;
}

class CalCheckInfo : public QDialog
{
    Q_OBJECT

public:
    explicit CalCheckInfo(QWidget *parent = nullptr);
    void showThis();
    ~CalCheckInfo();

    int calib_id;

private:
    Ui::CalCheckInfo *ui;

private slots:
    void OK();
};
