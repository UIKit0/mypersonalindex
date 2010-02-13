#include "frmTrade_UI.h"
#include "functions.h"
#include <QtGui>

void frmTrade_UI::setupUI(QDialog *dialog)
{
    dialog->setWindowFlags(dialog->windowFlags() & (~Qt::WindowContextHelpButtonHint));

    mainLayout = new QGridLayout(dialog);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);

    gpTrade = new QGroupBox("Trade", dialog);
    tradeLayout = new QFormLayout(gpTrade);

    validator = new mpiDoubleValidator(-100000000, 100000000, 4, dialog);
    type = new QLabel("&Type:", gpTrade);
    cmbType = new QComboBox(gpTrade);
    cmbType->addItem("Purchase");
    cmbType->addItem("Sale");
    cmbType->addItem("Fixed Purchase");
    cmbType->addItem("Fixed Sale");
    cmbType->addItem("Reinvestment");
    cmbType->addItem("Interest");
    cmbType->addItem("% of Portfolio");
    cmbType->addItem("% of AA Target");
    tradeLayout->setWidget(0, QFormLayout::LabelRole, type);
    tradeLayout->setWidget(0, QFormLayout::FieldRole, cmbType);
    shares = new QLabel("&Shares:", gpTrade);
    txtShares = new QLineEdit(gpTrade);
    txtShares->setValidator(validator);
    tradeLayout->setWidget(1, QFormLayout::LabelRole, shares);
    tradeLayout->setWidget(1, QFormLayout::FieldRole, txtShares);
    chkPrice = new QCheckBox("&Price:", gpTrade);
    txtPrice = new QLineEdit("Previous Close", gpTrade);
    txtPrice->setEnabled(false);  
    txtPrice->setValidator(validator);
    tradeLayout->setWidget(2, QFormLayout::LabelRole, chkPrice);
    tradeLayout->setWidget(2, QFormLayout::FieldRole, txtPrice);
    commission = new QLabel("&Commission:", gpTrade);
    txtCommission = new QLineEdit(gpTrade);
    txtCommission->setValidator(validator);
    tradeLayout->setWidget(3, QFormLayout::LabelRole, commission);
    tradeLayout->setWidget(3, QFormLayout::FieldRole, txtCommission);
    cash = new QLabel("T&o/From Cash:", gpTrade);
    cmbCash = new QComboBox(gpTrade);
    tradeLayout->setWidget(4, QFormLayout::LabelRole, cash);
    tradeLayout->setWidget(4, QFormLayout::FieldRole, cmbCash);

    gpDate = new QGroupBox("Date", dialog);
    dateLayout = new QFormLayout(gpDate);

    freq = new QLabel("&Frequency:", gpDate);
    cmbFreq = new QComboBox(gpDate);
    cmbFreq->addItem("Once");
    cmbFreq->addItem("Daily");
    cmbFreq->addItem("Weekly");
    cmbFreq->addItem("Monthly");
    cmbFreq->addItem("Yearly");
    dateLayout->setWidget(0, QFormLayout::LabelRole, freq);
    dateLayout->setWidget(0, QFormLayout::FieldRole, cmbFreq);
    date = new QLabel("&Date:", gpDate);
    deDate = functions::createDateEdit(gpDate);
    dateLayout->setWidget(1, QFormLayout::LabelRole, date);
    dateLayout->setWidget(1, QFormLayout::FieldRole, deDate);
    chkStarting = new QCheckBox("St&arting:", gpDate);
    deStarting = functions::createDateEdit(gpDate);
    deStarting->setEnabled(false);
    dateLayout->setWidget(2, QFormLayout::LabelRole, chkStarting);
    dateLayout->setWidget(2, QFormLayout::FieldRole, deStarting);
    chkEnding = new QCheckBox("&Ending:", gpDate);
    deEnding = functions::createDateEdit(gpDate);
    deEnding->setEnabled(false);
    dateLayout->setWidget(3, QFormLayout::LabelRole, chkEnding);
    dateLayout->setWidget(3, QFormLayout::FieldRole, deEnding);

    btnOkCancel = new QDialogButtonBox(dialog);
    btnOkCancel->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

    mainLayout->addWidget(gpTrade, 0, 0);
    mainLayout->addWidget(gpDate, 0, 1);
    mainLayout->addWidget(btnOkCancel, 1, 0, 1, 2);

    type->setBuddy(cmbType);
    shares->setBuddy(txtShares);
    commission->setBuddy(txtCommission);
    cash->setBuddy(cmbCash);
    freq->setBuddy(cmbFreq);
    date->setBuddy(deDate);
}