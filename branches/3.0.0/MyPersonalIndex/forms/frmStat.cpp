#include "frmStat.h"
#include "frmStatEdit.h"

frmStat::frmStat(const int &portfolioID, QWidget *parent, queries *sql, const QMap<int, globals::statistic> &stat, QList<int> *statList):
    frmTableViewBase<globals::statistic, frmStatEdit>(portfolioID, parent, sql, stat, true, "Choose Statistics", 1), m_statList(statList)
{
    if(!m_sql || !m_sql->isOpen())
    {
        reject();
        return;
    }

    this->setWindowTitle("Edit Statistics");
    m_model->setHeaderData(0, Qt::Horizontal, "Statistic");
    connectSlots();

    qSort(m_list);
    for(int i = 0; i < statList->count(); ++i)
        m_list.move(m_list.indexOf(m_map.value(statList->value(i))), i);

       // m_list.move(m_list.indexOf(m_map.value(statList->value(i))), i);

    loadItems();

    for(int i = 0; i < m_model->rowCount(); ++i)
    {
        m_model->item(i, 0)->setCheckable(true);
        m_model->item(i, 0)->setCheckState(statList->contains(m_list.value(i).id) ? Qt::Checked : Qt::Unchecked);
    }
}

void frmStat::connectSlots()
{
    connect(ui.btnAdd, SIGNAL(clicked()), this, SLOT(addStat()));
    connect(ui.btnEdit, SIGNAL(clicked()), this, SLOT(editStat()));
    connect(ui.btnDelete, SIGNAL(clicked()), this, SLOT(removeStat()));
    connect(ui.btnOkCancel, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.btnOkCancel, SIGNAL(rejected()), this, SLOT(reject()));
    connect(ui.btnSelectAll, SIGNAL(clicked()), this, SLOT(selectAll()));
    connect(ui.btnClear, SIGNAL(clicked()), this, SLOT(clearAll()));
    connect(ui.btnMoveUp, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(ui.btnMoveDown, SIGNAL(clicked()), this, SLOT(moveDown()));
}

void frmStat::updateList(const globals::statistic &stat, const int &row)
{
    int i = row == -1 ? m_model->rowCount() : row; // -1 is an insert

    Qt::CheckState state = Qt::Unchecked;
    if (row != -1 && m_model->item(row, 0))
        state = m_model->item(row, 0)->checkState();

    QStandardItem *desc = new QStandardItem(stat.description);
    desc->setCheckable(true);
    desc->setCheckState(state);
    m_model->setItem(i, 0, desc);
}

void frmStat::accept()
{
    frmTableViewBase<globals::statistic, frmStatEdit>::accept();
}

void frmStat::saveItem(const globals::statistic &stat)
{
    //m_sql->executeNonQuery(m_sql->updateAA(m_portfolioID, aa));
}

void frmStat::deleteItem(const globals::statistic &stat)
{
    //m_sql->executeNonQuery(m_sql->deleteItem(queries::table_AA, aa.id));
}

void frmStat::addStat()
{
    addItem();
}

void frmStat::editStat()
{
    editItem();
}

void frmStat::removeStat()
{
    removeItem();
}

void frmStat::selectAll()
{
    frmTableViewBase<globals::statistic, frmStatEdit>::selectAll();
}

void frmStat::clearAll()
{
    frmTableViewBase<globals::statistic, frmStatEdit>::clearAll();
}

void frmStat::moveUp()
{
    frmTableViewBase<globals::statistic, frmStatEdit>::moveUp();
}

void frmStat::moveDown()
{
    frmTableViewBase<globals::statistic, frmStatEdit>::moveDown();
}
