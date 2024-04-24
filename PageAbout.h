#ifndef PAGEABOUT_H
#define PAGEABOUT_H

#include <QWidget>

namespace Ui {
    class PageAbout;
}

class PageAbout : public QWidget
{
    Q_OBJECT

public:
    explicit PageAbout(QWidget *parent = nullptr);
    ~PageAbout();

private:
    Ui::PageAbout *ui;

private slots:
    void openLink(const QString url);

};

#endif // PAGEABOUT_H
