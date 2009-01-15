-- 1/11/2009 12:36 PM
-- Database information:
-- locale identifier: 1033
-- encryption mode: 
-- case sensitive: False

CREATE TABLE [AA] ([ID] int NOT NULL  IDENTITY (1,1) 
, [Portfolio] int NULL   
, [AA] nvarchar(255) NULL  
, [Target] numeric(6,2) NULL  
);
GO
CREATE TABLE [AvgPricePerShare] ([Ticker] int NULL   
, [Price] numeric(12,4) NULL  
);
GO
CREATE TABLE [ClosingPrices] ([Date] datetime NULL   
, [Ticker] nvarchar(10) NULL  
, [Price] numeric(12,4) NULL  
, [Change] numeric(18,4) NULL  
);
GO
CREATE TABLE [Dividends] ([Date] datetime NULL   
, [Ticker] nvarchar(10) NULL  
, [Amount] numeric(12,4) NULL  
);
GO
CREATE TABLE [NAV] ([Portfolio] int NULL   
, [Date] datetime NULL   
, [TotalValue] numeric(24,4) NULL  
, [NAV] numeric(24,8) NULL  
, [Change] numeric(18,4) NULL  
);
GO
CREATE TABLE [Portfolios] ([ID] int NOT NULL  IDENTITY (1,1) 
, [Name] nvarchar(100) NULL  
, [Dividends] bit NULL   
, [CostCalc] int NULL   
, [HoldingsShowHidden] bit NULL   
, [HoldingsSort] nvarchar(250) NULL  
, [NAVSort] bit NULL   
, [NAVStartValue] numeric(12,2) NULL  
, [AAThreshold] int NULL   
, [AASort] nvarchar(250) NULL  
, [AAShowBlank] bit NULL   
, [CorrelationShowHidden] bit NULL   
, [StartDate] datetime NULL   
);
GO
CREATE TABLE [Settings] ([DataStartDate] datetime NULL   
, [LastPortfolio] int NULL   
, [Version] numeric(6,2) NULL  
, [WindowX] int NULL   
, [WindowY] int NULL   
, [WindowHeight] int NULL   
, [WindowWidth] int NULL   
, [WindowState] int NULL   
);
GO
CREATE TABLE [Splits] ([Date] datetime NULL   
, [Ticker] nvarchar(10) NULL  
, [Ratio] numeric(10,4) NULL  
);
GO
CREATE TABLE [Stats] ([Portfolio] int NULL   
, [Statistic] int NULL   
, [Location] int NULL   
);
GO
CREATE TABLE [Tickers] ([ID] int NOT NULL  IDENTITY (1,1) 
, [Ticker] nvarchar(10) NULL  
, [Portfolio] int NULL   
, [Active] bit NULL   
, [AA] int NULL   
, [Hide] bit NULL   
);
GO
CREATE TABLE [Trades] ([Date] datetime NULL   
, [Portfolio] int NULL   
, [TickerID] int NULL   
, [Ticker] nvarchar(10) NULL  
, [Shares] numeric(12,4) NULL  
, [Price] numeric(12,4) NULL  
, [ID] int NULL   
);
GO
CREATE TABLE [UserStatistics] ([ID] int NOT NULL  IDENTITY (1,1) 
, [SQL] nvarchar(4000) NULL  
, [Format] int NULL   
, [Description] nvarchar(100) NULL  
);
GO
SET IDENTITY_INSERT [AA] ON
GO
SET IDENTITY_INSERT [AA] OFF
GO
SET IDENTITY_INSERT [Portfolios] ON
GO
SET IDENTITY_INSERT [Portfolios] OFF
GO
Insert Into [Settings] ([DataStartDate],[LastPortfolio],[Version],[WindowX],[WindowY],[WindowHeight],[WindowWidth],[WindowState]) Values ({ts '2008-01-02 12:00:00'},null,1.00,null,null,null,null,null);
GO
SET IDENTITY_INSERT [Tickers] ON
GO
SET IDENTITY_INSERT [Tickers] OFF
GO
SET IDENTITY_INSERT [UserStatistics] ON
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (1,N'SELECT NAV / %NAVStartValue% * 100 AS NAV
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date = ''%EndDate%''',2,N'Current Index Value');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (2,N'SELECT TotalValue
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date = ''%EndDate%''',0,N'Current Value');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (3,N'SELECT (CASE WHEN Days = 0 THEN 0
ELSE 100 * POWER(NAV, 1.0 / Days) - 100 END) AS DailyReturn
FROM
(SELECT NAV / %NAVStartValue% AS NAV
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date = ''%EndDate%'') a
CROSS JOIN
(SELECT COUNT(*) AS Days
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%'') b',1,N'Daily Return');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (4,N'SELECT SQRT((SUM(POWER(Change,2))-POWER(SUM(Change)/COUNT(*),2)) / COUNT(*)) FROM NAV WHERE Portfolio = %Portfolio% AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',1,N'Daily Standard Deviation');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (5,N'SELECT MAX(Date)
FROM NAV
WHERE Portfolio = %Portfolio% 
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',4,N'Date');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (6,N'SELECT COUNT(*) 
FROM NAV 
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',3,N'Days Invested');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (7,N'SELECT (CASE WHEN Days = 0 THEN 0
ELSE 100 * POWER(NAV, 1.0 / (Days * 6.5)) - 100 END) AS DailyReturn
FROM
(SELECT NAV / %NAVStartValue% AS NAV
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date = ''%EndDate%'') a
CROSS JOIN
(SELECT COUNT(*) AS Days
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%'') b',1,N'Hourly Return');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (8,N'SELECT MIN(Change)
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',1,N'Max % Down');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (9,N'SELECT Date
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
AND Change IN
(SELECT MIN(Change)
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%'')',4,N'Max % Down Day');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (10,N'SELECT MAX(Change)
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',1,N'Max % Up');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (11,N'SELECT Date
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
AND Change IN
(SELECT MAX(Change)
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%'')',4,N'Max % Up Day');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (12,N'SELECT MAX(NAV)
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',2,N'Maximum Index Value');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (13,N'SELECT Date
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
AND NAV IN (
	SELECT MAX(NAV)
	FROM NAV
	WHERE Portfolio = %Portfolio%
	AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
)',4,N'Maximum Index Value Day');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (14,N'SELECT MAX(TotalValue)
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',0,N'Maximum Portfolio Value');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (15,N'SELECT Date
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
AND TotalValue IN (
	SELECT MAX(TotalValue)
	FROM NAV
	WHERE Portfolio = %Portfolio%
	AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
)',4,N'Maximum Portfolio Value Day');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (16,N'SELECT MIN(NAV)
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',2,N'Minimum Index Value');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (17,N'SELECT Date
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
AND NAV IN (
	SELECT MIN(NAV)
	FROM NAV
	WHERE Portfolio = %Portfolio%
	AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
)',4,N'Minimum Index Value Day');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (18,N'SELECT MIN(TotalValue)
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',0,N'Minimum Portfolio Value');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (19,N'SELECT Date
FROM NAV
WHERE Portfolio = %Portfolio%
AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
AND TotalValue IN (
	SELECT MIN(TotalValue)
	FROM NAV
	WHERE Portfolio = %Portfolio%
	AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''
)',4,N'Minimum Portfolio Value Day');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (20,N'SELECT (CASE WHEN DATEDIFF(day, ''%StartDate%'', ''%EndDate%'') / 30 = 0 THEN NAV / %NAVStartValue% - 1
              ELSE POWER(NAV / %NAVStartValue%, 1.0 / (DATEDIFF(day, ''%StartDate%'', ''%EndDate%'') / 30)) - 1 END) * 100 AS MonthlyReturn
