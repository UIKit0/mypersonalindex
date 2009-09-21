#include "queries.h"

//enum { closingPrices_Date, closingPrices_Ticker, closingPrices_Price, closingPrices_Change };
const QStringList queries::closingPricesColumns = QStringList() << "Date" << "Ticker" << "Price" << "Change";

//enum { splits_Date, splits_Ticker, splits_Ratio };
const QStringList queries::splitsColumns = QStringList() << "Date" << "Ticker" << "Ratio";

//enum { dividends_Date, dividends_Ticker, dividends_Amount };
const QStringList queries::dividendsColumns = QStringList() << "Date" << "Ticker" << "Amount";

//enum { dividends_Date, dividends_Ticker, dividends_Amount };
const QStringList queries::tradesColumns = QStringList() << "ID" << "Portfolio" << "TickerID" << "Ticker"
    << "Date" << "Shares" << "Price" << "Custom";

const QString queries::table_AA = "AA";
const QString queries::table_Acct = "Accounts";
const QString queries::table_AvgPricePerShare = "AvgPricePerShare";
const QString queries::table_ClosingPrices = "ClosingPrices";
const QString queries::table_Dividends = "Dividends";
const QString queries::table_NAV = "NAV";
const QString queries::table_Portfolios = "Portfolios";
const QString queries::table_Settings = "Settings";
const QString queries::table_Splits = "Splits";
const QString queries::table_Stats = "Stats";
const QString queries::table_Tickers = "Tickers";
const QString table_TickersAA = "TickersAA";
const QString table_TickersTrades = "TickersTrades";
const QString queries::table_Trades = "Trades";
const QString queries::table_UserStatistics = "UserStatistics";

queries::queries()
{
    QString location = getDatabaseLocation();
    if (QSqlDatabase::contains(location))
        db = QSqlDatabase::database(location);
    else
    {
        db = QSqlDatabase::addDatabase("QSQLITE", location);
        db.setDatabaseName(location);
        db.open();
        QSqlQuery("SELECT load_extension('libsqlitefunctions.so')", db);
    }
}

QString queries::getDatabaseLocation()
{
    QSettings cfg(QSettings::IniFormat, QSettings::UserScope, "MyPersonalIndex", "MPI");
    return QFileInfo(cfg.fileName()).absolutePath().append("/MPI.sqlite");
}

void queries::executeNonQuery(queryInfo *q)
{
    if (!q)
        return;

    QSqlQuery query(db);
    query.prepare(q->sql);
    foreach(const parameter &p, q->parameters)
        query.bindValue(p.name, p.value);

    query.exec();
    delete q;
}

QSqlQueryModel* queries::executeDataSet(queryInfo *q)
{
    if (!q)
        return 0;

    QSqlQuery query(db);
    query.prepare(q->sql);
    foreach(const parameter &p, q->parameters)
        query.bindValue(p.name, p.value);

    query.exec();

    if(!query.isActive())
    {
        delete q;
        return 0;
    }

    QSqlQueryModel *dataset = new QSqlQueryModel();
    dataset->setQuery(query);

    delete q;
    return dataset;
}

bool queries::executeTableSelect(QSqlTableModel *model, const QString &tableName, const int &sort, const QString &filter)
{
    if (!model)
        return false;

    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->setTable(tableName);
    if (!filter.isEmpty())
        model->setFilter(filter);
    if (sort != -1)
        model->setSort(sort, Qt::AscendingOrder);

    return model->select();
}

void queries::executeTableUpdate(const QString &tableName, const QMap<QString, QVariantList> &values)
{
    if (tableName.isEmpty() || values.isEmpty())
        return;

    db.transaction();

    QSqlQuery query(db);
    QStringList parameters, columns;
    QString sql("INSERT INTO %1(%2) VALUES (%3)");

    QMap<QString, QVariantList>::const_iterator i;
    for (i = values.begin(); i != values.end(); ++i)
    {
         parameters.append("?");
         columns.append(i.key());
     }

    query.prepare(sql.arg(tableName, columns.join(","), parameters.join(",")));

    QList<QVariantList> binds = values.values();
    int count = binds.at(0).count();
    for (int x = 1; x < binds.count(); x++)
        if (binds.at(x).count() != count) // all the lists must be the same size
        {
            db.commit();
            return;
        }

    for (int i = 0; i < count; i++)
    {
        for (int x = 0; x < binds.count(); x++)
            query.addBindValue(binds.at(x).at(i));
        query.exec();
    }

    db.commit();
}

QSqlQuery* queries::executeResultSet(queryInfo *q)
{
    if (!q)
        return 0;

    QSqlQuery *query = new QSqlQuery(db);
    query->setForwardOnly(true);
    query->prepare(q->sql);
    foreach(const parameter &p, q->parameters)
        query->bindValue(p.name, p.value);

    query->exec();
    delete q;

    if (query->isActive() && query->first())
        return query;

    return 0;
}

QVariant queries::executeScalar(queryInfo *q, const QVariant &nullValue)
{
    if (!q)
        return QVariant();

    QSqlQuery query(db);
    query.setForwardOnly(true);
    query.prepare(q->sql);
    foreach(const parameter &p, q->parameters)
        query.bindValue(p.name, p.value);

    query.exec();

    if (query.isActive() && query.first())
    {
        QVariant retValue = query.value(0);
        delete q;
        return retValue;
    }

    delete q;
    return nullValue;
}

queries::queryInfo* queries::getLastDate()
{
    return new queryInfo(
         "SELECT MAX(Date) from ClosingPrices",
        QList<parameter>()
    );
}

queries::queryInfo* queries::getIdentity()
{
    return new queryInfo(
        "SELECT last_insert_rowid()",
        QList<parameter>()
    );
}
