﻿using System;
using System.Data;
using System.Data.SqlServerCe;
using System.Windows.Forms;

namespace MyPersonalIndex
{
    public partial class frmTickers : Form
    {

        public struct TickerRetValues
        {
            public DateTime MinDate;
            public bool Changed;
        }

        public TickerRetValues TickerReturnValues { get { return _TickerReturnValues; } }

        private TickerQueries SQL = new TickerQueries();
        private const int OriginalWidth = 336;
        private const int ExpandedWidth = 679;
        private const int DateColumn = 0;
        private int PortfolioID;
        private int TickerID;
        private bool Pasted;
        private TickerRetValues _TickerReturnValues = new TickerRetValues();

        public frmTickers(int Portfolio, int Ticker, string sTicker)
        { 
            InitializeComponent();
            this.Width = OriginalWidth;
            PortfolioID = Portfolio;
            TickerID = Ticker;
            this.Text = (string.IsNullOrEmpty(sTicker) ? "New Ticker" : sTicker) + " Properties";
            txtSymbol.Text = sTicker;
            gbActivity.Text = sTicker + " Activity";
        }

        private void frmTickers_Load(object sender, EventArgs e)
        {
            if (SQL.Connection == ConnectionState.Closed)
            {
                DialogResult = DialogResult.Cancel;
                return;
            }

            Pasted = false;
            cmbHis.SelectedIndex = 0;
            cmbHis.SelectedIndexChanged += new EventHandler(cmbHis_SelectedIndexChanged);

            LoadAADropDown();
            LoadAcctDropDown();
            LoadTicker();

            dsTicker.Tables.Add(SQL.ExecuteDataset(TickerQueries.GetTrades(PortfolioID, TickerID)));
            dgTickers.DataSource = dsTicker.Tables[0];

            dsTicker.AcceptChanges();

            if (TickerID == -1)
            {
                txtSymbol.Enabled = true;
                btnHistorical.Visible = false;
                btnCustom.Enabled = false;
            }

            _TickerReturnValues.MinDate = DateTime.Today;
            _TickerReturnValues.Changed = chkCalc.Checked;
            foreach (DataRow dr in dsTicker.Tables[0].Rows)
                if (Convert.ToDateTime(dr[(int)TickerQueries.eGetTrades.Date]) < _TickerReturnValues.MinDate)
                    _TickerReturnValues.MinDate = Convert.ToDateTime(dr[(int)TickerQueries.eGetTrades.Date]);

        }

        private void LoadTicker()
        {
            using (SqlCeResultSet rs = SQL.ExecuteResultSet(TickerQueries.GetAttributes(PortfolioID, TickerID)))
                if (rs.HasRows)
                {
                    rs.ReadFirst();
                    cmbAA.SelectedValue = rs.GetInt32((int)TickerQueries.eGetAttributes.AA);
                    if (cmbAA.SelectedValue == null)
                        cmbAA.SelectedValue = -1;
                    cmbAcct.SelectedValue = rs.GetInt32((int)TickerQueries.eGetAttributes.Acct);
                    if (cmbAcct.SelectedValue == null)
                        cmbAcct.SelectedValue = -1;
                    chkCalc.Checked = rs.GetSqlBoolean((int)TickerQueries.eGetAttributes.Active).IsTrue;
                    chkHide.Checked = rs.GetSqlBoolean((int)TickerQueries.eGetAttributes.Hide).IsTrue;
                }
        }

        private void LoadAADropDown()
        {
            DataTable t = new DataTable();
            t.Columns.Add("Display");
            t.Columns.Add("Value");
            t.Rows.Add("(Blank)", -1);

            using (SqlCeResultSet rs = SQL.ExecuteResultSet(Queries.GetAA(PortfolioID)))
                foreach (SqlCeUpdatableRecord rec in rs)
                    t.Rows.Add(rec.GetString((int)TickerQueries.eGetAA.AA), rec.GetInt32((int)TickerQueries.eGetAA.ID));

            cmbAA.DisplayMember = "Display";
            cmbAA.ValueMember = "Value";
            cmbAA.DataSource = t;

            cmbAA.SelectedValue = -1;
        }

