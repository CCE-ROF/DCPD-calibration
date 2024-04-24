#include "Reports.h"
#include "xlsxdocument.h"

Reports *Reports::m_instance;

Reports::Reports(QObject *parent)
    : QObject{parent}
{
    m_instance = this;
    calib_id = 0;
}

void Reports::printPreview(int calib_id)
{
    if (calib_id == 0)
        calib_id = getLastCheck();

    this->calib_id = calib_id;

    QDir dir(qApp->applicationDirPath());
    QString fileName = dir.absolutePath() + "/CalibData.xml";
    auto report = new QtRPT(this);

    if (report->loadReport(fileName) == false)
        qDebug()<<"Report file not found";

    QObject::connect(report, SIGNAL(setValue(const int, const QString, QVariant&, const int)),
                     this, SLOT(setValue(const int, const QString, QVariant&, const int)));
    QObject::connect(report, SIGNAL(setValueImage(const int, const QString, QImage&, const int)),
                     this, SLOT(setValueImage(const int, const QString, QImage&, const int)));
    QObject::connect(report, SIGNAL(setDSInfo(DataSetInfo &)),
                     this, SLOT(setDSInfo(DataSetInfo &)));

    loadResultData();
    report->printExec(true);
}

int Reports::getLastCheck()
{
    QSqlQuery query;
    query.exec("SELECT id FROM SVCalCheckHeader ORDER BY id DESC LIMIT 1");
    query.first();
    int idx = query.record().indexOf("id");
    return query.value(idx).toInt();
}

void Reports::exportData(int calib_id)
{
    if (calib_id == 0)
        calib_id = getLastCheck();

    this->calib_id = calib_id;
    loadResultData();

    //---------------------------
    int idx = query_report.record().indexOf("model");
    QString model = query_report.value(idx).toString();

    idx = query_report.record().indexOf("sn");
    QString sn = query_report.value(idx).toString();

    idx = query_report.record().indexOf("operator");
    QString user = query_report.value(idx).toString();

    idx = query_report.record().indexOf("date_calib");
    QString date = query_report.value(idx).toString();

    idx = query_report.record().indexOf("temp");
    QString temp = query_report.value(idx).toString();

    idx = query_report.record().indexOf("FlukeSN");
    QString fluke = query_report.value(idx).toString();

    idx = query_report.record().indexOf("DividerSN");
    QString divider = query_report.value(idx).toString();
    //---------------------------
    QXlsx::Format formatHeader;
    formatHeader.setFontBold(true);
    formatHeader.setFontSize(12);

    QXlsx::Format topLeftBorder;
    topLeftBorder.setTopBorderStyle(QXlsx::Format::BorderThin);
    topLeftBorder.setLeftBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::Format topRightBorder;
    topRightBorder.setTopBorderStyle(QXlsx::Format::BorderThin);
    topRightBorder.setRightBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::Format topBorder;
    topBorder.setTopBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::Format bottomLeftBorder;
    bottomLeftBorder.setBottomBorderStyle(QXlsx::Format::BorderThin);
    bottomLeftBorder.setLeftBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::Format bottomRightBorder;
    bottomRightBorder.setBottomBorderStyle(QXlsx::Format::BorderThin);
    bottomRightBorder.setRightBorderStyle(QXlsx::Format::BorderThin);

    QXlsx::Format bottomBorder;
    bottomBorder.setBottomBorderStyle(QXlsx::Format::BorderThin);


    QXlsx::Document xlsx;
    for (int i = 1; i < 8; i++)
        xlsx.setColumnWidth(i, 15);
    xlsx.write("B4", "Calibration", formatHeader);
    xlsx.write("C4", "" + model, formatHeader);
    xlsx.write("D4", "S/N " + sn, formatHeader);

    //xlsx.write("B5", "Model Information", formatHeader);
    xlsx.write("B6", "Date:", topLeftBorder);
    xlsx.write("C6", date, topBorder);
    xlsx.write("D6", "", topBorder);
    xlsx.write("E6", "Temp", topBorder);
    xlsx.write("F6", temp, topBorder);
    xlsx.write("G6", "C", topRightBorder);

    xlsx.write("B7", "Technician", bottomLeftBorder);
    xlsx.write("C7", user, bottomBorder);
    xlsx.write("D7", "", bottomBorder);
    xlsx.write("E7", "", bottomBorder);
    xlsx.write("F7", "", bottomBorder);
    xlsx.write("G7", "", bottomRightBorder);

    xlsx.write("B9", "Calibration Tool", topLeftBorder);
    xlsx.write("C9", "", topBorder);
    xlsx.write("D9", "Fluke 7746A", topBorder);
    xlsx.write("E9", "S/N", topBorder);
    xlsx.write("F9", fluke, topBorder);
    xlsx.write("G9", "", topRightBorder);

    xlsx.write("B10", "", bottomLeftBorder);
    xlsx.write("C10", "", bottomBorder);
    xlsx.write("D10", "Shunt Device", bottomBorder);
    xlsx.write("E10", "S/N", bottomBorder);
    xlsx.write("F10", divider, bottomBorder);
    xlsx.write("G10", divider, bottomRightBorder);

    QXlsx::Format cell_format;
    cell_format.setBorderStyle(QXlsx::Format::BorderThin);

    xlsx.write("B13", "Set Value, A", cell_format);
    xlsx.write("C13", "Output, A", cell_format);
    xlsx.write("D13", "Abs Error, ma", cell_format);
    xlsx.write("E13", "Error, %", cell_format);
    xlsx.write("F13", "Tolerance, %", cell_format);
    xlsx.write("G13", "Acceptance", cell_format);

    int row = 14;
    while (query_report.next())
    {
        idx = query_report.record().indexOf("RowNo");
        QString value = QString::number(query_report.value(idx).toDouble(), 'f', 3);
        xlsx.write(QString("B%1").arg(row), value, cell_format);

        idx = query_report.record().indexOf("Aao");
        value = QString::number(query_report.value(idx).toDouble(), 'f', 4);
        xlsx.write(QString("C%1").arg(row), value, cell_format);

        idx = query_report.record().indexOf("diff");
        value = QString::number(query_report.value(idx).toDouble(), 'f', 4);
        xlsx.write(QString("D%1").arg(row), value, cell_format);

        idx = query_report.record().indexOf("error");
        value = QString::number(query_report.value(idx).toDouble(), 'f', 4);
        xlsx.write(QString("E%1").arg(row), value, cell_format);

        xlsx.write(QString("F%1").arg(row), "0.1", cell_format);

        QString comp = qAbs(query_report.value(idx).toDouble()) <= 0.1 ? "PASS" : "FAILED";
        xlsx.write(QString("G%1").arg(row), comp, cell_format);

        row++;
    }

    QWidget *w = qobject_cast<QWidget*>(this->parent());
    QString fileName = QFileDialog::getSaveFileName(w, tr("Save File"), "", tr("XLSX Files (*.xlsx)"));
    if (fileName.isEmpty() || fileName.isNull() )
        return;

    xlsx.saveAs(fileName);
}

