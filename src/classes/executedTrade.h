#ifndef EXECUTEDTRADE_H
#define EXECUTEDTRADE_H

#include <QMap>
#include "queriesBatch.h"
#include "objectBase.h"

class QSqlQuery;
class executedTrade
{
public:

    double shares;
    double price;
    double commission;
    int associatedTradeID;

    executedTrade():
            shares(0),
            price(0),
            commission(0),
            associatedTradeID(0)
    {}

    executedTrade(double shares_, double price_, double commission_, int associatedTradeID_):
        shares(shares_),
        price(price_),
        commission(commission_),
        associatedTradeID(associatedTradeID_)
    {}

    static executedTrade load(const QSqlQuery &q_);

    bool operator==(const executedTrade &other_) const;
    bool operator!=(const executedTrade &other_) const { return !(*this == other_); }
};

template <class T, class V>
class QHash;
class executedTradeMap: public objectBase, public queriesBatch
{
public:
    executedTradeMap(int parent_):
        objectBase(parent_)
    {}

    QMap<int, executedTrade>::const_iterator lowerBound(int date_) const { return m_trades.lowerBound(date_); }
    QMap<int, executedTrade>::const_iterator constBegin() const { return m_trades.constBegin(); }
    QMap<int, executedTrade>::const_iterator constEnd() const { return m_trades.constEnd(); }

    void insert(int date_, const executedTrade &executedTrade_);

    void updateAssociatedTradeID(const QHash<int, int> &tradeIDMapping_);

    void remove();
    void remove(int beginDate_);

    bool operator==(const executedTradeMap &other_) const { return m_trades == other_.m_trades; }
    bool operator!=(const executedTradeMap &other_) const { return !(*this == other_); }

    void insertBatch(queries dataSource_);

    int rowsToBeInserted() const { return m_trades.count(); }
    QVariant data(int row_, int column_) const;

private:
   QMap<int, executedTrade> m_trades;
};

#endif // EXECUTEDTRADE_H