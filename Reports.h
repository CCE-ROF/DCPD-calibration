#pragma once

#include <QObject>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <qtrpt.h>

class Reports : public QObject
{
    Q_OBJECT
public:
    static Reports *instance()
    {
        return m_instance;
    }

    explicit Reports(QObject *parent = nullptr);

private:
    static Reports *m_instance;
    QSqlQuery query_report;
    int reportRecordCount;
    int calib_id;
    void loadResultData();
    int getLastCheck();

public slots:
    void printPreview(int calib_id);
    void exportData(int calib_id);
    void setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage);
    void setValueImage(const int recNo, const QString paramName, QImage &paramValue, const int reportPage);
    void setDSInfo(DataSetInfo &dsInfo);

signals:

};
