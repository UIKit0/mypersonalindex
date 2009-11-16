#ifndef FRMTICKER_H
#define FRMTICKER_H

#include <QtGui>
#include "frmTicker_UI.h"
#include "globals.h"
#include "queries.h"
#include "mpiModelBase.h"
#include "frmTrade.h"
#include "functions.h"

class tickerAAModel;
class tickerTradeModel;
class frmTicker : public QDialog
{
    Q_OBJECT

public:

    const globals::security& getReturnValues() const { return m_security; }
    //const int &getTickerID() const { return m_security.id; }
    const QMap<int, globals::security>& getCashAccounts() const { return m_data.tickers; }

    frmTicker(const int &portfolioID, const globals::portfolioData &data, const globals::security& security, const queries &sql, QWidget *parent = 0);

private:

    frmTicker_UI ui;
    int m_portfolioID;
    const globals::portfolioData &m_data;
    globals::security m_security;
    globals::security m_securityOriginal;
    const queries &m_sql;
    tickerTradeModel *m_modelTrade;
    tickerAAModel *m_modelAA;

    void connectSlots();
    void loadSecurity();
    void loadDropDowns();
    void installAAModel();

private slots:
    void accept();
    void resetExpense();
    void addAA();
    void updateAAPercentage();
    void saveItem(globals::dynamicTrade *trade);
    void deleteItem(const globals::dynamicTrade &trade);
    void customContextMenuRequested(const QPoint&);
};

class tickerAAModel: public QAbstractTableModel
{
    Q_OBJECT

public:

    QList<globals::tickerAATarget> getList() { return m_list; }

    tickerAAModel(const QList<globals::tickerAATarget> &values, const QMap<int, globals::assetAllocation> &aaValues, const int &cols, QTableView *parent = 0):
            QAbstractTableModel(parent), m_aaValues(aaValues), m_parent(parent), m_columns(cols), m_list(values) {}

    Qt::ItemFlags flags(const QModelIndex &index) const
    {
        Qt::ItemFlags flags = QAbstractTableModel::flags(index);
        if (index.column() == 1)
                flags = flags | Qt::ItemIsEditable;

        return flags;
    }

    int rowCount(const QModelIndex&) const
    {
        return m_list.count();
    }

    int columnCount (const QModelIndex&) const
    {
        return m_columns;
    }

    double totalPercentage()
    {
        double total = 0;

        foreach(const globals::tickerAATarget &aa, m_list)
            total += aa.second;

        return total;
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_list.size())
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            if (index.column() == 0)
                return m_aaValues.value(m_list.at(index.row()).first).description;

            if (index.column() == 1)
                return functions::doubleToPercentage(m_list.at(index.row()).second);
        }

        if (role == Qt::EditRole)
        {
            if (index.column() == 0)
                return m_aaValues.value(m_list.at(index.row()).first).description;

            if (index.column() == 1)
                return m_list.at(index.row()).second;
        }

        return QVariant();
    }

    QVariant headerData (int, Qt::Orientation, int) const
    {
        return QVariant();
    }

    bool setData (const QModelIndex &index, const QVariant &value, int role)
    {
        if (index.isValid() && index.column() == 1 && role == Qt::EditRole)
        {
            m_list[index.row()].second = value.toDouble();
            emit updateHeader();
            return true;
        }

        return false;
    }

public slots:
    void addNew(const int &id)
    {
        beginInsertRows(QModelIndex(), m_list.count(), m_list.count());
        double total = totalPercentage();
        m_list.append(qMakePair(id, total >= 100 ? 0 : 100 - total));
        endInsertRows();
        emit updateHeader();
    }

    void deleteSelected()
    {
        QList<int> indexes;
        foreach(const QModelIndex &q, m_parent->selectionModel()->selectedRows())
            indexes.append(q.row());
        qSort(indexes);

        if(indexes.count() == 0)
            return;

        for(int i = indexes.count() - 1; i >= 0; --i)
        {
            beginRemoveRows(QModelIndex(), i, i);
            m_list.removeAt(indexes.at(i));
            endRemoveRows();
        }
        emit updateHeader();
    }

signals:
    void updateHeader();

private:
    const QMap<int, globals::assetAllocation> &m_aaValues;
    QTableView *m_parent;
    int m_columns;
    QList<globals::tickerAATarget> m_list;
};

