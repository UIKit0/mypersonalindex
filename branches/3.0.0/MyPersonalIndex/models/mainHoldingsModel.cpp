#include "mainHoldingsModel.h"

//enum { row_Active, row_Symbol, row_Cash, row_Price, row_Shares, row_Avg, row_Cost, row_Value, row_ValueP, row_Gain, row_GainP, row_Acct, row_TaxLiability, row_NetValue, row_ID };
const QStringList holdingsRow::columns = QStringList() << "Active" << "Symbol" << "Cash" << "Closing Price" << "Shares" << "Avg Price\nPer Share"
     << "Cost Basis" << "Total Value" << "% of\nPortfolio" << "Gain/Loss" << "% Gain/Loss" << "Account" << "Asset Allocation" << "Tax Liability" << "After Tax Value" << "ID";

const QVariantList holdingsRow::columnsType = QVariantList() << QVariant(QVariant::Int) << QVariant(QVariant::String) << QVariant(QVariant::Int)
     << QVariant(QVariant::Double) << QVariant(QVariant::Double) << QVariant(QVariant::Double) << QVariant(QVariant::Double) << QVariant(QVariant::Double)
     << QVariant(QVariant::Double) << QVariant(QVariant::Double) << QVariant(QVariant::Double) << QVariant(QVariant::String) << QVariant(QVariant::String)
     << QVariant(QVariant::Double) << QVariant(QVariant::Double);

holdingsRow::holdingsRow(const security &s, const dailyInfoPortfolio *info, const QMap<int, account> &accounts, const QMap<int, assetAllocation> &aa, const QString &sort)
    : baseRow(sort)
{
    securityInfo value = info->securitiesInfo.value(s.id);
    double costBasis = info->avgPrices.value(s.id) * value.shares;
    //row_Active
    this->values.append((int)s.includeInCalc);
    //row_Symbol
    this->values.append(s.symbol);
    //row_Cash
    this->values.append((int)s.cashAccount);
    //row_Price
    double price = prices::instance().price(s.symbol, info->date);
    this->values.append(price == 0 ? QVariant() : price);
    //row_Shares
    this->values.append(value.shares);
    //row_Avg
    this->values.append(value.shares == 0 ? QVariant() : info->avgPrices.value(s.id));
    //row_Cost
    this->values.append(value.shares == 0 ? QVariant() : costBasis);
    //row_Value
    this->values.append(value.shares == 0 ? QVariant() : value.totalValue);
    //row_ValueP
    this->values.append(info->totalValue == 0 ? QVariant() : value.totalValue / info->totalValue);
    //row_Gain
    this->values.append(value.shares == 0 ? QVariant() : value.totalValue - costBasis);
    //row_GainP
    this->values.append(value.shares == 0 || costBasis == 0 ? QVariant() : (value.totalValue / costBasis) - 1);
    //row_Acct
    this->values.append(s.account == -1 ? QVariant() : accounts.value(s.account).description);
    //row_AA
    QStringList aaList;
    for(QMap<int, double>::const_iterator i = s.aa.constBegin(); i != s.aa.constEnd(); ++i)
        aaList.append(QString("%1 - %2").arg(aa.value(i.key()).description, functions::doubleToPercentage(i.value())));
    this->values.append(aaList.join(", "));
    //row_TaxLiability
    this->values.append(value.taxLiability == 0 ? QVariant() : value.taxLiability);
    //row_NetValue
    this->values.append(value.shares == 0 ? QVariant() : value.totalValue - value.taxLiability);
    //row_ID
    this->values.append(s.id);
}

QMap<int, QString> holdingsRow::fieldNames()
{
    QMap<int, QString> names;

    for (int i = 0; i < columns.count(); ++i)
        names[i] = QString(columns.at(i)).replace('\n', ' ');

    names.remove(row_ID);
    return names;
}

QVariant mainHoldingsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    int column = m_viewableColumns.at(index.column());
    QVariant value = m_rows.at(index.row())->values.at(column);

    if (role == Qt::DisplayRole)
    {
        if (value.isNull() || column == holdingsRow::row_Active || column == holdingsRow::row_Cash)
            return QVariant();

        switch (column)
        {
            case holdingsRow::row_Avg:
            case holdingsRow::row_Cost:
            case holdingsRow::row_Gain:
            case holdingsRow::row_Price:
            case holdingsRow::row_Value:
            case holdingsRow::row_TaxLiability:
            case holdingsRow::row_NetValue:
                return functions::doubleToCurrency(value.toDouble());
            case holdingsRow::row_GainP:
            case holdingsRow::row_ValueP:
                return functions::doubleToPercentage(value.toDouble());
            case holdingsRow::row_Shares:
                return functions::doubleToLocalFormat(value.toDouble(), 4);
        }

        return value;
    }

    if (role == Qt::CheckStateRole && (column == holdingsRow::row_Active || column == holdingsRow::row_Cash))
    {
        return value.toInt() == 1 ? Qt::Checked : Qt::Unchecked;
    }

    if (role == Qt::TextColorRole && column == holdingsRow::row_GainP)
        return value.toDouble() == 0 ? QVariant() :
            value.toDouble() > 0 ?  qVariantFromValue(QColor(Qt::darkGreen)) : qVariantFromValue(QColor(Qt::red));

    return QVariant();
}

QVariant mainHoldingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
     if (section >= m_viewableColumns.count())
        return QVariant();

    if (role == Qt::TextAlignmentRole)
        return (int)Qt::AlignLeft | Qt::AlignVCenter;

    if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
        return QVariant();

    int column = m_viewableColumns.at(section);
    QString extra;
    switch(column)
    {
        case holdingsRow::row_Cost:
            extra = QString("\n[%1]").arg(functions::doubleToCurrency(m_costBasis));
            break;
        case holdingsRow::row_Value:
            extra = QString("\n[%1]").arg(functions::doubleToCurrency(m_totalValue));
            break;
        case holdingsRow::row_Gain:
            extra = QString("\n[%1]").arg(functions::doubleToCurrency(m_totalValue - m_costBasis));
            break;
        case holdingsRow::row_GainP:
            extra = QString("\n[%1]").arg(functions::doubleToPercentage((m_totalValue - m_costBasis) / m_costBasis));
            break;
        case holdingsRow::row_TaxLiability:
            extra = QString("\n[%1]").arg(functions::doubleToCurrency(m_taxLiability));
            break;
        case holdingsRow::row_NetValue:
            extra = QString("\n[%1]").arg(functions::doubleToCurrency(m_totalValue - m_taxLiability));
            break;
    }

    return QString(holdingsRow::columns.at(column)).append(extra);
}
