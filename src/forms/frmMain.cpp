#include "frmMain.h"
#include "frmMain_UI.h"
#include <QCoreApplication>
#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileInfo>
#include "frmEdit.h"
#include "settingsFactory.h"
#include "updatePrices.h"
#include "tradeDateCalendar.h"
#include "calculatorTrade.h"
#include "security.h"
#include "frmMainTableView_UI.h"
#include "mainAAModel.h"
#include "mainSecurityModel.h"
#include "historicalNAV.h"
#include "frmSort.h"
#include "frmColumns.h"
#include "mpiFile_State.h"

#ifdef CLOCKTIME
#include <QTime>
#endif

frmMain::frmMain(QWidget *parent_):
    QMainWindow(parent_),
    ui(new frmMain_UI()),
    ui_assetAllocation(new frmMainTableView_UI()),
    ui_security(new frmMainTableView_UI()),
    m_file(new mpiFile_State(this)),
    m_currentPortfolio(0),
    m_futureWatcherYahoo(0),
    m_futureWatcherTrade(0)
{
    ui->setupUI(this);
    connectSlots();
    loadSettings();
    m_file->newFile();
}

frmMain::~frmMain()
{
    if (m_futureWatcherYahoo)
    {
        m_futureWatcherYahoo->cancel();
        m_futureWatcherYahoo->waitForFinished();
    }

    if (m_futureWatcherTrade)
    {
        m_futureWatcherTrade->cancel();
        m_futureWatcherTrade->waitForFinished();
    }

    delete ui;
    delete ui_assetAllocation;
    delete m_file;
}

void frmMain::connectSlots()
{
    connect(ui->helpAbout, SIGNAL(triggered()), this, SLOT(about()));
    connect(ui->fileOpen, SIGNAL(triggered()), m_file, SLOT(open()));
    connect(ui->fileSave, SIGNAL(triggered()), m_file, SLOT(save()));
    connect(ui->fileSaveAs, SIGNAL(triggered()), m_file, SLOT(saveAs()));
    connect(ui->fileNew, SIGNAL(triggered()), m_file, SLOT(newFile()));
    connect(ui->fileExit, SIGNAL(triggered()), this, SLOT(close()));
    connect(m_file, SIGNAL(fileNameChange(QString,bool)), this, SLOT(fileChange(QString,bool)));
    connect(ui->portfolioAdd, SIGNAL(triggered()), this, SLOT(addPortfolio()));
    connect(ui->portfolioEdit, SIGNAL(triggered()), this, SLOT(editPortfolio()));
    connect(ui->portfolioDelete, SIGNAL(triggered()), this, SLOT(deletePortfolio()));
    connect(ui->portfolioDropDownCmb, SIGNAL(currentIndexChanged(int)), this, SLOT(portfolioDropDownChange(int)));
    connect(ui->importYahoo, SIGNAL(triggered()), this, SLOT(importYahoo()));
    connect(ui->viewAssetAllocation, SIGNAL(triggered()), this, SLOT(tabAA()));
    connect(ui->viewSecurities, SIGNAL(triggered()), this, SLOT(tabSecurity()));
}

void frmMain::loadSettings()
{
    m_settings = settingsFactory().getSettings();
    if (m_settings.windowState != Qt::WindowActive)
    {
        resize(m_settings.windowSize);
        move(m_settings.windowLocation);
        if (m_settings.windowState != Qt::WindowNoState)
            this->setWindowState(this->windowState() | m_settings.windowState);
    }
}

void frmMain::fileChange(const QString &filePath_, bool newFile_)
{
    m_currentPortfolio = 0; // invalidated
    setWindowModified(false);
    if (filePath_.isEmpty())
        setWindowTitle(QString("untitled.mpi[*] - %1").arg(QCoreApplication::applicationName()));
    else
        setWindowTitle(QString("%1[*] - %2").arg(QFileInfo(filePath_).fileName(), QCoreApplication::applicationName()));

    updateRecentFileActions(filePath_);

    // track current portfolio
    int currentID = ui->portfolioDropDownCmb->currentIndex() == -1 || newFile_ ?
        UNASSIGNED :
        m_file->portfolioIdentities.value(ui->portfolioDropDownCmb->itemData(ui->portfolioDropDownCmb->currentIndex()).toInt(), UNASSIGNED);

    if (newFile_)
        refreshPortfolioPrices();

    refreshPortfolioCmb(currentID);
}