class tickerTradeModel : public mpiModelBase<globals::dynamicTrade, frmTrade>
{
    Q_OBJECT

public:
    tickerTradeModel(const QList<globals::dynamicTrade> &values, const QMap<int, globals::security> &cashAccounts, const int &cols, QTableView *parent = 0, QDialog *dialog = 0):
            mpiModelBase<globals::dynamicTrade, frmTrade>(values, cols, parent, dialog), m_cashAccounts(cashAccounts) { }

private:
    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_list.size())
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            globals::dynamicTrade trade = m_list.at(index.row());
            switch(index.column())
            {
                case 0:
                    return globals::dynamicTrade::tradeTypeToString(trade.tradeType);
                    break;
                case 1:
                    return globals::dynamicTrade::valueToString(trade.tradeType, trade.value);
                    break;
                case 2:
                    return trade.price < 0 ? "Prev Close" : functions::doubleToCurrency(trade.price);
                    break;
                case 3:
                    return trade.commission < 0 ? "" : functions::doubleToCurrency(trade.commission);
                    break;
                case 4:
                    return m_cashAccounts.contains(trade.cashAccount) ? m_cashAccounts.value(trade.cashAccount).ticker : "";
                    break;
                case 5:
                    return globals::dynamicTrade::frequencyToString(trade.frequency);
                    break;
                case 6:
                    return globals::dynamicTrade::dateToString(trade.frequency, trade.date);
                    break;
                case 7:
                    return trade.startDate != 0 ? QDate::fromJulianDay(trade.startDate).toString(Qt::SystemLocaleShortDate) : "";
                    break;
                case 8:
                    return trade.endDate != 0 ? QDate::fromJulianDay(trade.endDate).toString(Qt::SystemLocaleShortDate) : "";
                    break;
            }
        }

        return QVariant();
    }

    QVariant headerData (int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
            return QVariant();

        switch(section)
        {
            case 0:
                return "Type";
                break;
            case 1:
                return "Value";
                break;
            case 2:
                return "Price";
                break;
            case 3:
                return "Comm.";
                break;
            case 4:
                return "$ Acct";
                break;
            case 5:
                return "Freq.";
                break;
            case 6:
                return "Date";
                break;
            case 7:
                return "Start";
                break;
            case 8:
                return "End";
                break;
        }

        return QVariant();
    }

    QString internalCopy(const globals::dynamicTrade &item)
    {
        return QString("%1\t%2\t%3\t%4\t%5\t%6\t%7\t%8\t%9").arg(
               QString::number((int)item.tradeType), functions::doubleToLocalFormat(item.value), functions::doubleToLocalFormat(item.price),
               functions::doubleToLocalFormat(item.commission), QString::number(item.cashAccount),
               QString::number((int)item.frequency), QString::number(item.date),
               QString::number(item.startDate), QString::number(item.endDate));
    }

    globals::dynamicTrade internalPaste(const QStringList &value, bool *ok)
    {
        globals::dynamicTrade item;

        if (value.count() != 9)
        {
            (*ok) = false;
            return item;
        }

        int i = value.at(0).toInt(ok);
        if (*ok && i < globals::tradeType_Count)
            item.tradeType = (globals::dynamicTradeType)i;

        if (*ok)
            item.value = value.at(1).toDouble(ok);

        if (*ok)
            item.price = value.at(2).toDouble(ok);

        if (*ok)
            item.commission = value.at(3).toDouble(ok);

        if (*ok)
            item.cashAccount = value.at(4).toInt(ok);

        if (*ok)
        {
            int i = value.at(5).toInt(ok);
            if (*ok && i < globals::tradeFreq_Count)
                item.frequency = (globals::dynamicTradeFreq)i;
        }

        if (*ok)
            item.date = value.at(6).toInt(ok);

        if (*ok)
            item.startDate = value.at(7).toInt(ok);

        if (*ok)
            item.endDate = value.at(8).toInt(ok);

        return item;
    }

public slots:
    inline void addNew() { addItem(); autoResize(); }
    inline void editSelected() { editItems(); autoResize(); }
    inline void deleteSelected() { removeItems(); autoResize(); }
    inline void copy() { beginCopy(); }
    inline void paste() { beginPaste(); }

signals:
    void saveItem(globals::dynamicTrade *trade);
    void deleteItem(const globals::dynamicTrade& trade);

private:
    const QMap<int, globals::security> &m_cashAccounts;

    void autoResize()
    {
        if (m_list.count() == 0)
            m_parent->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
        else
            m_parent->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    }
};

#endif // FRMTICKER_H