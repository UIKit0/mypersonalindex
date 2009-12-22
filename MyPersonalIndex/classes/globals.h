#ifndef GLOBALS_H
#define GLOBALS_H

#include <QtGui>

class globals
{
public:
    static const QString shortDateFormat; // see cpp
    static const char signifyPortfolio = '`';
    static const char stockPrices = 'd';
    static const char stockDividends = 'v';
    static const int nonLeapYear = 2009;

    enum avgShareCalc { calc_FIFO, calc_LIFO, calc_AVG };
    enum outputFormat { format_Currency, format_Percentage, format_Decimal, format_Integer, format_ShortDate, format_LongDate, format_None };
    enum statVariables { stat_Portfolio, stat_PortfolioName, stat_StartDate, stat_EndDate, stat_PreviousDay, stat_TotalValue };
    enum dynamicTradeType { tradeType_Purchase, tradeType_Sale, tradeType_DivReinvest, tradeType_Interest, tradeType_Fixed, tradeType_TotalValue, tradeType_AA, tradeType_Count };
    enum dynamicTradeFreq { tradeFreq_Once, tradeFreq_Daily, tradeFreq_Weekly, tradeFreq_Monthly, tradeFreq_Yearly, tradeFreq_Count };
    enum tickerHistoryChoice { history_All, history_Change, history_Dividends, history_Splits, history_Trades };
    enum thesholdMethod { threshold_Portfolio, theshold_AA };
    enum { columnIDs_Holdings, columnIDs_AA, columnIDs_Acct };

    struct dynamicTrade
    {
        int id;
        dynamicTradeType tradeType;
        double value;
        double price;
        double commission;
        int cashAccount;
        dynamicTradeFreq frequency;
        int date;
        int startDate;
        int endDate;

        dynamicTrade(): id(-1), tradeType(tradeType_Purchase), value(-1), price(-1), commission(-1), cashAccount(-1), frequency(tradeFreq_Once), date(0), startDate(0), endDate(0) {}

        bool operator==(const dynamicTrade &other) const {
            return this->tradeType == other.tradeType
                    && this->value == other.value
                    && this->price == other.price
                    && this->commission == other.commission
                    && this->cashAccount == other.cashAccount
                    && this->frequency == other.frequency
                    && this->date == other.date
                    && this->startDate == other.startDate
                    && this->endDate == other.endDate;
        }

        bool operator!=(const dynamicTrade &other) const {
            return !(*this == other);
        }

        static QString tradeTypeToString(const dynamicTradeType &t)
        {
            switch (t)
            {
                case tradeType_Purchase:
                    return "Purchase";
                case tradeType_Sale:
                    return "Sale";
                case tradeType_DivReinvest:
                    return "Reinvestment";
                case tradeType_Interest:
                    return "Interest";
                case tradeType_Fixed:
                    return "Fixed Amount";
                case tradeType_TotalValue:
                    return "% of Portfolio";
                case tradeType_AA:
                    return "% of AA Target";
                default:
                    return "";
            }
        }

        static QString frequencyToString(const dynamicTradeFreq &f)
        {
            switch (f)
            {
                case tradeFreq_Once:
                    return "Once";
                case tradeFreq_Daily:
                    return "Daily";
                case tradeFreq_Weekly:
                    return "Weekly";
                case tradeFreq_Monthly:
                    return "Monthly";
                case tradeFreq_Yearly:
                    return "Yearly";
                default:
                    return "";
            }
        }

        static QString valueToString(const dynamicTradeType &t, const double &v)
        {
            if (v < 0)
                return "";

            switch (t)
            {
                case tradeType_Purchase:
                case tradeType_Sale:
                case tradeType_DivReinvest:
                    return QString("%L1").arg(v, 0, 'f', 2);
                case tradeType_Interest:
                case tradeType_Fixed:
                    return QString("$%L1").arg(v, 0, 'f', 2);
                case tradeType_TotalValue:
                case tradeType_AA:
                    return QString("%L1%").arg(v, 0, 'f', 2);
                default:
                    return "";
            }
        }