void Reports::setValue(const int recNo, const QString paramName, QVariant &paramValue, const int reportPage)
{
    Q_UNUSED(reportPage);

    QString colName = "";
    if (paramName == "model")
        colName = "model";
    else if (paramName == "SN")
        colName = "sn";
    else if (paramName == "Date")
        colName = "date_calib";
    else if (paramName == "Technician")
        colName = "operator";
    else if (paramName == "Temp")
        colName = "temp";
    else if (paramName == "deviceSN")
        colName = "FlukeSN";
    else if (paramName == "shuntSN")
        colName = "DividerSN";
    else if (paramName == "value1")
        colName = "RowNo";
    else if (paramName == "value2")
        colName = "Aao";
    else if (paramName == "value3")
        colName = "diff";
    else if (paramName == "value4")
        colName = "error";
    else if (paramName == "value5")
    {
        paramValue = "0.1";
        return;
    }
    else if (paramName == "value6")
    {
        query_report.seek(recNo);
        int idx = query_report.record().indexOf("error");
        paramValue =  qAbs(query_report.value(idx).toDouble()) <= 0.1 ? "PASS" : "FAILED";
        return;
    }
    else
    {
        paramValue = "";
        return;
    }

    query_report.seek(recNo);
    int idx = query_report.record().indexOf(colName);
    if (idx == -1)
    {
        paramValue = "";
    }
    else
    {
        if (colName == "error")
            paramValue =  QString::number(query_report.value(idx).toDouble(), 'f', 4);
        else if (colName == "diff")
            paramValue =  QString::number(query_report.value(idx).toDouble()*1000, 'f', 1);
        else if (colName == "Aao")
            paramValue =  QString::number(query_report.value(idx).toDouble(), 'f', 4);
        else if (colName == "Aa" || colName == "Va" || colName == "RowNo")
            paramValue =  QString::number(query_report.value(idx).toDouble(), 'f', 3);
        else
            paramValue = query_report.value(idx).toString();
    }
}

void Reports::setValueImage(const int recNo, const QString paramName, QImage &paramValue, const int reportPage)
{
    Q_UNUSED(recNo);
    Q_UNUSED(reportPage);

    if (paramName == "image")
    {
        auto image = new QImage(QCoreApplication::applicationDirPath()+"/logo.png");
        paramValue = *image;
    }
}

void Reports::setDSInfo(DataSetInfo &dsInfo)
{
    dsInfo.recordCount = reportRecordCount;
}

void Reports::loadResultData()
{
    reportRecordCount = 0;

    QSqlQuery query;
    query.exec("SELECT runs FROM SVCalCheckHeader WHERE id = '" + QString::number(calib_id) + "'");
    query.first();
    int idx = query.record().indexOf("runs");
    int runs = query.value(idx).toInt();


    QString Va  = "(";
    QString Aa  = "(";
    QString Aao = "(";
    for (int i = 1; i <= runs; i++)
    {
        if (i != runs)
        {
            Va  = Va  + (QString("cd.v%1+").arg(i));
            Aa  = Aa  + (QString("cd.a%1+").arg(i));
            Aao = Aao + (QString("cd.ao%1+").arg(i));
        }
        else
        {
            Va  = Va  + (QString("cd.v%1").arg(i));
            Aa  = Aa  + (QString("cd.a%1").arg(i));
            Aao = Aao + (QString("cd.ao%1").arg(i));
        }
    }
    Va  = Va + ")/" + QString::number(runs);
    Aa  = Aa + ")/" + QString::number(runs);
    Aao = Aao + ")/" + QString::number(runs);

    QString sqlStr = "SELECT ch.*, "
                     "RowNo, " +
                     Va  + " as Va, " +
                     Aa  + " as Aa,  " +
                     Aao + " as Aao,  " +
                     "RowNo - " + Aao + " as diff, " +
                     "(RowNo - " + Aao + ") * 100 / RowNo as error " +
                     " FROM SVCalCheckHeader ch "
                     "LEFT OUTER JOIN SVCalCheckData cd ON cd.header_id = ch.id "
                     "WHERE ch.id = " + QString::number(calib_id);
    qDebug() << sqlStr;
    query_report.exec(sqlStr);

    query_report.last();
    reportRecordCount = query_report.at()+1;
    query_report.seek(0);
}

