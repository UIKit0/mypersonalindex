#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <QMap>
#include <QSharedData>
#include "security.h"
#include "assetAllocation.h"
#include "account.h"
#include "portfolioAttributes.h"
#include "queries.h"

class portfolioData;
class portfolio
{
public:
    portfolio(int id_ = UNASSIGNED, const QString &description_ = QString());
    portfolio(const portfolio &other_);
    ~portfolio();

    portfolio& operator=(const portfolio &other_);

    QStringList symbols() const;
    int endDate() const;

    static int getOpenIdentity() { return --IDENTITY_COUNTER; }

    bool operator==(const portfolio &other_) const;
    bool operator!=(const portfolio &other_) const { return !(*this == other_); }

    QMap<int, security>& securities() const;
    QMap<int, assetAllocation>& assetAllocations() const;
    QMap<int, account>& accounts() const;
    portfolioAttributes& attributes() const;

    void save(const queries &dataSource_);
    void remove(const queries &dataSource_) const;

    void detach();

private:
    QExplicitlySharedDataPointer<portfolioData> d;
    static int IDENTITY_COUNTER;
};

#endif // PORTFOLIO_H