        private void LoadAcctDropDown()
        {
            DataTable t = new DataTable();
            t.Columns.Add("Display");
            t.Columns.Add("Value");
            t.Rows.Add("(Blank)", -1);

            using (SqlCeResultSet rs = SQL.ExecuteResultSet(Queries.GetAcct(PortfolioID)))
                foreach(SqlCeUpdatableRecord rec in rs)
                    t.Rows.Add(rec.GetString((int)TickerQueries.eGetAcct.Name), rec.GetInt32((int)TickerQueries.eGetAcct.ID));

            cmbAcct.DisplayMember = "Display";
            cmbAcct.ValueMember = "Value";
            cmbAcct.DataSource = t;

            cmbAcct.SelectedValue = -1;
        }

        private void dgTickers_DefaultValuesNeeded(object sender, DataGridViewRowEventArgs e)
        {
            e.Row.Cells[0].Value = DateTime.Today;
        }

        private void dgTickers_KeyDown(object sender, KeyEventArgs e)
        {
            if (!(e.Control && e.KeyCode == Keys.V))
                return;
            
            Pasted = true;

            string s = Clipboard.GetText();
            s = s.Replace("\r", "");
            string[] lines = s.Split('\n');
            int row = dgTickers.CurrentCell.RowIndex;
            int origrow = dgTickers.CurrentCell.RowIndex;
            int col = dgTickers.CurrentCell.ColumnIndex;

            dgTickers.CancelEdit();
            dsTicker.AcceptChanges();

            foreach (string line in lines)
            {
                if (line == "")
                    continue;

                string[] cells = line.Split('\t');
                if (row < dgTickers.Rows.Count - 1)
                {
                    for (int i = 0; i < cells.Length; i++)
                    {
                        if (col + i < 3)
                        {
                            try
                            {
                                switch (col + i)
                                {
                                    case 0:
                                        dsTicker.Tables[0].Rows[row][(int)TickerQueries.eGetTrades.Date] = Convert.ToDateTime(cells[i]);
                                        break;
                                    case 1:
                                        dsTicker.Tables[0].Rows[row][(int)TickerQueries.eGetTrades.Shares] = Convert.ToDecimal(cells[i]);
                                        break;
                                    case 2:
                                        dsTicker.Tables[0].Rows[row][(int)TickerQueries.eGetTrades.Price] = Double.Parse(cells[i], System.Globalization.NumberStyles.Currency);
                                        break;
                                }
                            }
                            catch (System.FormatException)
                            {
                                // do nothing
                            }
                            catch (System.OverflowException)
                            {
                                // do nothing
                            }
                            dsTicker.AcceptChanges();
                        }
                    }
                    row++;

                }
                else
                {
                    if (cells.Length == dgTickers.Columns.Count)
                    {
                        try
                        {
                            dsTicker.Tables[0].Rows.Add(Convert.ToDateTime(cells[0]),
                                                        Convert.ToDecimal(cells[1]),
                                                        Double.Parse(cells[2], System.Globalization.NumberStyles.Currency));

                            row++;
                        }
                        catch (System.FormatException)
                        {
                            // do nothing
                        }
                        catch (System.OverflowException)
                        {
                            // do nothing
                        }
                        dsTicker.AcceptChanges();
                    }
                }
            }
              
            dgTickers.CurrentCell = dgTickers[col, origrow];
        }