FROM NAV 
WHERE Portfolio = %Portfolio%
AND Date = ''%EndDate%''',1,N'Monthly Return');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (21,N'SELECT SQRT(22) * SQRT((SUM(POWER(Change,2))-POWER(SUM(Change)/COUNT(*),2)) / COUNT(*)) FROM NAV WHERE Portfolio = %Portfolio% AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',1,N'Monthly Standard Deviation');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (22,N'SELECT (CASE WHEN z >= 0 THEN  (1.0 - 0.39894228 * exp( -z * z / 2.0 ) * t * ( t *( t * ( t * ( t * 1.330274429 + -1.821255978 ) + 1.781477937 ) + -0.356563782 ) + 0.319381530 ))
        ELSE ( 0.39894228 * exp( -z * z / 2.0 ) * t * ( t *( t * ( t * ( t * 1.330274429 + -1.821255978 ) + 1.781477937 ) + -0.356563782 ) + 0.319381530 )) END) * 100 AS Probability
FROM (SELECT (CASE WHEN YearlyGain / StdDev >= 0 THEN 1.0 / ( 1.0 + 0.2316419 * (YearlyGain / StdDev) ) ELSE 1.0 / ( 1.0 - 0.2316419 * (YearlyGain / StdDev) ) END) AS t,
	  YearlyGain / StdDev AS z
	  FROM (SELECT SQRT(250) * SQRT((SUM(POWER(Change,2))-POWER(SUM(Change)/COUNT(*),2)) / COUNT(*)) / 100 AS StdDev
			FROM NAV 
			WHERE Portfolio = %Portfolio%
			AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%'') a
	  CROSS JOIN
			(SELECT (CASE WHEN DATEDIFF(day, ''%StartDate%'', ''%EndDate%'') / 365 = 0 THEN NAV / %NAVStartValue% - 1 ELSE
					POWER(NAV / %NAVStartValue%, 1/(DATEDIFF(day, ''%StartDate%'', ''%EndDate%'') / 365)) - 1 END) AS YearlyGain
			FROM NAV 
			WHERE Portfolio = %Portfolio%
			AND Date = ''%EndDate%'') b
) Calcs',1,N'Probability of Yearly Gain');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (23,N'SELECT 100 - (CASE WHEN z >= 0 THEN  (1.0 - 0.39894228 * exp( -z * z / 2.0 ) * t * ( t *( t * ( t * ( t * 1.330274429 + -1.821255978 ) + 1.781477937 ) + -0.356563782 ) + 0.319381530 ))
        ELSE ( 0.39894228 * exp( -z * z / 2.0 ) * t * ( t *( t * ( t * ( t * 1.330274429 + -1.821255978 ) + 1.781477937 ) + -0.356563782 ) + 0.319381530 )) END) * 100 AS Probability
FROM (SELECT (CASE WHEN YearlyGain / StdDev >= 0 THEN 1.0 / ( 1.0 + 0.2316419 * (YearlyGain / StdDev) ) ELSE 1.0 / ( 1.0 - 0.2316419 * (YearlyGain / StdDev) ) END) AS t,
	  YearlyGain / StdDev AS z
	  FROM (SELECT SQRT(250) * SQRT((SUM(POWER(Change,2))-POWER(SUM(Change)/COUNT(*),2)) / COUNT(*)) / 100 AS StdDev
			FROM NAV 
			WHERE Portfolio = %Portfolio%
			AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%'') a
	  CROSS JOIN
			(SELECT (CASE WHEN DATEDIFF(day, ''%StartDate%'', ''%EndDate%'') / 365 = 0 THEN NAV / %NAVStartValue% - 1 ELSE
					POWER(NAV / %NAVStartValue%, 1/(DATEDIFF(day, ''%StartDate%'', ''%EndDate%'') / 365)) - 1 END) AS YearlyGain
			FROM NAV 
			WHERE Portfolio = %Portfolio%
			AND Date = ''%EndDate%'') b
) Calcs',1,N'Probability Of Yearly Loss');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (24,N'SELECT (CASE WHEN DATEDIFF(day, ''%StartDate%'', ''%EndDate%'') / 365 = 0 THEN NAV / %NAVStartValue% - 1
              ELSE POWER(NAV / %NAVStartValue%, 1.0 / (DATEDIFF(day, ''%StartDate%'', ''%EndDate%'') / 365)) - 1 END) * 100 AS YearlyReturn
FROM NAV 
WHERE Portfolio = %Portfolio%
AND Date = ''%EndDate%''',1,N'Yearly Return');
GO
Insert Into [UserStatistics] ([ID],[SQL],[Format],[Description]) Values (25,N'SELECT SQRT(250) * SQRT((SUM(POWER(Change,2))-POWER(SUM(Change)/COUNT(*),2)) / COUNT(*)) FROM NAV WHERE Portfolio = %Portfolio% AND Date BETWEEN ''%StartDate%'' AND ''%EndDate%''',1,N'Yearly Standard Deviation');
GO
SET IDENTITY_INSERT [UserStatistics] OFF
GO
ALTER TABLE [AA] ADD PRIMARY KEY ([ID]);
GO
ALTER TABLE [Portfolios] ADD PRIMARY KEY ([ID]);
GO
ALTER TABLE [Tickers] ADD PRIMARY KEY ([ID]);
GO
ALTER TABLE [UserStatistics] ADD PRIMARY KEY ([ID]);
GO
GO
CREATE UNIQUE INDEX [DateTicker] ON [ClosingPrices] ([Date] Asc,[Ticker] Asc);
GO
CREATE UNIQUE INDEX [DateTicker] ON [Dividends] ([Date] Asc,[Ticker] Asc);
GO
CREATE UNIQUE INDEX [DatePortfolio] ON [NAV] ([Portfolio] Asc,[Date] Asc);
GO
CREATE UNIQUE INDEX [DateTicker] ON [Splits] ([Date] Asc,[Ticker] Asc);
GO
CREATE INDEX [DateTickerPortfolio] ON [Trades] ([Date] Asc,[Portfolio] Asc,[TickerID] Asc);
GO

