#pragma once

#include <QObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

class FlukeHelper : public QObject
{
    Q_OBJECT
public:
    static FlukeHelper *instance()
    {
        return m_instance;
    }

    explicit FlukeHelper(QObject *parent = nullptr);
    static QString getParameter(QString param_name);
    static void setParameter(QString param_name, QString param_value);
    static double ampereFromResolution(QList<QPair<double, double>> *labValueList, double Reading_mV, int Resolution_number);

private:
    static FlukeHelper *m_instance;

signals:

};