        private void btnOK_Click(object sender, EventArgs e)
        {
            if (string.IsNullOrEmpty(txtSymbol.Text))
            {
                MessageBox.Show("Set a symbol before saving!");
                return;
            }
            if (TickerReturnValues.Changed != chkCalc.Checked)
            {
                _TickerReturnValues.Changed = true;
                _TickerReturnValues.MinDate = DateTime.MinValue;  // if any custom trades, we need to redo all NAV
            }
            if (TickerID == -1)
            {
                SQL.ExecuteNonQuery(TickerQueries.InsertNewTicker(PortfolioID, txtSymbol.Text, Convert.ToInt32(((DataRowView)cmbAA.SelectedItem)["Value"]), 
                    Convert.ToInt32(((DataRowView)cmbAcct.SelectedItem)["Value"]), chkHide.Checked, chkCalc.Checked));
                TickerID = Convert.ToInt32(SQL.ExecuteScalar(Queries.GetIdentity()));
            }
            else
            {
                SQL.ExecuteNonQuery(TickerQueries.UpdateTicker(PortfolioID, TickerID, Convert.ToInt32(((DataRowView)cmbAA.SelectedItem)["Value"]),
                    Convert.ToInt32(((DataRowView)cmbAcct.SelectedItem)["Value"]), chkHide.Checked, chkCalc.Checked));
                if (dsTicker.HasChanges() || Pasted)
                    SQL.ExecuteNonQuery(Queries.DeleteTickerTrades(PortfolioID, TickerID, false));
            }
        
            if (dsTicker.HasChanges() || Pasted)
            {
                _TickerReturnValues.Changed = true;
                dsTicker.AcceptChanges();

                using (SqlCeResultSet rs = SQL.ExecuteTableUpdate(TickerQueries.Tables.Trades))
                {
                    SqlCeUpdatableRecord newRecord = rs.CreateRecord();
                    int i = 0;

                    foreach (DataRow dr in dsTicker.Tables[0].Rows)
                    {
                        newRecord.SetDateTime((int)TickerQueries.Tables.eTrades.Date, Convert.ToDateTime(dr[(int)TickerQueries.eGetTrades.Date]));
                        newRecord.SetDecimal((int)TickerQueries.Tables.eTrades.Shares, Convert.IsDBNull(dr[(int)TickerQueries.eGetTrades.Shares]) ? 0 : Convert.ToDecimal(dr[(int)TickerQueries.eGetTrades.Shares]));
                        newRecord.SetDecimal((int)TickerQueries.Tables.eTrades.Price, Convert.IsDBNull(dr[(int)TickerQueries.eGetTrades.Price]) ? 0 : Convert.ToDecimal(dr[(int)TickerQueries.eGetTrades.Price]));
                        newRecord.SetInt32((int)TickerQueries.Tables.eTrades.Portfolio, PortfolioID);
                        newRecord.SetInt32((int)TickerQueries.Tables.eTrades.TickerID, TickerID);
                        newRecord.SetInt32((int)TickerQueries.Tables.eTrades.ID, i);
                        newRecord.SetString((int)TickerQueries.Tables.eTrades.Ticker, txtSymbol.Text);
                        rs.Insert(newRecord);

                        i++;
                        if (Convert.ToDateTime(dr[(int)TickerQueries.eGetTrades.Date]) < TickerReturnValues.MinDate)
                            _TickerReturnValues.MinDate = Convert.ToDateTime(dr[(int)TickerQueries.eGetTrades.Date]);
                    }
                }
            }
            DialogResult = DialogResult.OK;
        }

        private void btnCancel_Click(object sender, EventArgs e)
        {
            if (_TickerReturnValues.MinDate == DateTime.MinValue)
            {
                _TickerReturnValues.Changed = true;
                DialogResult = DialogResult.OK;
            }
            else
                DialogResult = DialogResult.Cancel;
        }

        private void frmTickers_FormClosing(object sender, FormClosingEventArgs e)
        {
            SQL.Dispose();
        }

        private void dgTickers_DataError(object sender, DataGridViewDataErrorEventArgs e)
        {
            MessageBox.Show("Invalid format, must be a number!", "Invalid Data", MessageBoxButtons.OK, MessageBoxIcon.Error);
        }

