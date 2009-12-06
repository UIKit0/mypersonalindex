#ifndef ACCTMODEL_H
#define ACCTMODEL_H

#include "globals.h"
#include "functions.h"
#include "mpiEditModelBase.h"
#include "frmAcctEdit.h"


class acctModel : public mpiEditModelBase<globals::account, frmAcctEdit>
{
    Q_OBJECT

public:
    acctModel(const QList<globals::account> &values, const int &cols, QTableView *parent = 0, QDialog *dialog = 0):
            mpiEditModelBase<globals::account, frmAcctEdit>(values, cols, parent, dialog) { }

private:
    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        if (index.row() >= m_list.size())
            return QVariant();

        if (role == Qt::DisplayRole)
        {
            if (index.column() == 0)
            {
                return m_list.at(index.row()).description;
            }
            if (index.column() == 1)
            {
                return m_list.at(index.row()).taxRate < 0 ? "None" :
                    functions::doubleToPercentage(m_list.at(index.row()).taxRate);
            }
            if (index.column() == 2)
            {
                return m_list.at(index.row()).taxDeferred  ? "Yes" : "No";
            }
        }

        return QVariant();
    }

    QVariant headerData (int section, Qt::Orientation orientation, int role) const
    {
        if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
            return QVariant();

        if (section == 0)
            return "Description";

        if (section == 1)
            return "Tax Rate";

        if (section == 2)
            return "Tax Deferred";

        return QVariant();
    }

    QString internalCopy(const globals::account &item)
    {
        return QString("%1\t%2\t%3").arg(item.description, item.taxRate < 0 ? "None" : functions::doubleToPercentage(item.taxRate), QString(item.taxDeferred ? "Yes": "No"));
    }

    globals::account internalPaste(const QStringList &value, bool *ok)
    {
        globals::account item;

        if (value.count() != 3)
        {
            (*ok) = false;
            return item;
        }

        item.description = value.at(0);
        item.taxRate = functions::stringToDouble(value.at(1), ok);

        QString taxDef = value.at(2).toUpper();
        if (taxDef == "YES" || taxDef == "NO")
            item.taxDeferred = taxDef == "YES";
        else
            (*ok) = false;

        return item;
    }

public slots:
    inline void addNew() { addItem(); }
    inline void editSelected() { editItems(); }
    inline void deleteSelected() { removeItems(); }
    inline void copy() { beginCopy(); }
    inline void paste() { beginPaste(); }

signals:
    void saveItem(globals::account *acct);
    void deleteItem(const globals::account& acct);
};

#endif // ACCTMODEL_H