void frmMain::setCurrentPortfolio(portfolio *portfolio_)
{
    m_currentPortfolio = portfolio_;
    if (portfolio_)
        m_currentCalculator.setPortfolio(*portfolio_);
}

void frmMain::refreshPortfolioCmb(int id_)
{
    ui->portfolioDropDownCmb->blockSignals(true);
    ui->portfolioDropDownCmb->clear();
    foreach(const portfolio &p, m_file->portfolios)
    {
        if (p.deleted())
            continue;

        ui->portfolioDropDownCmb->addItem(p.description(), p.id());
    }

    int index = ui->portfolioDropDownCmb->findData(id_);
    if (index == -1 && ui->portfolioDropDownCmb->count() != 0)
        index = 0;

    if (index != -1)
        ui->portfolioDropDownCmb->setCurrentIndex(index);

    portfolioDropDownChange(ui->portfolioDropDownCmb->currentIndex());
    ui->portfolioDropDownCmb->blockSignals(false);
}

void frmMain::closeEvent(QCloseEvent *event_)
{
    if (!m_file->maybeSave())
    {
        event_->ignore();
        return;
    }

    event_->accept();
    saveSettings();
}

void frmMain::saveSettings()
{
    m_settings.windowSize = size();
    m_settings.windowLocation = pos();
    m_settings.windowState = isMaximized() ? Qt::WindowMaximized : isMinimized() ? Qt::WindowMinimized : Qt::WindowNoState;
    m_settings.save();
}

void frmMain::updateRecentFileActions(const QString &newFilePath_)
{
    if (!newFilePath_.isEmpty())
        m_settings.addRecentFile(newFilePath_);

    ui->fileRecent->clear();
    foreach(const QString &s, m_settings.recentFiles())     
        connect(ui->fileRecent->addAction(s), SIGNAL(triggered()), this, SLOT(recentFileSelected()));

    int counter = 1;
    foreach(QAction *action, ui->fileRecent->actions())
        action->setShortcut(QString("Ctrl+%1").arg(QString::number(counter++)));
}

void frmMain::recentFileSelected()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (!action || action->text() == m_file->path())
        return;

    m_file->open(action->text());
}

void frmMain::addPortfolio()
{
    portfolio p(portfolio::getOpenIdentity());
    frmEdit f(p, this);
    f.exec();
    if (p == f.getPortfolio())
        return;

    setWindowModified(true);
    m_file->modified = true;
    portfolio newPortfolio = f.getPortfolio();
    m_file->portfolios.insert(newPortfolio.id(), newPortfolio);
    refreshPortfolioPrices();
    ui->portfolioDropDownCmb->addItem(newPortfolio.description(), newPortfolio.id());
    ui->portfolioDropDownCmb->setCurrentIndex(ui->portfolioDropDownCmb->count() - 1);
    recalculateTrades(*m_currentPortfolio);
}

void frmMain::editPortfolio()
{
    if (!m_currentPortfolio)
        return;

    frmEdit f(*m_currentPortfolio, this);
    f.exec();
    if (*m_currentPortfolio == f.getPortfolio())
        return;

    setWindowModified(true);
    m_file->modified = true;
    m_file->portfolios[m_currentPortfolio->id()] = f.getPortfolio();
    refreshPortfolioPrices();

    setCurrentPortfolio(&m_file->portfolios[m_currentPortfolio->id()]);
    ui->portfolioDropDownCmb->setItemText(ui->portfolioDropDownCmb->currentIndex(), m_currentPortfolio->description());
    recalculateTrades(*m_currentPortfolio, 0);
}

