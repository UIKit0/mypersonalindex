#include "securityHistoryModel.h"

int securityHistoryModel::rowCount(const QModelIndex&) const
{
    switch (m_choice)
    {
        case historyChoice_All:
        case historyChoice_Change:
            return m_history.prices.count();
        case historyChoice_Dividends:
            return m_history.dividends.count();
        case historyChoice_Splits:
            return m_history.splits.count();
        case historyChoice_Trades:
            return m_trades.count();
    }
    return 0;
}

int securityHistoryModel::columnCount (const QModelIndex&) const
{
    switch (m_choice)
    {
        case historyChoice_All:
            return 5;
        case historyChoice_Change:
        case historyChoice_Dividends:
        case historyChoice_Splits:
            return 2;
        case historyChoice_Trades:
            return 5;
    }
    return 0;
}

QVariant securityHistoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || role != Qt::DisplayRole)
        return QVariant();

    int row = index.row();
    int column = index.column();

    switch (m_choice)
    {
        case historyChoice_All:
        { 
            if (m_descending)
                row = m_history.prices.count() - row - 1;

            QMap<int, double>::const_iterator i = m_history.prices.constBegin() + row;

            if (column == 0)
                return QDate::fromJulianDay(i.key()).toString(Qt::SystemLocaleShortDate);
            else if (column == 1)
                return functions::doubleToLocalFormat(*i);
            else if (column == 2)
            {
                if (row == 0)
                    return QVariant();
                else
                    return functions::doubleToPercentage((i.value() * m_history.split(i.key())) / (i-1).value() - 1);
            }
            else if (column == 3)
            {
                if(m_history.dividends.contains(i.key()))
                    return functions::doubleToLocalFormat(m_history.dividend(i.key()));
                else
                    return QVariant();
            }
            else if (column == 4)
            {
                if(m_history.splits.contains(i.key()))
                    return functions::doubleToLocalFormat(m_history.split(i.key()));
                else
                    return QVariant();
            }
            break;
        }
        case historyChoice_Change:
        {
            if (m_descending)
                row = m_history.prices.count() - row - 1;

            QMap<int, double>::const_iterator i = m_history.prices.constBegin() + row;

            if (column == 0)
                return QDate::fromJulianDay(i.key()).toString(Qt::SystemLocaleShortDate);
            else if (column == 1)
            {
                if (row == 0)
                    return QVariant();
                else
                    return functions::doubleToPercentage((i.value() * m_history.split(i.key())) / (i-1).value() - 1);
            }
            break;
        }
        case historyChoice_Dividends:
        {
            if (m_descending)
                row = m_history.dividends.count() - row - 1;
            QMap<int, double>::const_iterator i = m_history.dividends.constBegin() + row;

            if (column == 0)
                return QDate::fromJulianDay(i.key()).toString(Qt::SystemLocaleShortDate);
            else if (column == 1)
                return functions::doubleToLocalFormat(i.value());

            break;
        }
        case historyChoice_Splits:
        {
            if (m_descending)
                row = m_history.splits.count() - row - 1;
            QMap<int, double>::const_iterator i = m_history.splits.constBegin() + row;

            if (column == 0)
                return QDate::fromJulianDay(i.key()).toString(Qt::SystemLocaleShortDate);
            else if (column == 1)
                return functions::doubleToLocalFormat(i.value());

            break;
        }
        case historyChoice_Trades:
        {
            if (m_descending)
                row = m_trades.count() - row - 1;

            if (column == 0)
                return QDate::fromJulianDay(m_trades.at(row).date).toString(Qt::SystemLocaleShortDate);
            else if (column == 1)
                return functions::doubleToLocalFormat(m_trades.at(row).price);
            else if (column == 2)
                return functions::doubleToLocalFormat(m_trades.at(row).shares, 4);
            else if (column == 3)
                return functions::doubleToCurrency(m_trades.at(row).shares * m_trades.at(row).price);
            else if (column == 4)
            {
                if (m_trades.at(row).commission > 0)
                    return functions::doubleToLocalFormat(m_trades.at(row).commission);
                else
                    return QVariant();
            }
            break;
        }
    }

    return QVariant();
}

QVariant securityHistoryModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    if (section == 0)
        return "Date";

    switch (m_choice)
    {
        case historyChoice_All:
            if (section == 1)
                return "Price";
            else if (section == 2)
                return "Change";
            else if (section == 3)
                return "Dividend";
            else if (section == 4)
                return "Split";
            break;
        case historyChoice_Change:
            if (section == 1)
                return "Change";
            break;
        case historyChoice_Dividends:
            if (section == 1)
                return "Dividend";
            break;
        case historyChoice_Splits:
            if (section == 1)
                return "Split";
            break;
        case historyChoice_Trades:
            if (section == 1)
                return "Price";
            else if (section == 2)
                return "Shares";
            else if (section == 3)
                return "Value";
            else if (section == 4)
                return "Comm.";
            break;
    }
    return QVariant();
}