        static QString dateToString(const dynamicTradeFreq &f, const int &date)
        {
            switch (f)
            {
                case tradeFreq_Once:
                    return date != 0 ? QDate::fromJulianDay(date).toString(shortDateFormat) : "";
                case tradeFreq_Daily:
                    return "Market Days";
                case tradeFreq_Weekly:
                    return date != 0 ? QDate::fromJulianDay(date).toString("dddd") : "";
                case tradeFreq_Monthly:
                    return date != 0 ? QDate::fromJulianDay(date).toString("dd") : "";
                case tradeFreq_Yearly:
                    return date != 0 ? QDate::fromJulianDay(date).toString("dd MMM") : "";
                default:
                    return "";
            }
        }
    };

    struct portfolio
    {
        int id;
        QString description;
        bool dividends;
        avgShareCalc costCalc;
        int startValue;
        int aaThreshold;
        thesholdMethod aaThresholdMethod;
        int startDate;
        // start date may be updated if it is a non-market day, but the original date also needs to be tracked
        int origStartDate;
        bool holdingsShowHidden;
        bool navSortDesc;
        bool aaShowBlank;
        bool correlationShowHidden;
        bool acctShowBlank;
        QString holdingsSort;
        QString aaSort;
        QString acctSort;

        portfolio(): id(-1), dividends(true), costCalc(calc_FIFO), startValue(100),
            aaThreshold(5), aaThresholdMethod(threshold_Portfolio), startDate(QDate::currentDate().toJulianDay()), origStartDate(QDate::currentDate().toJulianDay()),
            holdingsShowHidden (true), navSortDesc(true), aaShowBlank(true), correlationShowHidden(true), acctShowBlank(true) {}

        bool operator==(const portfolio &other) const {
            return this->id == other.id
                    && this->description == other.description
                    && this->dividends == other.dividends
                    && this->costCalc == other.costCalc
                    && this->startValue == other.startValue
                    && this->aaThreshold == other.aaThreshold
                    && this->aaThresholdMethod == other.aaThresholdMethod
                    // exclude startDate
                    && this->origStartDate == other.origStartDate
                    && this->holdingsShowHidden == other.holdingsShowHidden
                    && this->navSortDesc == other.navSortDesc
                    && this->aaShowBlank == other.aaShowBlank
                    && this->correlationShowHidden == other.correlationShowHidden
                    && this->acctShowBlank == other.acctShowBlank
                    && this->holdingsSort == other.holdingsSort
                    && this->aaSort == other.aaSort
                    && this->acctSort == other.acctSort
                    ;
        }

        bool operator!=(const portfolio &other) const {
            return !(*this == other);
        }
    };

    struct trade
    {
        int date;
        double shares;
        double price;
        double commission;

        trade(): date(0), shares(0), price(0), commission(0) {}

        bool operator==(const int &other) const {
            return this->date == other;
        }

        bool operator!=(const int &other) const {
            return !(*this == other);
        }
    };

    struct statistic
    {
        int id;
        QString description;
        QString sql;
        outputFormat format;

        statistic(): id(-1), format(format_None) {}

        bool operator==(const statistic &other) const {
            return this->id == other.id
                    && this->description == other.description
                    && this->sql == other.sql
                    && this->format == other.format;
        }

        bool operator!=(const statistic &other) const {
            return !(*this == other);
        }

        bool operator<(const statistic &other) const {
            return this->description.toUpper() < other.description.toUpper();
        }
    };

    struct securityAATarget
    {
        int id;
        double target;

        securityAATarget(int p_id, double p_target): id(p_id), target(p_target) {}

        bool operator==(const securityAATarget &other) const {
            return this->id == other.id
                    && this->target == other.target;
        }

        bool operator!=(const securityAATarget &other) const {
            return !(*this == other);
        }

    };

    struct security
    {
        int id;
        QString ticker;
        int account;
        double expense;
        bool divReinvest;
        bool cashAccount;
        bool includeInCalc;
        bool hide;
        QList<securityAATarget> aa;
        QMap<int, dynamicTrade> trades;

        security(): id(-1), account(-1), expense(-1), divReinvest(false), cashAccount(false),
            includeInCalc(true), hide(false) {}

