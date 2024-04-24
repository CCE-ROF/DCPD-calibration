#include "FlukeHelper.h"
#include <QDebug>
#include <QVariant>

FlukeHelper *FlukeHelper::m_instance;

FlukeHelper::FlukeHelper(QObject *parent)
    : QObject{parent}
{
    m_instance = this;
}

QString FlukeHelper::getParameter(QString param_name)
{
    QSqlQuery query;
    query.exec("SELECT * FROM Params WHERE param_name = '" + param_name + "'");
    query.first();
    int idx = query.record().indexOf("param_value");
    return query.value(idx).toString();
}

void FlukeHelper::setParameter(QString param_name, QString param_value)
{
    QString sqlStr = "UPDATE Params SET param_value = :param_value WHERE param_name = :param_name";
    QSqlQuery query;
    query.prepare(sqlStr);
    query.bindValue(":param_name", param_name);
    query.bindValue(":param_value", param_value);
    query.exec();

    if (query.lastError().type() != QSqlError::NoError)
        qDebug() << query.lastError().text();
}

double FlukeHelper::ampereFromResolution(QList<QPair<double, double>> *labValueList,
                                         double Reading_mV,
                                         int Resolution_number)
{
    int i = 1;
    if (i > labValueList->size())
        return 0;

    //                                              R    *   I
    while (qAbs(Reading_mV) >= labValueList->at(i).first * labValueList->at(i).second)
    {
        if (i >= Resolution_number)
        {
            i = Resolution_number;
            break;
        }
        i = i + 1;
    }

    double Ampere = 0;

    if (Reading_mV != 0)
    {
        double Rn   = labValueList->at(i).first;
        double Rn_1 = labValueList->at(i-1).first;
        double An   = labValueList->at(i).second;
        double An_1 = labValueList->at(i-1).second;

        double slope = (Rn - Rn_1) / (An - An_1);
        //double Offset = An - (slope * Rn);
        double Offset = Rn - (slope * An);
        double Resistor = (slope * Resolution_number + Offset) / 1000;
        Ampere = Reading_mV / Resistor;

        qDebug() << "    " << "ResNum:"  << Resolution_number
                           << "R:" << labValueList->at(i).first
                           << "I:" << labValueList->at(i).second
                           << "mV:" << Reading_mV
                           << "Amp:" << Ampere
                           << "Slope:" << slope
                           << "Offset" << Offset
                           << "Resistor" << Resistor
                              ;

    }
    else
    {
        Ampere = 0;
    }

    return Ampere;
}