        private void cmbHis_SelectedIndexChanged(object sender, EventArgs e)
        {
            dgHistory.DataSource = null;
            dgHistory.Columns.Clear();

            int i;
            i = dgHistory.Columns.Add("colHisDate", "Date");
            dgHistory.Columns[i].DefaultCellStyle.Format = "d";
            dgHistory.Columns[i].SortMode = DataGridViewColumnSortMode.NotSortable;
            dgHistory.Columns[i].DataPropertyName = "Date";

            if (cmbHis.SelectedIndex == 0)
            {
                i = dgHistory.Columns.Add("colHisPrice", "Price");
                dgHistory.Columns[i].DefaultCellStyle.Format = "N2";
                dgHistory.Columns[i].SortMode = DataGridViewColumnSortMode.NotSortable;
                dgHistory.Columns[i].DataPropertyName = "Price";
            }

            if (cmbHis.SelectedIndex == 0 || cmbHis.SelectedIndex == 1)
            {
                i = dgHistory.Columns.Add("colHisChange", "Change");
                dgHistory.Columns[i].DefaultCellStyle.Format = "#0.00'%'";
                dgHistory.Columns[i].SortMode = DataGridViewColumnSortMode.NotSortable;
                dgHistory.Columns[i].DataPropertyName = "Change";
            }

            if (cmbHis.SelectedIndex == 0 || cmbHis.SelectedIndex == 2)
            {
                i = dgHistory.Columns.Add("colHisDividend", "Dividend");
                dgHistory.Columns[i].DefaultCellStyle.Format = "N2";
                dgHistory.Columns[i].SortMode = DataGridViewColumnSortMode.NotSortable;
                dgHistory.Columns[i].DataPropertyName = "Dividend";
            }

            if (cmbHis.SelectedIndex == 0 || cmbHis.SelectedIndex == 3)
            {
                i = dgHistory.Columns.Add("colHisSplit", "Split");
                dgHistory.Columns[i].DefaultCellStyle.Format = "N2";
                dgHistory.Columns[i].SortMode = DataGridViewColumnSortMode.NotSortable;
                dgHistory.Columns[i].DataPropertyName = "Split";
            }

            if (cmbHis.SelectedIndex == 4)
            {
                i = dgHistory.Columns.Add("colHisPrice", "Price");
                dgHistory.Columns[i].DefaultCellStyle.Format = "N2";
                dgHistory.Columns[i].SortMode = DataGridViewColumnSortMode.NotSortable;
                dgHistory.Columns[i].DataPropertyName = "Price";

                i = dgHistory.Columns.Add("colHisShares", "Shares");
                dgHistory.Columns[i].DefaultCellStyle.Format = "N4";
                dgHistory.Columns[i].SortMode = DataGridViewColumnSortMode.NotSortable;
                dgHistory.Columns[i].DataPropertyName = "Shares";
            }

            dgHistory.DataSource = SQL.ExecuteDataset(TickerQueries.GetHistorical(txtSymbol.Text, TickerID, cmbHis.SelectedIndex, chkSort.Checked));
        }

        private void btnHistorical_Click(object sender, EventArgs e)
        {
            if (gbHistorical.Enabled)
                return;

            this.Width = ExpandedWidth;
            this.Left = this.Left - ((ExpandedWidth - OriginalWidth) / 2);
            gbHistorical.Enabled = true;
            cmbHis_SelectedIndexChanged(null, null);
        }

        private void btnClose_Click(object sender, EventArgs e)
        {
            this.Width = 336;
            this.Left = this.Left + ((ExpandedWidth - OriginalWidth) / 2);
            dgHistory.DataSource = null;
            gbHistorical.Enabled = false;
        }

        private void chkSort_CheckedChanged(object sender, EventArgs e)
        {
            cmbHis_SelectedIndexChanged(null, null);
        }

        private void btnCustom_Click(object sender, EventArgs e)
        {
            using (frmTrades f = new frmTrades(PortfolioID, TickerID, txtSymbol.Text))
            {
                if (f.ShowDialog() == DialogResult.OK)
                {
                    Pasted = true;  // ticker changed
                    _TickerReturnValues.MinDate = DateTime.MinValue;
                }
            }
        }
    }
}