void frmMain::deletePortfolio()
{
    if (!m_currentPortfolio)
        return;

    if (QMessageBox::question(this, QCoreApplication::applicationName(), QString("Are you sure you want to delete portfolio %1?")
        .arg(m_currentPortfolio->description()), QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes)
        return;

    setWindowModified(true);
    m_file->modified = true;
    m_currentPortfolio->setDeleted(true);
    ui->portfolioDropDownCmb->removeItem(ui->portfolioDropDownCmb->currentIndex());
}

void frmMain::refreshPortfolioPrices()
{
    foreach(portfolio p, m_file->portfolios)
    {
        if (p.deleted())
            continue;

        foreach(security s, p.securities())
        {
            if (s.deleted())
                continue;

            s.setHistoricalPrices(m_file->prices.getHistoricalPrice(s.description()));
        }
    }
}

void frmMain::portfolioDropDownChange(int currentIndex_)
{
    ui->portfolioDelete->setDisabled(currentIndex_ == -1);
    ui->portfolioEdit->setDisabled(currentIndex_ == -1);
    ui->portfolioDropDownCmb->setDisabled(ui->portfolioDropDownCmb->count() == 0);
    setCurrentPortfolio(currentIndex_ == -1 ? 0 : &m_file->portfolios[ui->portfolioDropDownCmb->itemData(currentIndex_).toInt()]);
}

void frmMain::about()
{
    QMessageBox::about(this, "About My Personal Index", "<h2>My Personal Index " + QString::number(APP_VERSION / 100.0) + "</h2>"
        "<p>Copyright &copy; 2010"
        "<p>By Matthew Wikler"
        "<p>Create personal indexes and perform analysis to make better investing decisions."
        "<br><a href='http://code.google.com/p/mypersonalindex/'>http://code.google.com/p/mypersonalindex/</a></p>");
}

void frmMain::importYahoo()
{
    if (!updatePrices::isInternetConnection())
    {
        QMessageBox::critical(this, QCoreApplication::applicationName(), "Cannot contact Yahoo! Finance, please check your internet connection.");
        return;
    }

    int beginDate = tradeDateCalendar::endDate() + 1;
    foreach(const portfolio &p, m_file->portfolios)
    {
        if (p.deleted())
            continue;

        beginDate = qMin(beginDate, p.startDate());
    }

    QList<historicalPrices> prices;
    foreach(const QString &s, portfolio::symbols(m_file->portfolios))
        prices.append(m_file->prices.getHistoricalPrice(s));

    showProgressBar("Downloading", prices.count());
    updatePricesOptions options(beginDate, tradeDateCalendar::endDate(), m_settings.splits);

    m_futureWatcherYahoo = new QFutureWatcher<int>();
    connect(m_futureWatcherYahoo, SIGNAL(finished()), this, SLOT(importYahooFinished()));
    connect(m_futureWatcherYahoo, SIGNAL(progressValueChanged(int)), ui->progressBar, SLOT(setValue(int)));
    m_futureWatcherYahoo->setFuture(QtConcurrent::mapped(prices, updatePrices(options)));
}

void frmMain::importYahooFinished()
{
    hideProgressBar();

    int earliestUpdate = tradeDateCalendar::endDate() + 1;
    foreach(const int &result, m_futureWatcherYahoo->future())
        if (result != UNASSIGNED)
            earliestUpdate = qMin(earliestUpdate, result);

    delete m_futureWatcherYahoo;
    m_futureWatcherYahoo = 0;

    if (earliestUpdate > tradeDateCalendar::endDate())
        return;

    setWindowModified(true);
    m_file->modified = true;
    recalculateTrades(m_file->portfolios.values(), earliestUpdate);
}

void frmMain::recalculateTrades(const portfolio &portfolio_, int beginDate_)
{
    recalculateTrades(QList<portfolio>() << portfolio_, beginDate_);
}

void frmMain::recalculateTrades(const QList<portfolio> &portfolios_, int beginDate_)
{
    showProgressBar("Calculating", portfolios_.count());
    m_futureWatcherTrade = new QFutureWatcher<void>();
    connect(m_futureWatcherTrade, SIGNAL(finished()), this, SLOT(recalculateTradesFinished()));
    m_futureWatcherTrade->setFuture(QtConcurrent::mapped(portfolios_, calculatorTrade(beginDate_)));
}

void frmMain::recalculateTradesFinished()
{
    delete m_futureWatcherTrade;
    m_futureWatcherTrade = 0;
    hideProgressBar();
}

void frmMain::showProgressBar(const QString &description_, int steps_)
{
    ui->progressBar->setMaximum(steps_);
    ui->progressBar->setValue(0);
    ui->progressBar->setFormat(QString("%1: %p%").arg(description_));
    ui->cornerWidget->setCurrentIndex(1);
}

void frmMain::hideProgressBar()
{
    ui->cornerWidget->setCurrentIndex(0);
}

QWidget* frmMain::setupTable(tab tab_, frmMainTableView_UI *ui_)
{
    ui_->setupUI(tableColumns(tab_), this);
    ui_->toolbarDateBeginEdit->setDate(QDate::fromJulianDay(m_currentPortfolio->startDate()));
    ui_->toolbarDateEndEdit->setDate(QDate::fromJulianDay(m_currentPortfolio->endDate()));
    setSortDropDown(m_settings.viewableColumnsSorting.value(settingsColumn(tab_)), ui_->toolbarSortCmb);
    connect(ui_->toolbarDateBeginEdit, SIGNAL(dateChanged(QDate)), this, SLOT(refreshTab()));
    connect(ui_->toolbarDateEndEdit, SIGNAL(dateChanged(QDate)), this, SLOT(refreshTab()));
    connect(ui_->toolbarSortCmb, SIGNAL(activated(int)), this, SLOT(sortChanged(int)));
    connect(ui_->toolbarReorder, SIGNAL(triggered()), this, SLOT(modifyColumns()));
    connect(ui_->toolbarExport, SIGNAL(triggered()), ui_->table, SLOT(exportTable()));
    connect(ui_->tableCopy, SIGNAL(activated()), ui_->table, SLOT(copyTable()));

    ui->centralWidget->addWidget(ui_->widget);
    return ui_->widget;
}

void frmMain::switchToTab(tab tab_)
{
    if (!m_currentPortfolio || m_currentTab == tab_)
        return;

    if (m_tabs.contains(tab_))
    {
        ui->centralWidget->setCurrentWidget(m_tabs.value(tab_));
        m_currentTab = tab_;
        return;
    }

    switch (tab_)
    {
        case tab_assetAllocation:
            m_tabs.insert(tab_assetAllocation, setupTable(tab_assetAllocation, ui_assetAllocation));
            break;
        case tab_security:
            m_tabs.insert(tab_security, setupTable(tab_assetAllocation, ui_security));
            break;
    }

    ui->centralWidget->setCurrentWidget(m_tabs.value(tab_));
    m_currentTab = tab_;
    refreshTab();
}

QAbstractItemModel* frmMain::createModel(tab tab_, int beginDate_, int endDate_)
{
    snapshot portfolioValue = m_currentCalculator.portfolioSnapshot(endDate_);

    switch(tab_)
    {
        case tab_assetAllocation:
            return new mainAAModel(
                aaRow::getRows(
                    m_currentPortfolio->assetAllocations(),
                    beginDate_,
                    endDate_,
                    m_currentCalculator,
                    portfolioValue,
                    m_settings.viewableColumnsSorting.value(settings::columns_AA)
                ),
                portfolioValue,
                m_currentCalculator.nav(*m_currentPortfolio, beginDate_, endDate_),
                m_settings.viewableColumns.value(settings::columns_AA),
                ui_assetAllocation->table
            );
        case tab_security:
            return new mainSecurityModel(
                securityRow::getRows(
                    m_currentPortfolio->securities(),
                    m_currentPortfolio->assetAllocations(),
                    m_currentPortfolio->accounts(),
                    beginDate_,
                    endDate_,
                    m_currentCalculator,
                    portfolioValue,
                    m_settings.viewableColumnsSorting.value(settings::columns_Security)
                ),
                portfolioValue,
                m_currentCalculator.nav(*m_currentPortfolio, beginDate_, endDate_),
                m_settings.viewableColumns.value(settings::columns_Security),
                ui_security->table
            );
    }

    return 0;
}

QAbstractItemModel* frmMain::getModel(tab tab_)
{
    switch(tab_)
    {
        case tab_assetAllocation:
            return ui_assetAllocation->table->model();
        case tab_security:
            return ui_security->table->model();
    }

    return 0;
}

void frmMain::refreshTab()
{
    if (!m_currentPortfolio)
        return;
#ifdef CLOCKTIME
    QTime t;
    t.start();
#endif

    switch(m_currentTab)
    {
    case tab_assetAllocation:
        ui_assetAllocation->table->setModel(createModel(tab_assetAllocation, ui_assetAllocation->toolbarDateBeginEdit->date().toJulianDay(), ui_assetAllocation->toolbarDateEndEdit->date().toJulianDay()));
        break;
    case tab_security:
        ui_security->table->setModel(createModel(tab_security, ui_security->toolbarDateBeginEdit->date().toJulianDay(), ui_security->toolbarDateEndEdit->date().toJulianDay()));
        break;
    }

#ifdef CLOCKTIME
    qDebug("Time elapsed (refresh): %d ms", t.elapsed());
#endif
}

void frmMain::setSortDropDown(const QList<orderBy> &sort_, QComboBox *dropDown_)
{
    dropDown_->blockSignals(true);

    if (sort_.isEmpty()) // no sort
        dropDown_->setCurrentIndex(0);
    else if (sort_.at(0).direction == orderBy::order_descending || sort_.count() > 1) // custom sort
        dropDown_->setCurrentIndex(dropDown_->count() - 1);
    else
        dropDown_->setCurrentIndex(dropDown_->findData(sort_.at(0).column));

    dropDown_->blockSignals(false);
}

void frmMain::sortChanged(int index_)
{
    int columnID = static_cast<QComboBox*>(sender())->itemData(index_).toInt();
    settings::column col = settingsColumn(m_currentTab);

    switch(columnID)
    {
    case -1:
        m_settings.viewableColumnsSorting[col].clear();
        break;
    case -2:
        {
            frmSort f(m_settings.viewableColumnsSorting.value(col), tableColumns(m_currentTab), this);
            if (f.exec())
                 m_settings.viewableColumnsSorting[col] = f.getReturnValues();
            else
            {
                setSortDropDown(m_settings.viewableColumnsSorting.value(col), static_cast<QComboBox*>(sender()));
                return;
            }
        }
        break;
    default:
        m_settings.viewableColumnsSorting[col].clear();
        m_settings.viewableColumnsSorting[col].append(orderBy(columnID, orderBy::order_ascending));
        break;
    }
    setSortDropDown(m_settings.viewableColumnsSorting.value(col), static_cast<QComboBox*>(sender()));
    static_cast<mpiViewModelBase*>(getModel(m_currentTab))->setColumnSort(m_settings.viewableColumnsSorting.value(col));
}

settings::column frmMain::settingsColumn(tab tab_)
{
    switch(tab_)
    {
        case tab_assetAllocation:
            return settings::columns_AA;
        case tab_security:
            return settings::columns_Security;
    }

    return settings::columns_Security;
}

QMap<int, QString> frmMain::tableColumns(tab tab_)
{
    switch(tab_)
    {
        case tab_assetAllocation:
            return aaRow::fieldNames();
        case tab_security:
            return securityRow::fieldNames();
    }

    return QMap<int, QString>();
}

void frmMain::modifyColumns()
{
    settings::column col = settingsColumn(m_currentTab);
    frmColumns f(m_settings.viewableColumns.value(col), tableColumns(m_currentTab), this);
    if (!f.exec())
        return;

    m_settings.viewableColumns[col] = f.getReturnValues();
    static_cast<mpiViewModelBase*>(getModel(m_currentTab))->setViewableColumns(m_settings.viewableColumns.value(col));
}