        bool operator==(const security &other) const {
            return this->id == other.id
                    && this->ticker == other.ticker
                    && this->account == other.account
                    && this->expense == other.expense
                    && this->divReinvest == other.divReinvest
                    && this->cashAccount == other.cashAccount
                    && this->includeInCalc == other.includeInCalc
                    && this->hide == other.hide
                    && this->aa == other.aa
                    && this->trades == other.trades;
        }

        bool operator!=(const security &other) const {
            return !(*this == other);
        }

        int firstTradeDate()
        {
            int minDate = -1;
            foreach(dynamicTrade d, trades)
            {
                if (d.frequency != tradeFreq_Once && (d.startDate < minDate || minDate == -1))
                    minDate = d.startDate;
                else if (d.startDate < d.date && (d.endDate > d.date || d.endDate == 0) && (d.date < minDate || minDate == -1))
                    minDate = d.date;
            }

            return minDate;
        }
    };

    struct securityInfo
    {
        double closePrice;
        double dividendAmount;
        QString ticker;

        securityInfo(): closePrice(0), dividendAmount(0) {}
    };

    struct securityValue
    {
        double totalValue;
        double shares;
        double costBasis;
        double dividendAmount;
        double commission;
        double taxLiability;

        securityValue(): totalValue(0), shares(0), costBasis(0), dividendAmount(0), commission(0), taxLiability(0) {}
    };

    struct assetAllocation
    {
        int id;
        QString description;
        double target;

        assetAllocation(): id(-1), target(-1) {}

        bool operator==(const assetAllocation &other) const {
            return this->id == other.id
                    && this->description == other.description
                    && this->target == other.target;
        }

        bool operator!=(const assetAllocation &other) const {
            return !(*this == other);
        }
    };

    struct account
    {
        int id;
        QString description;
        double taxRate;
        bool taxDeferred;

        account(): id(-1), taxRate(-1), taxDeferred(false) {}

        bool operator==(const account &other) const {
            return this->id == other.id
                    && this->description == other.description
                    && this->taxRate == other.taxRate
                    && this->taxDeferred == other.taxDeferred;
        }

        bool operator!=(const account &other) const {
            return !(*this == other);
        }
    };

    struct settings
    {
        int dataStartDate;
        bool splits;
        int version;
        bool tickersIncludeDividends;
        QVariant lastPortfolio;
        QSize windowSize;
        QPoint windowLocation;
        Qt::WindowState state;
        QMap<int, QList<int> > columns;

        settings(): version(0), lastPortfolio(QVariant(QVariant::Int)) {}

        bool operator==(const settings &other) const {
            // these are the only static properties, the other properties cannot be edited by the user
            return this->dataStartDate == other.dataStartDate
                    && this->splits == other.splits;
        }

        bool operator!=(const settings &other) const {
            return !(*this == other);
        }
    };

    struct navInfo
    {
        double nav;
        double totalValue;

        navInfo(): nav(0), totalValue(0) {}
        navInfo(const double &p_nav, const double &p_totalValue): nav(p_nav), totalValue(p_totalValue) {}
    };

    typedef QMap<int, QList<trade> > tradeList;

    struct portfolioData
    {
        QMap<int, security> tickers;
        QMap<int, assetAllocation> aa;
        QMap<int, account> acct;
        QList<int> stats;
        tradeList trades;
        QMap<int, navInfo> nav;
    };

    struct myPersonalIndex
    {
        portfolioData data;
        portfolio info;

        myPersonalIndex(const globals::portfolio &p): info(p) {}
    };

    struct dynamicTradeInfo
    {
        QString ticker;
        int tickerID;
        dynamicTrade trade;

        dynamicTradeInfo(const QString &p_ticker, const int &p_tickerID, const dynamicTrade &p_trade): ticker(p_ticker), tickerID(p_tickerID), trade(p_trade) {}
    };

    struct securityPriceInfo
    {
        QMap<int, double> splits;
        QMap<int, double> dividends;
        QMap<int, double> prices;
    };

    typedef QHash<QString, QMap<int, double> > splitData;
    typedef QHash<QString, securityPriceInfo> securityPrices;
};

#endif // GLOBALS_H