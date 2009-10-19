#include "frmAA.h"
#include "frmAAEdit.h"

frmAA::frmAA(const int &portfolioID, QWidget *parent, queries *sql, const QMap<int, globals::assetAllocation> &aa):
    QDialog(parent), m_sql(sql), m_map(aa), m_portfolio(portfolioID)
{
    if(!m_sql || !m_sql->isOpen())
    {
        reject();
        return;
    }

    ui.setupUI(this, "Asset Allocation", false);
    this->setWindowTitle("Edit Asset Allocation");

    m_model = new aaModel(m_map.values(), 2, ui.table, this);
    ui.table->setModel(m_model);

    connectSlots();
}

void frmAA::connectSlots()
{
    connect(ui.btnAdd, SIGNAL(clicked()), m_model, SLOT(addNew()));
    connect(ui.btnEdit, SIGNAL(clicked()), m_model, SLOT(editSelected()));
    connect(ui.table, SIGNAL(doubleClicked(QModelIndex)), m_model, SLOT(editSelected()));
    connect(ui.btnDelete, SIGNAL(clicked()), m_model, SLOT(deleteSelected()));
    connect(ui.btnOkCancel, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.btnOkCancel, SIGNAL(rejected()), this, SLOT(reject()));
    connect(m_model, SIGNAL(saveItem(globals::assetAllocation*)), this, SLOT(saveItem(globals::assetAllocation*)));
    connect(m_model, SIGNAL(deleteItem(globals::assetAllocation)), this, SLOT(deleteItem(globals::assetAllocation)));
}

void frmAA::accept()
{
    QMap<int, globals::assetAllocation> returnValues = m_model->saveList(m_map);

    if (returnValues != m_map)
    {
        m_map = returnValues;
        QDialog::accept();
    }
    else
        QDialog::reject();
}

void frmAA::saveItem(globals::assetAllocation *aa)
{
    m_sql->executeNonQuery(m_sql->updateAA(m_portfolio, (*aa)));
    if (aa->id == -1)
        aa->id = m_sql->executeScalar(m_sql->getIdentity()).toInt();
}

void frmAA::deleteItem(const globals::assetAllocation &aa)
{
    m_sql->executeNonQuery(m_sql->deleteItem(queries::table_AA, aa.id));
}