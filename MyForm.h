#pragma once

#include <complex>
#include <cmath>

namespace motorsimulation {

    using namespace System;
    using namespace System::Drawing;
    using namespace System::Windows::Forms;
    using namespace System::Windows::Forms::DataVisualization::Charting;
    using namespace System::Globalization;
    using namespace System::Collections::Generic;
    using namespace System::Drawing::Printing;

    public ref class MyForm : public Form
    {
    private:
        // ── Main inputs ────────────────────────────────────────
        TextBox^ txtV, ^ txtFreq, ^ txtPoles;
        TextBox^ txtR1, ^ txtX1, ^ txtR2, ^ txtX2, ^ txtXm, ^ txtRc;
        TextBox^ txtMechLoss, ^ txtRatedSpeed, ^ txtLoadTorque;
        ComboBox^ cbConn;

        // ── Buttons ────────────────────────────────────────────
        Button^ btnAnalyze, ^ btnSave, ^ btnReset;
        Button^ btnExport, ^ btnCSV, ^ btnPrint;
        Button^ btnSaveProf, ^ btnLoadProf, ^ btnFreeze;
        Button^ btnParamCalc;

        // ── Status / warning ───────────────────────────────────
        Label^ lblStatus, ^ lblWarning;
        RichTextBox^ txtLog;

        // ── 5 Charts ───────────────────────────────────────────
        Chart^ chartTorque, ^ chartCurrent, ^ chartEff, ^ chartSlip, ^ chartPout;

        // ── State ──────────────────────────────────────────────
        bool isFrozen;

        // ── Sweep arrays for CSV export ────────────────────────
        List<double>^ swN, ^ swSlip, ^ swTq, ^ swI, ^ swEff, ^ swPF, ^ swPout;

        // ── Param-calc dialog controls (valid during dialog) ───
        TextBox^ pc_Vnl, ^ pc_Inl, ^ pc_Pnl;
        TextBox^ pc_Vbr, ^ pc_Ibr, ^ pc_Pbr, ^ pc_R1dc;
        ComboBox^ pc_Conn;
        RichTextBox^ pc_Result;
        Button^ pc_BtnApply;
        double pc_r1, pc_x1, pc_r2, pc_x2, pc_xm, pc_rc;

        // ── Print document ─────────────────────────────────────
        PrintDocument^ printDoc;

    public:
        MyForm(void) {
            isFrozen = false;
            swN = gcnew List<double>(); swSlip = gcnew List<double>();
            swTq = gcnew List<double>(); swI = gcnew List<double>();
            swEff = gcnew List<double>(); swPF = gcnew List<double>();
            swPout = gcnew List<double>();
            pc_r1 = pc_x1 = pc_r2 = pc_x2 = pc_xm = pc_rc = 0.0;
            printDoc = gcnew PrintDocument();
            printDoc->PrintPage += gcnew PrintPageEventHandler(this, &MyForm::OnPrintPage);
            InitializeUI();
        }

    private:
        // =========================================================
        //  HELPERS
        // =========================================================
        void LogLine(String^ lbl, String^ val, Color c) {
            txtLog->SelectionColor = Color::FromArgb(115, 115, 115);
            txtLog->AppendText(lbl);
            txtLog->SelectionColor = c;
            txtLog->AppendText(val + "\n");
        }
        void LogHeader(String^ text) {
            txtLog->SelectionColor = Color::FromArgb(0, 175, 175);
            txtLog->AppendText(text + "\n");
        }

        double Val(TextBox^ t) {
            double r = 0;
            if (!Double::TryParse(t->Text->Trim()->Replace(",", "."),
                NumberStyles::Any,
                CultureInfo::InvariantCulture, r))
                throw gcnew Exception("Invalid input: \"" + t->Text + "\"");
            return r;
        }
        double SafeVal(TextBox^ t, double fallback) {
            double r = 0;
            return Double::TryParse(t->Text->Trim()->Replace(",", "."),
                NumberStyles::Any,
                CultureInfo::InvariantCulture, r) ? r : fallback;
        }

        TextBox^ AddInput(Panel^ p, String^ lbl, String^ def, int y) {
            Label^ l = gcnew Label();
            l->Text = lbl; l->Location = Point(15, y + 3);
            l->AutoSize = true;
            l->ForeColor = Color::FromArgb(170, 170, 170);
            l->Font = gcnew System::Drawing::Font("Segoe UI", 9);
            p->Controls->Add(l);
            TextBox^ t = gcnew TextBox();
            t->Text = def; t->Location = Point(218, y);
            t->Width = 108; t->Height = 24;
            t->BackColor = Color::FromArgb(44, 44, 52);
            t->ForeColor = Color::White;
            t->BorderStyle = BorderStyle::FixedSingle;
            t->Font = gcnew System::Drawing::Font("Consolas", 9);
            p->Controls->Add(t);
            return t;
        }
        void AddSectionLabel(Panel^ p, String^ text, int y) {
            Label^ l = gcnew Label();
            l->Text = text; l->Location = Point(10, y);
            l->AutoSize = true;
            l->ForeColor = Color::FromArgb(0, 185, 185);
            l->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            p->Controls->Add(l);
        }
        void AddHint(Panel^ p, String^ text, int y) {
            Label^ l = gcnew Label();
            l->Text = text; l->Location = Point(15, y);
            l->AutoSize = true;
            l->ForeColor = Color::FromArgb(80, 80, 80);
            l->Font = gcnew System::Drawing::Font("Segoe UI", 7, FontStyle::Italic);
            p->Controls->Add(l);
        }

        // =========================================================
        //  DIALOG HELPERS (Lambdalar yerine eklendi)
        // =========================================================
        void AddDlgLabel(Panel^ dp, String^ text, int y, Color c, bool bold) {
            Label^ l = gcnew Label(); l->Text = text; l->Location = Point(12, y);
            l->AutoSize = true; l->ForeColor = c;
            l->Font = gcnew System::Drawing::Font("Segoe UI", 9, bold ? FontStyle::Bold : FontStyle::Regular);
            dp->Controls->Add(l);
        }

        TextBox^ AddDlgInput(Panel^ dp, String^ lbl, String^ def, int y) {
            Label^ l = gcnew Label(); l->Text = lbl; l->Location = Point(12, y + 3);
            l->AutoSize = true; l->ForeColor = Color::FromArgb(170, 170, 170);
            dp->Controls->Add(l);
            TextBox^ t = gcnew TextBox(); t->Text = def; t->Location = Point(270, y);
            t->Width = 105; t->BackColor = Color::FromArgb(44, 44, 52);
            t->ForeColor = Color::White; t->BorderStyle = BorderStyle::FixedSingle;
            t->Font = gcnew System::Drawing::Font("Consolas", 9);
            dp->Controls->Add(t); return t;
        }

        // ── Build one dark-themed Chart ───────────────────────
        Chart^ MakeChart(String^ yTitle, Color yColor, String^ xTitle,
            bool hasY2, String^ y2Title, Color y2Color) {
            Chart^ ch = gcnew Chart();
            ch->Dock = DockStyle::Fill;
            ch->BackColor = Color::FromArgb(20, 20, 27);

            ChartArea^ a = gcnew ChartArea("main");
            a->BackColor = Color::FromArgb(25, 25, 33);
            a->BorderColor = Color::FromArgb(48, 48, 62);

            // ── X axis
            a->AxisX->Title = xTitle;
            a->AxisX->TitleForeColor = Color::FromArgb(195, 195, 195);
            a->AxisX->TitleFont = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            a->AxisX->LabelStyle->ForeColor = Color::FromArgb(178, 178, 178);
            a->AxisX->LabelStyle->Font = gcnew System::Drawing::Font("Consolas", 7);
            a->AxisX->Minimum = 0;
            a->AxisX->IsMarginVisible = false;
            a->AxisX->MajorGrid->LineColor = Color::FromArgb(36, 36, 48);
            a->AxisX->MajorGrid->LineDashStyle = ChartDashStyle::Dot;
            a->AxisX->LineColor = Color::FromArgb(65, 65, 80);
            // Zoom enabled — drag to select region, right-click to reset
            a->AxisX->ScaleView->Zoomable = true;

            // ── Y1 axis
            a->AxisY->Title = yTitle;
            a->AxisY->TitleForeColor = yColor;
            a->AxisY->TitleFont = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            a->AxisY->LabelStyle->ForeColor = yColor;
            a->AxisY->LabelStyle->Font = gcnew System::Drawing::Font("Consolas", 7);
            a->AxisY->Minimum = 0;
            a->AxisY->MajorGrid->LineColor = Color::FromArgb(36, 36, 48);
            a->AxisY->MajorGrid->LineDashStyle = ChartDashStyle::Dot;
            a->AxisY->LineColor = Color::FromArgb(65, 65, 80);
            a->AxisY->ScaleView->Zoomable = true;

            // ── Y2 axis (optional)
            if (hasY2) {
                a->AxisY2->Enabled = AxisEnabled::True;
                a->AxisY2->Title = y2Title;
                a->AxisY2->TitleForeColor = y2Color;
                a->AxisY2->TitleFont = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
                a->AxisY2->LabelStyle->ForeColor = y2Color;
                a->AxisY2->LabelStyle->Font = gcnew System::Drawing::Font("Consolas", 7);
                a->AxisY2->MajorGrid->Enabled = false;
                a->AxisY2->LineColor = Color::FromArgb(65, 65, 80);
                a->AxisY2->Minimum = 0.0;
                a->AxisY2->Maximum = 1.05;
            }

            // ── Crosshair cursors (follow mouse)
            a->CursorX->LineColor = Color::FromArgb(140, 255, 215, 0);
            a->CursorX->LineWidth = 1;
            a->CursorX->LineDashStyle = ChartDashStyle::Dash;
            a->CursorX->IsUserSelectionEnabled = true;  // drag → zoom
            a->CursorY->LineColor = Color::FromArgb(70, 190, 190, 190);
            a->CursorY->LineWidth = 1;
            a->CursorY->LineDashStyle = ChartDashStyle::Dot;
            a->CursorY->IsUserSelectionEnabled = true;

            ch->ChartAreas->Add(a);

            Legend^ lg = gcnew Legend();
            lg->BackColor = Color::FromArgb(26, 26, 36);
            lg->ForeColor = Color::White;
            lg->Font = gcnew System::Drawing::Font("Segoe UI", 8);
            ch->Legends->Add(lg);

            return ch;
        }

        void AddSeries(Chart^ ch, String^ name, Color c,
            int width, AxisType yAxis, ChartDashStyle dash,
            String^ tooltip) {
            Series^ s = gcnew Series(name);
            s->ChartType = SeriesChartType::Spline;
            s->BorderWidth = width;
            s->Color = c;
            s->YAxisType = yAxis;
            s->BorderDashStyle = dash;
            s->ShadowOffset = 1;
            s->ToolTip = tooltip;      // hover tooltip
            ch->Series->Add(s);
        }

        // Overload without tooltip (frozen/reference series)
        void AddSeries(Chart^ ch, String^ name, Color c,
            int width, AxisType yAxis, ChartDashStyle dash) {
            AddSeries(ch, name, c, width, yAxis, dash, "");
        }

        // ── Hook zoom + crosshair events to a chart ───────────
        void EnableInteraction(Chart^ ch) {
            ch->MouseWheel += gcnew MouseEventHandler(this, &MyForm::OnChartWheel);
            ch->DoubleClick += gcnew EventHandler(this, &MyForm::OnChartDblClick);
            ch->MouseMove += gcnew MouseEventHandler(this, &MyForm::OnChartMouseMove);
        }

        // ── Rated-speed stripline ─────────────────────────────
        void AddRatedStripLine(Chart^ ch, double nRated) {
            ch->ChartAreas["main"]->AxisX->StripLines->Clear();
            StripLine^ sl = gcnew StripLine();
            sl->Interval = 0;
            sl->IntervalOffset = nRated;
            sl->StripWidth = 3;               // 3 RPM wide on 0-1500 axis — thin visible line
            sl->BackColor = Color::FromArgb(100, 255, 220, 0);
            sl->BorderColor = Color::FromArgb(220, 255, 220, 0);
            sl->BorderWidth = 1;
            sl->BorderDashStyle = ChartDashStyle::Dash;
            sl->Text = "n_rated";
            sl->ForeColor = Color::FromArgb(255, 225, 0);
            sl->Font = gcnew System::Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            sl->TextAlignment = StringAlignment::Near;
            sl->TextLineAlignment = StringAlignment::Far;  // text at bottom → avoids curve
            ch->ChartAreas["main"]->AxisX->StripLines->Add(sl);
        }

        // ── Critical-slip stripline ───────────────────────────
        void AddSlipStripLine(Chart^ ch, double slipCrit) {
            ch->ChartAreas["main"]->AxisX->StripLines->Clear();
            StripLine^ sl = gcnew StripLine();
            sl->Interval = 0;
            sl->IntervalOffset = slipCrit;
            sl->StripWidth = 0.006;           // slip axis 0-1, thin line
            sl->BackColor = Color::FromArgb(80, 255, 100, 0);
            sl->BorderColor = Color::FromArgb(200, 255, 100, 0);
            sl->BorderWidth = 1;
            sl->BorderDashStyle = ChartDashStyle::Dash;
            sl->Text = "s_crit";
            sl->ForeColor = Color::FromArgb(255, 140, 0);
            sl->Font = gcnew System::Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            sl->TextAlignment = StringAlignment::Near;
            sl->TextLineAlignment = StringAlignment::Far;
            ch->ChartAreas["main"]->AxisX->StripLines->Add(sl);
        }

        // ── IEC 60034-30 efficiency class ─────────────────────
        String^ IEClass(double eta) {
            if (eta >= 93.0) return "IE4 (Super Premium)";
            if (eta >= 91.0) return "IE3 (Premium)";
            if (eta >= 87.6) return "IE2 (High Efficiency)";
            return "IE1 (Standard)";
        }

        void ShowWarning(String^ msg) {
            lblWarning->Text = "  WARN: " + msg;
            lblWarning->Visible = true;
        }
        void HideWarning() { lblWarning->Visible = false; }

        // =========================================================
        //  UI BUILD
        // =========================================================
        void InitializeUI() {
            this->Text = "Asynchronous Motor Analyzer";
            this->Size = System::Drawing::Size(1280, 860);
            this->MinimumSize = System::Drawing::Size(980, 680);
            this->WindowState = FormWindowState::Maximized;
            this->BackColor = Color::FromArgb(17, 17, 22);
            this->ForeColor = Color::White;
            this->Font = gcnew System::Drawing::Font("Segoe UI", 9);

            // ── Status bar ───────────────────────────────────────
            lblStatus = gcnew Label();
            lblStatus->Dock = DockStyle::Bottom;
            lblStatus->Height = 24;
            lblStatus->BackColor = Color::FromArgb(0, 85, 85);
            lblStatus->ForeColor = Color::White;
            lblStatus->Text = "  Ready — Enter parameters and click RUN ANALYSIS  |  Drag chart to zoom  |  Dbl-click to reset zoom";
            lblStatus->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Italic);
            this->Controls->Add(lblStatus);

            // ── Warning label (above status bar) ─────────────────
            lblWarning = gcnew Label();
            lblWarning->Dock = DockStyle::Bottom;
            lblWarning->Height = 22;
            lblWarning->BackColor = Color::FromArgb(140, 30, 30);
            lblWarning->ForeColor = Color::White;
            lblWarning->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            lblWarning->Visible = false;
            this->Controls->Add(lblWarning);

            // ── Right panel (Fill — BEFORE left) ─────────────────
            Panel^ rightPanel = gcnew Panel();
            rightPanel->Dock = DockStyle::Fill;
            rightPanel->BackColor = Color::FromArgb(17, 17, 22);
            rightPanel->Padding = System::Windows::Forms::Padding(4);
            this->Controls->Add(rightPanel);

            // ── Separator ─────────────────────────────────────────
            Panel^ sep = gcnew Panel();
            sep->Dock = DockStyle::Left;
            sep->Width = 2;
            sep->BackColor = Color::FromArgb(0, 142, 142);
            this->Controls->Add(sep);

            // ── Left panel ────────────────────────────────────────
            Panel^ leftPanel = gcnew Panel();
            leftPanel->Dock = DockStyle::Left;
            leftPanel->Width = 356;
            leftPanel->BackColor = Color::FromArgb(22, 22, 29);
            leftPanel->AutoScroll = true;
            this->Controls->Add(leftPanel);

            // ─── Title ────────────────────────────────────────────
            int y = 12;
            Label^ titleLbl = gcnew Label();
            titleLbl->Text = "ASYNCHRONOUS MOTOR ANALYZER";
            titleLbl->Font = gcnew System::Drawing::Font("Segoe UI", 10, FontStyle::Bold);
            titleLbl->ForeColor = Color::FromArgb(0, 200, 200);
            titleLbl->Location = Point(10, y);
            titleLbl->AutoSize = true;
            leftPanel->Controls->Add(titleLbl); y += 32;

            // ─── SUPPLY ───────────────────────────────────────────
            AddSectionLabel(leftPanel, "[ SUPPLY ]", y); y += 20;
            txtV = AddInput(leftPanel, "Line Voltage (V)", "400", y); y += 30;
            txtFreq = AddInput(leftPanel, "Frequency (Hz)", "50", y); y += 30;
            txtPoles = AddInput(leftPanel, "No. of Poles", "4", y); y += 30;

            Label^ connLbl = gcnew Label();
            connLbl->Text = "Connection Type";
            connLbl->Location = Point(15, y + 3);
            connLbl->AutoSize = true;
            connLbl->ForeColor = Color::FromArgb(170, 170, 170);
            leftPanel->Controls->Add(connLbl);
            cbConn = gcnew ComboBox();
            cbConn->Items->Add("Star (Y)"); cbConn->Items->Add("Delta");
            cbConn->SelectedIndex = 0; cbConn->Location = Point(218, y);
            cbConn->Width = 108; cbConn->DropDownStyle = ComboBoxStyle::DropDownList;
            cbConn->BackColor = Color::FromArgb(44, 44, 52); cbConn->ForeColor = Color::White;
            leftPanel->Controls->Add(cbConn); y += 36;

            // ─── EQUIVALENT CIRCUIT ───────────────────────────────
            AddSectionLabel(leftPanel, "[ EQUIVALENT CIRCUIT  (per-phase, Ohm) ]", y); y += 20;
            txtR1 = AddInput(leftPanel, "Stator Resistance  R1", "0.641", y); y += 30;
            txtX1 = AddInput(leftPanel, "Stator Reactance   X1", "1.106", y); y += 30;
            txtR2 = AddInput(leftPanel, "Rotor Resistance   R2'", "0.332", y); y += 30;
            txtX2 = AddInput(leftPanel, "Rotor Reactance    X2'", "0.464", y); y += 30;
            txtXm = AddInput(leftPanel, "Magnetizing React. Xm", "26.30", y); y += 30;
            txtRc = AddInput(leftPanel, "Core Loss Resist.  Rc", "210.0", y); y += 24;
            AddHint(leftPanel, "  ^ Zm = Rc // jXm   (iron loss modelled in circuit)", y); y += 18;

            // ─── MECHANICAL & NAMEPLATE ───────────────────────────
            AddSectionLabel(leftPanel, "[ MECHANICAL  &  NAMEPLATE ]", y); y += 20;
            txtMechLoss = AddInput(leftPanel, "Friction+Windage (W)", "150", y); y += 24;
            AddHint(leftPanel, "  ^ nameplate / no-load test — INPUT (constant)", y); y += 18;
            txtRatedSpeed = AddInput(leftPanel, "Rated Speed (RPM)", "1450", y); y += 24;
            AddHint(leftPanel, "  ^ nameplate — used for rated-point analysis", y); y += 18;
            txtLoadTorque = AddInput(leftPanel, "Fan Load Torque (Nm)", "0", y); y += 24;
            AddHint(leftPanel, "  ^ T_L = TL*(n/ns)^2  (0 = disabled)", y); y += 22;

            // ─── BUTTONS ROW 1 ────────────────────────────────────
            btnAnalyze = gcnew Button();
            btnAnalyze->Text = "> RUN ANALYSIS";
            btnAnalyze->Size = System::Drawing::Size(152, 40);
            btnAnalyze->Location = Point(10, y);
            btnAnalyze->FlatStyle = FlatStyle::Flat;
            btnAnalyze->BackColor = Color::FromArgb(0, 140, 140);
            btnAnalyze->ForeColor = Color::White;
            btnAnalyze->Font = gcnew System::Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            btnAnalyze->FlatAppearance->BorderSize = 0;
            btnAnalyze->Click += gcnew EventHandler(this, &MyForm::Analyze_Click);
            leftPanel->Controls->Add(btnAnalyze);

            btnSave = gcnew Button();
            btnSave->Text = "PNG"; btnSave->Size = System::Drawing::Size(55, 40);
            btnSave->Location = Point(168, y); btnSave->FlatStyle = FlatStyle::Flat;
            btnSave->BackColor = Color::FromArgb(40, 44, 56); btnSave->ForeColor = Color::White;
            btnSave->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            btnSave->FlatAppearance->BorderSize = 0;
            btnSave->Click += gcnew EventHandler(this, &MyForm::Save_Click);
            leftPanel->Controls->Add(btnSave);

            btnPrint = gcnew Button();
            btnPrint->Text = "PDF"; btnPrint->Size = System::Drawing::Size(55, 40);
            btnPrint->Location = Point(229, y); btnPrint->FlatStyle = FlatStyle::Flat;
            btnPrint->BackColor = Color::FromArgb(55, 35, 55); btnPrint->ForeColor = Color::FromArgb(200, 160, 255);
            btnPrint->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            btnPrint->FlatAppearance->BorderSize = 0;
            btnPrint->Click += gcnew EventHandler(this, &MyForm::Print_Click);
            leftPanel->Controls->Add(btnPrint);

            btnReset = gcnew Button();
            btnReset->Text = "CLR"; btnReset->Size = System::Drawing::Size(47, 40);
            btnReset->Location = Point(290, y); btnReset->FlatStyle = FlatStyle::Flat;
            btnReset->BackColor = Color::FromArgb(72, 30, 30); btnReset->ForeColor = Color::White;
            btnReset->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            btnReset->FlatAppearance->BorderSize = 0;
            btnReset->Click += gcnew EventHandler(this, &MyForm::Reset_Click);
            leftPanel->Controls->Add(btnReset); y += 44;

            // ─── BUTTONS ROW 2 ────────────────────────────────────
            btnExport = gcnew Button();
            btnExport->Text = "EXPORT TXT"; btnExport->Size = System::Drawing::Size(105, 34);
            btnExport->Location = Point(10, y); btnExport->FlatStyle = FlatStyle::Flat;
            btnExport->BackColor = Color::FromArgb(30, 58, 30); btnExport->ForeColor = Color::FromArgb(100, 220, 100);
            btnExport->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            btnExport->FlatAppearance->BorderSize = 1;
            btnExport->FlatAppearance->BorderColor = Color::FromArgb(55, 120, 55);
            btnExport->Click += gcnew EventHandler(this, &MyForm::Export_Click);
            leftPanel->Controls->Add(btnExport);

            btnCSV = gcnew Button();
            btnCSV->Text = "EXPORT CSV"; btnCSV->Size = System::Drawing::Size(103, 34);
            btnCSV->Location = Point(120, y); btnCSV->FlatStyle = FlatStyle::Flat;
            btnCSV->BackColor = Color::FromArgb(28, 52, 60); btnCSV->ForeColor = Color::FromArgb(80, 200, 240);
            btnCSV->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            btnCSV->FlatAppearance->BorderSize = 1;
            btnCSV->FlatAppearance->BorderColor = Color::FromArgb(50, 110, 140);
            btnCSV->Click += gcnew EventHandler(this, &MyForm::CSV_Click);
            leftPanel->Controls->Add(btnCSV);

            btnFreeze = gcnew Button();
            btnFreeze->Text = "FREEZE"; btnFreeze->Size = System::Drawing::Size(103, 34);
            btnFreeze->Location = Point(228, y); btnFreeze->FlatStyle = FlatStyle::Flat;
            btnFreeze->BackColor = Color::FromArgb(38, 38, 70); btnFreeze->ForeColor = Color::FromArgb(160, 160, 255);
            btnFreeze->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            btnFreeze->FlatAppearance->BorderSize = 1;
            btnFreeze->FlatAppearance->BorderColor = Color::FromArgb(80, 80, 160);
            btnFreeze->Click += gcnew EventHandler(this, &MyForm::Freeze_Click);
            leftPanel->Controls->Add(btnFreeze); y += 38;

            // ─── BUTTONS ROW 3 ────────────────────────────────────
            btnSaveProf = gcnew Button();
            btnSaveProf->Text = "SAVE PROFILE"; btnSaveProf->Size = System::Drawing::Size(115, 34);
            btnSaveProf->Location = Point(10, y); btnSaveProf->FlatStyle = FlatStyle::Flat;
            btnSaveProf->BackColor = Color::FromArgb(42, 38, 28); btnSaveProf->ForeColor = Color::FromArgb(220, 190, 80);
            btnSaveProf->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            btnSaveProf->FlatAppearance->BorderSize = 1;
            btnSaveProf->FlatAppearance->BorderColor = Color::FromArgb(110, 95, 40);
            btnSaveProf->Click += gcnew EventHandler(this, &MyForm::SaveProfile_Click);
            leftPanel->Controls->Add(btnSaveProf);

            btnLoadProf = gcnew Button();
            btnLoadProf->Text = "LOAD PROFILE"; btnLoadProf->Size = System::Drawing::Size(115, 34);
            btnLoadProf->Location = Point(130, y); btnLoadProf->FlatStyle = FlatStyle::Flat;
            btnLoadProf->BackColor = Color::FromArgb(42, 38, 28); btnLoadProf->ForeColor = Color::FromArgb(220, 190, 80);
            btnLoadProf->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            btnLoadProf->FlatAppearance->BorderSize = 1;
            btnLoadProf->FlatAppearance->BorderColor = Color::FromArgb(110, 95, 40);
            btnLoadProf->Click += gcnew EventHandler(this, &MyForm::LoadProfile_Click);
            leftPanel->Controls->Add(btnLoadProf);

            btnParamCalc = gcnew Button();
            btnParamCalc->Text = "CALC"; btnParamCalc->Size = System::Drawing::Size(91, 34);
            btnParamCalc->Location = Point(250, y); btnParamCalc->FlatStyle = FlatStyle::Flat;
            btnParamCalc->BackColor = Color::FromArgb(28, 45, 45); btnParamCalc->ForeColor = Color::FromArgb(0, 190, 190);
            btnParamCalc->Font = gcnew System::Drawing::Font("Segoe UI", 8, FontStyle::Bold);
            btnParamCalc->FlatAppearance->BorderSize = 1;
            btnParamCalc->FlatAppearance->BorderColor = Color::FromArgb(0, 100, 100);

            ToolTip^ tip = gcnew ToolTip();
            tip->SetToolTip(btnParamCalc, "Parameter Calculator — compute R1,X1,R2,X2,Xm,Rc from No-Load & Blocked-Rotor test data");

            btnParamCalc->Click += gcnew EventHandler(this, &MyForm::ParamCalc_Click);
            leftPanel->Controls->Add(btnParamCalc); y += 42;

            // ─── REPORT BOX ───────────────────────────────────────
            AddSectionLabel(leftPanel, "[ ANALYSIS REPORT ]", y); y += 20;
            txtLog = gcnew RichTextBox();
            txtLog->Location = Point(10, y);
            txtLog->Size = System::Drawing::Size(330, 480);
            txtLog->BackColor = Color::FromArgb(9, 9, 13);
            txtLog->ForeColor = Color::FromArgb(0, 205, 0);
            txtLog->Font = gcnew System::Drawing::Font("Consolas", 8);
            txtLog->BorderStyle = BorderStyle::None;
            txtLog->ReadOnly = true;
            txtLog->ScrollBars = RichTextBoxScrollBars::Vertical;
            leftPanel->Controls->Add(txtLog);

            // =========================================================
            //  CHART GRID  —  TableLayoutPanel  2 cols × 3 rows
            //  Row 0 (40%): Torque-Speed  |  Current-Speed
            //  Row 1 (40%): Eff+PF-Speed  |  Torque-Slip
            //  Row 2 (20%): Eta vs Pout  (colspan 2)
            // =========================================================
            TableLayoutPanel^ grid = gcnew TableLayoutPanel();
            grid->Dock = DockStyle::Fill;
            grid->ColumnCount = 2; grid->RowCount = 3;
            grid->BackColor = Color::FromArgb(10, 10, 14);
            grid->CellBorderStyle = TableLayoutPanelCellBorderStyle::Single;
            grid->Padding = System::Windows::Forms::Padding(0);
            grid->Margin = System::Windows::Forms::Padding(0);
            grid->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 50.0f));
            grid->ColumnStyles->Add(gcnew ColumnStyle(SizeType::Percent, 50.0f));
            grid->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 40.0f));
            grid->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 40.0f));
            grid->RowStyles->Add(gcnew RowStyle(SizeType::Percent, 20.0f));

            // [0,0] Torque vs Speed
            chartTorque = MakeChart("Torque (Nm)", Color::OrangeRed, "Speed (RPM)", false, "", Color::White);
            AddSeries(chartTorque, "Torque (Nm)", Color::OrangeRed, 3,
                AxisType::Primary, ChartDashStyle::Solid,
                "Speed: #VALX{F0} RPM\nTorque: #VALY{F2} Nm");
            AddSeries(chartTorque, "Load Line", Color::FromArgb(160, 160, 160), 2,
                AxisType::Primary, ChartDashStyle::DashDot,
                "Speed: #VALX{F0} RPM\nLoad T: #VALY{F2} Nm");
            AddSeries(chartTorque, "Frozen Tq", Color::FromArgb(110, 255, 120, 50), 2,
                AxisType::Primary, ChartDashStyle::Dash);
            EnableInteraction(chartTorque);
            grid->Controls->Add(chartTorque, 0, 0);

            // [1,0] Current vs Speed
            chartCurrent = MakeChart("Line Current (A)", Color::DeepSkyBlue, "Speed (RPM)", false, "", Color::White);
            AddSeries(chartCurrent, "Current (A)", Color::DeepSkyBlue, 3,
                AxisType::Primary, ChartDashStyle::Solid,
                "Speed: #VALX{F0} RPM\nCurrent: #VALY{F2} A");
            AddSeries(chartCurrent, "Frozen I", Color::FromArgb(110, 40, 160, 220), 2,
                AxisType::Primary, ChartDashStyle::Dash);
            EnableInteraction(chartCurrent);
            grid->Controls->Add(chartCurrent, 1, 0);

            // [0,1] Efficiency + PF vs Speed
            chartEff = MakeChart("Efficiency (%)", Color::LimeGreen, "Speed (RPM)", true, "Power Factor", Color::Gold);
            AddSeries(chartEff, "Efficiency (%)", Color::LimeGreen, 3,
                AxisType::Primary, ChartDashStyle::Solid,
                "Speed: #VALX{F0} RPM\nEfficiency: #VALY{F2} %");
            AddSeries(chartEff, "Power Factor", Color::Gold, 2,
                AxisType::Secondary, ChartDashStyle::Dash,
                "Speed: #VALX{F0} RPM\nPF: #VALY{F4}");
            AddSeries(chartEff, "Frozen Eff", Color::FromArgb(110, 80, 180, 80), 2,
                AxisType::Primary, ChartDashStyle::Dash);
            EnableInteraction(chartEff);
            grid->Controls->Add(chartEff, 0, 1);

            // [1,1] Torque vs Slip (X reversed)
            chartSlip = MakeChart("Torque (Nm)", Color::DarkOrange, "Slip (s)", false, "", Color::White);
            chartSlip->ChartAreas["main"]->AxisX->Maximum = 1.0;
            chartSlip->ChartAreas["main"]->AxisX->Interval = 0.1;
            chartSlip->ChartAreas["main"]->AxisX->IsReversed = true;
            AddSeries(chartSlip, "Slip-Torque", Color::DarkOrange, 3,
                AxisType::Primary, ChartDashStyle::Solid,
                "Slip: #VALX{F4}\nTorque: #VALY{F2} Nm");
            EnableInteraction(chartSlip);
            grid->Controls->Add(chartSlip, 1, 1);

            // [0-1,2] Eta vs Pout (colspan 2)
            chartPout = MakeChart("Efficiency Eta (%)", Color::MediumOrchid, "Output Power Pout (kW)", false, "", Color::White);
            AddSeries(chartPout, "Eta vs Pout", Color::MediumOrchid, 3,
                AxisType::Primary, ChartDashStyle::Solid,
                "Pout: #VALX{F2} kW\nEta: #VALY{F2} %");
            EnableInteraction(chartPout);
            grid->Controls->Add(chartPout, 0, 2);
            grid->SetColumnSpan(chartPout, 2);

            rightPanel->Controls->Add(grid);
        }

        // =========================================================
        //  CHART INTERACTION EVENTS
        // =========================================================
        void OnChartWheel(Object^ sender, MouseEventArgs^ e) {
            Chart^ ch = (Chart^)sender;
            ChartArea^ a = ch->ChartAreas["main"];
            double xMin = a->AxisX->ScaleView->IsZoomed
                ? a->AxisX->ScaleView->ViewMinimum : a->AxisX->Minimum;
            double xMax = a->AxisX->ScaleView->IsZoomed
                ? a->AxisX->ScaleView->ViewMaximum : a->AxisX->Maximum;
            double range = xMax - xMin;
            double center = xMin + range / 2.0;
            double factor = (e->Delta > 0) ? 0.75 : 1.33;
            double newHalf = (range * factor) / 2.0;
            double axMin = a->AxisX->Minimum;
            double axMax = a->AxisX->Maximum;
            if (newHalf * 2.0 >= (axMax - axMin) * 0.97) {
                a->AxisX->ScaleView->ZoomReset();
            }
            else {
                double nMin = Math::Max(axMin, center - newHalf);
                double nMax = Math::Min(axMax, center + newHalf);
                a->AxisX->ScaleView->Zoom(nMin, nMax);
            }
        }

        void OnChartDblClick(Object^ sender, EventArgs^ e) {
            Chart^ ch = (Chart^)sender;
            ch->ChartAreas["main"]->AxisX->ScaleView->ZoomReset();
            ch->ChartAreas["main"]->AxisY->ScaleView->ZoomReset();
        }

        void OnChartMouseMove(Object^ sender, MouseEventArgs^ e) {
            Chart^ ch = (Chart^)sender;
            try {
                ChartArea^ a = ch->ChartAreas["main"];
                a->CursorX->SetCursorPixelPosition(Point(e->X, e->Y), true);
                a->CursorY->SetCursorPixelPosition(Point(e->X, e->Y), true);
            }
            catch (...) {}
        }

        // =========================================================
        //  RESET
        // =========================================================
        void Reset_Click(Object^ sender, EventArgs^ e) {
            for each(Series ^ s in chartTorque->Series)  s->Points->Clear();
            for each(Series ^ s in chartCurrent->Series) s->Points->Clear();
            for each(Series ^ s in chartEff->Series)     s->Points->Clear();
            for each(Series ^ s in chartSlip->Series)    s->Points->Clear();
            for each(Series ^ s in chartPout->Series)    s->Points->Clear();

            chartTorque->ChartAreas["main"]->AxisX->StripLines->Clear();
            chartCurrent->ChartAreas["main"]->AxisX->StripLines->Clear();
            chartEff->ChartAreas["main"]->AxisX->StripLines->Clear();
            chartSlip->ChartAreas["main"]->AxisX->StripLines->Clear();
            chartPout->ChartAreas["main"]->AxisX->StripLines->Clear();

            swN->Clear(); swSlip->Clear(); swTq->Clear(); swI->Clear();
            swEff->Clear(); swPF->Clear(); swPout->Clear();

            isFrozen = false;
            btnFreeze->Text = "FREEZE";
            btnFreeze->BackColor = Color::FromArgb(38, 38, 70);

            txtLog->Clear();
            HideWarning();
            lblStatus->Text = "  Charts cleared.";
        }

        // =========================================================
        //  FREEZE / COMPARE MODE
        // =========================================================
        void Freeze_Click(Object^ sender, EventArgs^ e) {
            if (swTq == nullptr || swTq->Count == 0) {
                lblStatus->Text = "  Run analysis first before freezing.";
                return;
            }
            if (!isFrozen) {
                // Copy live series into frozen (dashed) series
                chartTorque->Series["Frozen Tq"]->Points->Clear();
                for each(DataPoint ^ pt in chartTorque->Series["Torque (Nm)"]->Points)
                    chartTorque->Series["Frozen Tq"]->Points->AddXY(pt->XValue, pt->YValues[0]);

                chartCurrent->Series["Frozen I"]->Points->Clear();
                for each(DataPoint ^ pt in chartCurrent->Series["Current (A)"]->Points)
                    chartCurrent->Series["Frozen I"]->Points->AddXY(pt->XValue, pt->YValues[0]);

                chartEff->Series["Frozen Eff"]->Points->Clear();
                for each(DataPoint ^ pt in chartEff->Series["Efficiency (%)"]->Points)
                    chartEff->Series["Frozen Eff"]->Points->AddXY(pt->XValue, pt->YValues[0]);

                isFrozen = true;
                btnFreeze->Text = "UNFREEZE";
                btnFreeze->BackColor = Color::FromArgb(80, 35, 35);
                lblStatus->Text = "  Comparison frozen — change parameters and run again to compare.";
            }
            else {
                chartTorque->Series["Frozen Tq"]->Points->Clear();
                chartCurrent->Series["Frozen I"]->Points->Clear();
                chartEff->Series["Frozen Eff"]->Points->Clear();
                isFrozen = false;
                btnFreeze->Text = "FREEZE";
                btnFreeze->BackColor = Color::FromArgb(38, 38, 70);
                lblStatus->Text = "  Freeze cleared.";
            }
        }

        // =========================================================
        //  EXPORT REPORT — TXT
        // =========================================================
        void Export_Click(Object^ sender, EventArgs^ e) {
            if (txtLog->Text->Trim()->Length == 0) {
                MessageBox::Show("No report to export. Run analysis first.", "Nothing to Export",
                    MessageBoxButtons::OK, MessageBoxIcon::Information);
                return;
            }
            try {
                SaveFileDialog^ sfd = gcnew SaveFileDialog();
                sfd->Filter = "Text File|*.txt"; sfd->FileName = "Motor_Report";
                if (sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK) return;

                System::IO::StreamWriter^ sw = gcnew System::IO::StreamWriter(
                    sfd->FileName, false, System::Text::Encoding::UTF8);

                sw->WriteLine("=======================================================");
                sw->WriteLine("   ASYNCHRONOUS INDUCTION MOTOR — ANALYSIS REPORT");
                sw->WriteLine("   Generated : " + DateTime::Now.ToString("yyyy-MM-dd  HH:mm:ss"));
                sw->WriteLine("=======================================================");
                sw->WriteLine();
                sw->WriteLine("INPUT PARAMETERS");
                sw->WriteLine("-------------------------------------------------------");
                sw->WriteLine("  Line Voltage        : " + txtV->Text + " V");
                sw->WriteLine("  Frequency           : " + txtFreq->Text + " Hz");
                sw->WriteLine("  No. of Poles        : " + txtPoles->Text);
                sw->WriteLine("  Connection          : " + cbConn->Text);
                sw->WriteLine("  Stator R1           : " + txtR1->Text + " Ohm");
                sw->WriteLine("  Stator X1           : " + txtX1->Text + " Ohm");
                sw->WriteLine("  Rotor  R2'          : " + txtR2->Text + " Ohm");
                sw->WriteLine("  Rotor  X2'          : " + txtX2->Text + " Ohm");
                sw->WriteLine("  Magnetizing Xm      : " + txtXm->Text + " Ohm");
                sw->WriteLine("  Core Loss Rc        : " + txtRc->Text + " Ohm");
                sw->WriteLine("  Friction+Windage    : " + txtMechLoss->Text + " W");
                sw->WriteLine("  Rated Speed (plate) : " + txtRatedSpeed->Text + " RPM");
                sw->WriteLine("  Fan Load Torque     : " + txtLoadTorque->Text + " Nm @ ns");
                sw->WriteLine();
                sw->WriteLine("SIMULATION RESULTS");
                sw->WriteLine("-------------------------------------------------------");
                sw->WriteLine(txtLog->Text);
                sw->WriteLine();
                sw->WriteLine("=======================================================");
                sw->WriteLine("  Model : Per-phase T-equivalent circuit");
                sw->WriteLine("  Zm = Rc || jXm  (iron loss modelled in circuit)");
                sw->WriteLine("  Power chain: Pin-Pcu1-Pfe-Pgap-Pcu2-Pconv-Pout");
                sw->WriteLine("=======================================================");
                sw->Close();

                lblStatus->Text = "  Report exported -> " + sfd->FileName;
                MessageBox::Show("Report saved!\n" + sfd->FileName, "Export OK",
                    MessageBoxButtons::OK, MessageBoxIcon::Information);
            }
            catch (Exception^ ex) {
                MessageBox::Show(ex->Message, "Export Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        // =========================================================
        //  EXPORT CSV — sweep data
        // =========================================================
        void CSV_Click(Object^ sender, EventArgs^ e) {
            if (swN == nullptr || swN->Count == 0) {
                MessageBox::Show("No data to export. Run analysis first.", "Nothing to Export",
                    MessageBoxButtons::OK, MessageBoxIcon::Information);
                return;
            }
            try {
                SaveFileDialog^ sfd = gcnew SaveFileDialog();
                sfd->Filter = "CSV File|*.csv"; sfd->FileName = "Motor_Data";
                if (sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK) return;

                System::IO::StreamWriter^ sw = gcnew System::IO::StreamWriter(
                    sfd->FileName, false, System::Text::Encoding::UTF8);

                sw->WriteLine("Speed_RPM,Slip,Torque_Nm,Current_A,Efficiency_pct,PowerFactor,Pout_kW");
                for (int i = 0; i < swN->Count; i++) {
                    sw->WriteLine(
                        swN[i].ToString("F1", CultureInfo::InvariantCulture) + "," +
                        swSlip[i].ToString("F6", CultureInfo::InvariantCulture) + "," +
                        swTq[i].ToString("F4", CultureInfo::InvariantCulture) + "," +
                        swI[i].ToString("F4", CultureInfo::InvariantCulture) + "," +
                        swEff[i].ToString("F3", CultureInfo::InvariantCulture) + "," +
                        swPF[i].ToString("F5", CultureInfo::InvariantCulture) + "," +
                        swPout[i].ToString("F4", CultureInfo::InvariantCulture)
                    );
                }
                sw->Close();
                lblStatus->Text = "  CSV exported (" + swN->Count.ToString() + " rows) -> " + sfd->FileName;
            }
            catch (Exception^ ex) {
                MessageBox::Show(ex->Message, "CSV Export Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        // =========================================================
        //  SAVE PNG
        // =========================================================
        void Save_Click(Object^ sender, EventArgs^ e) {
            try {
                SaveFileDialog^ sfd = gcnew SaveFileDialog();
                sfd->Filter = "PNG Image|*.png|JPEG Image|*.jpg";
                sfd->FileName = "Motor_Analysis";
                if (sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK) return;

                int W = 1600, H = 1000, HT = (int)(H * 0.4), HB = H - HT * 2;
                Bitmap^ bmp = gcnew Bitmap(W, H);
                Graphics^ g = Graphics::FromImage(bmp);
                g->Clear(Color::FromArgb(17, 17, 22));

                Bitmap^ t1 = gcnew Bitmap(W / 2, HT);
                chartTorque->DrawToBitmap(t1, Rectangle(0, 0, W / 2, HT));
                g->DrawImage(t1, 0, 0, W / 2, HT);

                Bitmap^ t2 = gcnew Bitmap(W / 2, HT);
                chartCurrent->DrawToBitmap(t2, Rectangle(0, 0, W / 2, HT));
                g->DrawImage(t2, W / 2, 0, W / 2, HT);

                Bitmap^ t3 = gcnew Bitmap(W / 2, HT);
                chartEff->DrawToBitmap(t3, Rectangle(0, 0, W / 2, HT));
                g->DrawImage(t3, 0, HT, W / 2, HT);

                Bitmap^ t4 = gcnew Bitmap(W / 2, HT);
                chartSlip->DrawToBitmap(t4, Rectangle(0, 0, W / 2, HT));
                g->DrawImage(t4, W / 2, HT, W / 2, HT);

                Bitmap^ t5 = gcnew Bitmap(W, HB);
                chartPout->DrawToBitmap(t5, Rectangle(0, 0, W, HB));
                g->DrawImage(t5, 0, HT * 2, W, HB);

                System::Drawing::Imaging::ImageFormat^ fmt =
                    sfd->FileName->EndsWith(".jpg", StringComparison::OrdinalIgnoreCase)
                    ? System::Drawing::Imaging::ImageFormat::Jpeg
                    : System::Drawing::Imaging::ImageFormat::Png;
                bmp->Save(sfd->FileName, fmt);
                lblStatus->Text = "  PNG exported -> " + sfd->FileName;
            }
            catch (Exception^ ex) {
                MessageBox::Show(ex->Message, "Export Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        // =========================================================
        //  PRINT / PDF
        // =========================================================
        void Print_Click(Object^ sender, EventArgs^ e) {
            PrintDialog^ pd = gcnew PrintDialog();
            pd->Document = printDoc;
            pd->UseEXDialog = true;
            if (pd->ShowDialog() == System::Windows::Forms::DialogResult::OK)
                printDoc->Print();
        }

        void OnPrintPage(Object^ sender, PrintPageEventArgs^ e) {
            Rectangle b = e->MarginBounds;
            int W = b.Width, H = b.Height, HT = (int)(H * 0.38), HB = H - HT * 2;

            Bitmap^ t1 = gcnew Bitmap(W / 2, HT);
            chartTorque->DrawToBitmap(t1, Rectangle(0, 0, W / 2, HT));
            e->Graphics->DrawImage(t1, b.X, b.Y, W / 2, HT);

            Bitmap^ t2 = gcnew Bitmap(W / 2, HT);
            chartCurrent->DrawToBitmap(t2, Rectangle(0, 0, W / 2, HT));
            e->Graphics->DrawImage(t2, b.X + W / 2, b.Y, W / 2, HT);

            Bitmap^ t3 = gcnew Bitmap(W / 2, HT);
            chartEff->DrawToBitmap(t3, Rectangle(0, 0, W / 2, HT));
            e->Graphics->DrawImage(t3, b.X, b.Y + HT, W / 2, HT);

            Bitmap^ t4 = gcnew Bitmap(W / 2, HT);
            chartSlip->DrawToBitmap(t4, Rectangle(0, 0, W / 2, HT));
            e->Graphics->DrawImage(t4, b.X + W / 2, b.Y + HT, W / 2, HT);

            Bitmap^ t5 = gcnew Bitmap(W, HB);
            chartPout->DrawToBitmap(t5, Rectangle(0, 0, W, HB));
            e->Graphics->DrawImage(t5, b.X, b.Y + HT * 2, W, HB);

            // Footer
            String^ footer = "Asynchronous Motor Analyzer  |  " +
                DateTime::Now.ToString("yyyy-MM-dd HH:mm");
            e->Graphics->DrawString(footer,
                gcnew System::Drawing::Font("Segoe UI", 7),
                Brushes::Gray,
                PointF((float)b.X, (float)(b.Y + H + 4)));
        }

        // =========================================================
        //  SAVE / LOAD PROFILE
        // =========================================================
        void SaveProfile_Click(Object^ sender, EventArgs^ e) {
            try {
                SaveFileDialog^ sfd = gcnew SaveFileDialog();
                sfd->Filter = "Motor Profile|*.mprof|All Files|*.*";
                sfd->FileName = "motor_profile";
                if (sfd->ShowDialog() != System::Windows::Forms::DialogResult::OK) return;

                System::IO::StreamWriter^ sw = gcnew System::IO::StreamWriter(sfd->FileName);
                sw->WriteLine("# Motor Analyzer Profile");
                sw->WriteLine("V=" + txtV->Text);
                sw->WriteLine("Freq=" + txtFreq->Text);
                sw->WriteLine("Poles=" + txtPoles->Text);
                sw->WriteLine("Conn=" + cbConn->SelectedIndex.ToString());
                sw->WriteLine("R1=" + txtR1->Text);
                sw->WriteLine("X1=" + txtX1->Text);
                sw->WriteLine("R2=" + txtR2->Text);
                sw->WriteLine("X2=" + txtX2->Text);
                sw->WriteLine("Xm=" + txtXm->Text);
                sw->WriteLine("Rc=" + txtRc->Text);
                sw->WriteLine("MechLoss=" + txtMechLoss->Text);
                sw->WriteLine("RatedSpeed=" + txtRatedSpeed->Text);
                sw->WriteLine("LoadTorque=" + txtLoadTorque->Text);
                sw->Close();
                lblStatus->Text = "  Profile saved -> " + sfd->FileName;
            }
            catch (Exception^ ex) {
                MessageBox::Show(ex->Message, "Save Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        void LoadProfile_Click(Object^ sender, EventArgs^ e) {
            try {
                OpenFileDialog^ ofd = gcnew OpenFileDialog();
                ofd->Filter = "Motor Profile|*.mprof|All Files|*.*";
                if (ofd->ShowDialog() != System::Windows::Forms::DialogResult::OK) return;

                array<String^>^ lines = System::IO::File::ReadAllLines(ofd->FileName);
                for each(String ^ line in lines) {
                    if (line->StartsWith("#") || !line->Contains("=")) continue;
                    array<String^>^ parts = line->Split(gcnew array<wchar_t>{'='}, 2);
                    if (parts->Length != 2) continue;
                    String^ k = parts[0]->Trim();
                    String^ v = parts[1]->Trim();
                    if (k == "V")           txtV->Text = v;
                    else if (k == "Freq")   txtFreq->Text = v;
                    else if (k == "Poles")  txtPoles->Text = v;
                    else if (k == "Conn") {
                        int idx = 0;
                        if (Int32::TryParse(v, idx)) cbConn->SelectedIndex = Math::Min(idx, 1);
                    }
                    else if (k == "R1")         txtR1->Text = v;
                    else if (k == "X1")         txtX1->Text = v;
                    else if (k == "R2")         txtR2->Text = v;
                    else if (k == "X2")         txtX2->Text = v;
                    else if (k == "Xm")         txtXm->Text = v;
                    else if (k == "Rc")         txtRc->Text = v;
                    else if (k == "MechLoss")   txtMechLoss->Text = v;
                    else if (k == "RatedSpeed") txtRatedSpeed->Text = v;
                    else if (k == "LoadTorque") txtLoadTorque->Text = v;
                }
                lblStatus->Text = "  Profile loaded: " + ofd->FileName;
            }
            catch (Exception^ ex) {
                MessageBox::Show(ex->Message, "Load Error", MessageBoxButtons::OK, MessageBoxIcon::Error);
            }
        }

        // =========================================================
        //  PARAMETER CALCULATOR — No-Load & Blocked-Rotor Tests
        // =========================================================
        void ParamCalc_Click(Object^ sender, EventArgs^ e) {
            Form^ dlg = gcnew Form();
            dlg->Text = "Parameter Calculator  —  No-Load & Blocked-Rotor Tests";
            dlg->Size = System::Drawing::Size(510, 610);
            dlg->StartPosition = FormStartPosition::CenterParent;
            dlg->BackColor = Color::FromArgb(22, 22, 29);
            dlg->ForeColor = Color::White;
            dlg->Font = gcnew System::Drawing::Font("Segoe UI", 9);
            dlg->FormBorderStyle = System::Windows::Forms::FormBorderStyle::FixedDialog;
            dlg->MaximizeBox = false; dlg->MinimizeBox = false;

            Panel^ dp = gcnew Panel();
            dp->Dock = DockStyle::Fill; dp->AutoScroll = true;
            dlg->Controls->Add(dp);

            int dy = 12;

            AddDlgLabel(dp, "NO-LOAD TEST  (rated voltage, no mechanical load)", dy, Color::FromArgb(0, 185, 185), true); dy += 24;
            AddDlgLabel(dp, "  — measures Xm and Rc (magnetizing + iron loss branch)", dy, Color::FromArgb(80, 80, 80), false); dy += 22;
            pc_Vnl = AddDlgInput(dp, "No-Load Line Voltage  V_NL (V)", "400", dy); dy += 30;
            pc_Inl = AddDlgInput(dp, "No-Load Line Current  I_NL (A)", "5.2", dy); dy += 30;
            pc_Pnl = AddDlgInput(dp, "No-Load Total Power   P_NL (W)", "380", dy); dy += 30;

            pc_Conn = gcnew ComboBox();
            pc_Conn->Items->Add("Star (Y)"); pc_Conn->Items->Add("Delta");
            pc_Conn->SelectedIndex = 0; pc_Conn->Location = Point(270, dy);
            pc_Conn->Width = 105; pc_Conn->DropDownStyle = ComboBoxStyle::DropDownList;
            pc_Conn->BackColor = Color::FromArgb(44, 44, 52); pc_Conn->ForeColor = Color::White;
            Label^ connL = gcnew Label(); connL->Text = "Connection";
            connL->Location = Point(12, dy + 3); connL->AutoSize = true;
            connL->ForeColor = Color::FromArgb(170, 170, 170); dp->Controls->Add(connL);
            dp->Controls->Add(pc_Conn); dy += 34;

            AddDlgLabel(dp, "BLOCKED-ROTOR TEST  (reduced voltage, rated current, rotor locked)", dy, Color::FromArgb(0, 185, 185), true); dy += 24;
            AddDlgLabel(dp, "  — measures R1+R2 and X1+X2  (short-circuit impedance)", dy, Color::FromArgb(80, 80, 80), false); dy += 22;
            pc_Vbr = AddDlgInput(dp, "Blocked-Rotor Voltage  V_BR (V)", "55", dy); dy += 30;
            pc_Ibr = AddDlgInput(dp, "Blocked-Rotor Current  I_BR (A)", "23.4", dy); dy += 30;
            pc_Pbr = AddDlgInput(dp, "Blocked-Rotor Power    P_BR (W)", "960", dy); dy += 30;
            pc_R1dc = AddDlgInput(dp, "DC Stator Resistance   R1_dc (Ohm)", "0.641", dy); dy += 34;

            AddDlgLabel(dp, "RESULTS", dy, Color::FromArgb(0, 185, 185), true); dy += 22;

            pc_Result = gcnew RichTextBox();
            pc_Result->Location = Point(12, dy); pc_Result->Size = System::Drawing::Size(460, 120);
            pc_Result->BackColor = Color::FromArgb(10, 10, 14); pc_Result->ForeColor = Color::FromArgb(0, 210, 0);
            pc_Result->Font = gcnew System::Drawing::Font("Consolas", 8);
            pc_Result->ReadOnly = true; pc_Result->BorderStyle = BorderStyle::None;
            dp->Controls->Add(pc_Result); dy += 126;

            // Calculate button
            Button^ btnCalc = gcnew Button();
            btnCalc->Text = "CALCULATE"; btnCalc->Size = System::Drawing::Size(220, 36);
            btnCalc->Location = Point(12, dy); btnCalc->FlatStyle = FlatStyle::Flat;
            btnCalc->BackColor = Color::FromArgb(0, 130, 130); btnCalc->ForeColor = Color::White;
            btnCalc->Font = gcnew System::Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            btnCalc->FlatAppearance->BorderSize = 0;
            btnCalc->Click += gcnew EventHandler(this, &MyForm::ParamCalcCompute_Click);
            dp->Controls->Add(btnCalc);

            // Apply button
            pc_BtnApply = gcnew Button();
            pc_BtnApply->Text = "APPLY TO MAIN FORM"; pc_BtnApply->Size = System::Drawing::Size(220, 36);
            pc_BtnApply->Location = Point(242, dy); pc_BtnApply->FlatStyle = FlatStyle::Flat;
            pc_BtnApply->BackColor = Color::FromArgb(30, 65, 30); pc_BtnApply->ForeColor = Color::FromArgb(100, 220, 100);
            pc_BtnApply->Font = gcnew System::Drawing::Font("Segoe UI", 9, FontStyle::Bold);
            pc_BtnApply->FlatAppearance->BorderSize = 1;
            pc_BtnApply->FlatAppearance->BorderColor = Color::FromArgb(55, 120, 55);
            pc_BtnApply->Enabled = false;
            pc_BtnApply->Click += gcnew EventHandler(this, &MyForm::ParamCalcApply_Click);
            dp->Controls->Add(pc_BtnApply);

            dlg->ShowDialog(this);
        }

        void ParamCalcCompute_Click(Object^ sender, EventArgs^ e) {
            try {
                bool star = (pc_Conn->SelectedIndex == 0);
                double Vnl = SafeVal(pc_Vnl, 400.0);
                double Inl = SafeVal(pc_Inl, 5.0);
                double Pnl = SafeVal(pc_Pnl, 300.0);
                double Vbr = SafeVal(pc_Vbr, 55.0);
                double Ibr = SafeVal(pc_Ibr, 23.0);
                double Pbr = SafeVal(pc_Pbr, 900.0);
                double R1dc = SafeVal(pc_R1dc, 0.641);

                // Per-phase values
                double Vph_nl = star ? Vnl / std::sqrt(3.0) : Vnl;
                double Vph_br = star ? Vbr / std::sqrt(3.0) : Vbr;
                // For Delta: line current = sqrt(3)*phase current
                double Iph_nl = star ? Inl : Inl / std::sqrt(3.0);
                double Iph_br = star ? Ibr : Ibr / std::sqrt(3.0);

                // No-load test analysis
                double Pph_nl = Pnl / 3.0;
                // Iron loss resistance: Rc = Vph^2 / P_fe_ph (assuming P_NL ≈ P_fe at no load)
                double Rc = (Pph_nl > 0) ? (Vph_nl * Vph_nl) / Pph_nl : 999.0;
                // Magnetizing reactance: Xm = Vph / Im, where Im = sqrt(Inl^2 - (Vph/Rc)^2)
                double Irc = Vph_nl / Rc;                                // core current
                double Im = std::sqrt(std::max(0.0, Iph_nl * Iph_nl - Irc * Irc));
                double Xm_raw = (Im > 0.001) ? Vph_nl / Im : 999.0;

                // Blocked-rotor test analysis
                double Pph_br = Pbr / 3.0;
                double Zbr = (Iph_br > 0) ? Vph_br / Iph_br : 0.0;
                double Rbr = (Iph_br > 0) ? Pph_br / (Iph_br * Iph_br) : 0.0;  // R1+R2
                double Xbr = std::sqrt(std::max(0.0, Zbr * Zbr - Rbr * Rbr));       // X1+X2
                double R2c = Rbr - R1dc;                // referred rotor resistance
                double X1c = Xbr / 2.0;                // X1 ≈ X2
                double X2c = Xbr / 2.0;
                double Xm_c = Xm_raw - X1c;             // subtract X1 from no-load reactance

                // Guard against negative/unreasonable values
                if (R2c < 0.0)   R2c = 0.0;
                if (Xm_c < 1.0)   Xm_c = 1.0;

                pc_r1 = R1dc;  pc_x1 = X1c;
                pc_r2 = R2c;   pc_x2 = X2c;
                pc_xm = Xm_c;  pc_rc = Rc;

                pc_Result->Clear();
                pc_Result->SelectionColor = Color::FromArgb(0, 210, 0);
                pc_Result->AppendText("Stator R1  = " + pc_r1.ToString("F4") + " Ohm\n");
                pc_Result->AppendText("Stator X1  = " + pc_x1.ToString("F4") + " Ohm\n");
                pc_Result->AppendText("Rotor  R2' = " + pc_r2.ToString("F4") + " Ohm\n");
                pc_Result->AppendText("Rotor  X2' = " + pc_x2.ToString("F4") + " Ohm\n");
                pc_Result->AppendText("Magnet. Xm = " + pc_xm.ToString("F4") + " Ohm\n");
                pc_Result->AppendText("Core   Rc  = " + pc_rc.ToString("F2") + " Ohm\n");

                pc_BtnApply->Enabled = true;
            }
            catch (Exception^ ex) {
                pc_Result->Clear();
                pc_Result->SelectionColor = Color::Red;
                pc_Result->AppendText("Error: " + ex->Message);
            }
        }

        void ParamCalcApply_Click(Object^ sender, EventArgs^ e) {
            txtR1->Text = pc_r1.ToString("F4", CultureInfo::InvariantCulture);
            txtX1->Text = pc_x1.ToString("F4", CultureInfo::InvariantCulture);
            txtR2->Text = pc_r2.ToString("F4", CultureInfo::InvariantCulture);
            txtX2->Text = pc_x2.ToString("F4", CultureInfo::InvariantCulture);
            txtXm->Text = pc_xm.ToString("F4", CultureInfo::InvariantCulture);
            txtRc->Text = pc_rc.ToString("F2", CultureInfo::InvariantCulture);
            lblStatus->Text = "  Parameters applied from test data. Click RUN ANALYSIS.";
        }

        // =========================================================
        //  MAIN ANALYSIS
        // =========================================================
        void Analyze_Click(Object^ sender, EventArgs^ e) {
            try {
                // ── Clear series (but NOT frozen series) ─────────
                chartTorque->Series["Torque (Nm)"]->Points->Clear();
                chartTorque->Series["Load Line"]->Points->Clear();
                chartCurrent->Series["Current (A)"]->Points->Clear();
                chartEff->Series["Efficiency (%)"]->Points->Clear();
                chartEff->Series["Power Factor"]->Points->Clear();
                chartSlip->Series["Slip-Torque"]->Points->Clear();
                chartPout->Series["Eta vs Pout"]->Points->Clear();

                swN->Clear(); swSlip->Clear(); swTq->Clear(); swI->Clear();
                swEff->Clear(); swPF->Clear(); swPout->Clear();

                // ── Read inputs ──────────────────────────────────
                double Vline = Val(txtV);
                double freq = Val(txtFreq);
                int    poles = (int)Val(txtPoles);
                double R1 = Val(txtR1);
                double X1 = Val(txtX1);
                double R2 = Val(txtR2);
                double X2 = Val(txtX2);
                double Xm = Val(txtXm);
                double Rc = Val(txtRc);
                double mechLoss = Val(txtMechLoss);
                double nRated = Val(txtRatedSpeed);
                double TL_ns = SafeVal(txtLoadTorque, 0.0);  // fan load at ns

                bool   isStar = (cbConn->SelectedIndex == 0);
                // Per-phase voltage
                double Vph = isStar ? Vline / std::sqrt(3.0) : Vline;
                double ns = (120.0 * freq) / (double)poles;
                double ws = (2.0 * System::Math::PI * ns) / 60.0;

                // X-axis ticks
                double dynInt = std::round((ns / 6.0) / 50.0) * 50.0;
                if (dynInt < 50.0) dynInt = 50.0;
                chartTorque->ChartAreas["main"]->AxisX->Maximum = ns;
                chartTorque->ChartAreas["main"]->AxisX->Interval = dynInt;
                chartCurrent->ChartAreas["main"]->AxisX->Maximum = ns;
                chartCurrent->ChartAreas["main"]->AxisX->Interval = dynInt;
                chartEff->ChartAreas["main"]->AxisX->Maximum = ns;
                chartEff->ChartAreas["main"]->AxisX->Interval = dynInt;

                // ── Tracking variables ────────────────────────────
                double maxTorque = 0, slipAtMaxT = 0, maxEff = 0, nAtMaxEff = 0;
                double startTorque = 0, startCurrent = 0;
                bool   firstPoint = true;

                double slipTarget = (ns - nRated) / ns;
                double rp_n = 0, rp_Tq = 0, rp_I = 0, rp_pf = 0, rp_eff = 0;
                double rp_pin = 0, rp_pout = 0;
                double rp_pcu1 = 0, rp_pcu2 = 0, rp_pfe = 0, rp_slip = 0;
                double minDiff = 1.0e9;

                // ── Sweep n = 0 → ns  (1 RPM steps) ──────────────
                for (double n = 0.0; n <= ns; n += 1.0) {
                    double slip = (ns - n) / ns;
                    if (slip < 1.0e-7) slip = 1.0e-7;

                    // ─ Equivalent circuit  Zm = Rc || jXm ─────────
                    std::complex<double> Z1(R1, X1);
                    std::complex<double> Zm_rc(Rc, 0.0);
                    std::complex<double> Zm_xm(0.0, Xm);
                    std::complex<double> Zm = (Zm_rc * Zm_xm) / (Zm_rc + Zm_xm);

                    std::complex<double> Vth = (double)Vph * (Zm / (Z1 + Zm));
                    std::complex<double> Zth = (Zm * Z1) / (Zm + Z1);

                    double R2s = R2 / slip;
                    double denom = ws * (std::pow(Zth.real() + R2s, 2)
                        + std::pow(Zth.imag() + X2, 2));
                    if (denom < 1.0e-12) continue;

                    // Electromagnetic torque [Nm]
                    double torque = (3.0 * std::pow(std::abs(Vth), 2) * R2s) / denom;

                    // Stator current and derived quantities
                    std::complex<double> Z2(R2s, X2);
                    std::complex<double> Zin = Z1 + (Zm * Z2) / (Zm + Z2);
                    std::complex<double> I1 = (double)Vph / Zin;
                    std::complex<double> Vm = (double)Vph - I1 * Z1;
                    std::complex<double> I2 = Vm / Z2;
                    std::complex<double> Irc = Vm / std::complex<double>(Rc, 0.0);

                    double Iph = std::abs(I1);
                    double Iline = isStar ? Iph : Iph * std::sqrt(3.0);
                    double pf = std::abs(std::cos(std::arg(I1)));

                    // Power flow
                    double Pin = 3.0 * (double)Vph * Iph * pf;
                    double Pcu1 = 3.0 * Iph * Iph * R1;
                    double Pfe = 3.0 * std::pow(std::abs(Irc), 2) * Rc;
                    double Pgap = 3.0 * std::pow(std::abs(I2), 2) * R2s;
                    double Pcu2 = slip * Pgap;
                    double Pconv = (1.0 - slip) * Pgap;
                    double Pout = Pconv - mechLoss;
                    double eff = (Pin > 1.0 && Pout > 0.0)
                        ? (Pout / Pin) * 100.0 : 0.0;

                    if (firstPoint) { startTorque = torque; startCurrent = Iline; firstPoint = false; }
                    if (torque > maxTorque) { maxTorque = torque; slipAtMaxT = slip; }
                    if (eff > maxEff) { maxEff = eff;       nAtMaxEff = n; }

                    double sd = std::abs(slip - slipTarget);
                    if (sd < minDiff) {
                        minDiff = sd;
                        rp_n = n;       rp_Tq = torque;  rp_I = Iline;
                        rp_pf = pf;      rp_eff = eff;
                        rp_pin = Pin;     rp_pout = Pout;
                        rp_pcu1 = Pcu1;    rp_pcu2 = Pcu2;    rp_pfe = Pfe;
                        rp_slip = slip;
                    }

                    // Store for CSV
                    swN->Add(n);         swSlip->Add(slip);
                    swTq->Add(torque);   swI->Add(Iline);
                    swEff->Add(eff);     swPF->Add(pf);
                    swPout->Add((Pout > 0) ? Pout / 1000.0 : 0.0);

                    // Plot
                    chartTorque->Series["Torque (Nm)"]->Points->AddXY(n, torque);
                    chartCurrent->Series["Current (A)"]->Points->AddXY(n, Iline);
                    chartEff->Series["Efficiency (%)"]->Points->AddXY(n, eff);
                    chartEff->Series["Power Factor"]->Points->AddXY(n, pf);
                    chartSlip->Series["Slip-Torque"]->Points->AddXY(slip, torque);
                    if (Pout > 0.0)
                        chartPout->Series["Eta vs Pout"]->Points->AddXY(Pout / 1000.0, eff);
                }

                // ── Fan/pump load line ─────────────────────────────
                if (TL_ns > 0.1) {
                    for (double n = 0.0; n <= ns; n += 10.0) {
                        double TL = TL_ns * (n / ns) * (n / ns);
                        chartTorque->Series["Load Line"]->Points->AddXY(n, TL);
                    }
                }

                // ── Rescale axes ──────────────────────────────────
                chartTorque->ChartAreas["main"]->RecalculateAxesScale();
                chartCurrent->ChartAreas["main"]->RecalculateAxesScale();
                chartEff->ChartAreas["main"]->RecalculateAxesScale();
                chartSlip->ChartAreas["main"]->RecalculateAxesScale();
                chartPout->ChartAreas["main"]->RecalculateAxesScale();

                // ── Striplines ────────────────────────────────────
                AddRatedStripLine(chartTorque, rp_n);
                AddRatedStripLine(chartCurrent, rp_n);
                AddRatedStripLine(chartEff, rp_n);
                AddSlipStripLine(chartSlip, slipAtMaxT);

                // Rated Pout stripline
                chartPout->ChartAreas["main"]->AxisX->StripLines->Clear();
                StripLine^ slP = gcnew StripLine();
                slP->Interval = 0; slP->IntervalOffset = rp_pout / 1000.0;
                slP->StripWidth = 0.15;
                slP->BackColor = Color::FromArgb(80, 255, 220, 0);
                slP->BorderColor = Color::FromArgb(210, 255, 215, 0);
                slP->BorderWidth = 1; slP->BorderDashStyle = ChartDashStyle::Dash;
                slP->Text = "Rated Pout"; slP->ForeColor = Color::FromArgb(255, 220, 0);
                slP->Font = gcnew System::Drawing::Font("Segoe UI", 9, FontStyle::Bold);
                slP->TextLineAlignment = StringAlignment::Far;
                chartPout->ChartAreas["main"]->AxisX->StripLines->Add(slP);

                // ── Derived quantities ─────────────────────────────
                double speedReg = (rp_n > 0) ? (ns - rp_n) / rp_n * 100.0 : 0.0;
                double torqueRatio = (startTorque > 0.0) ? maxTorque / startTorque : 0.0;
                double totalLoss = rp_pcu1 + rp_pcu2 + rp_pfe + mechLoss;
                String^ ieClass = IEClass(maxEff);

                // ── Protection warnings ───────────────────────────
                String^ warnMsg = "";
                if (rp_I > 0 && startCurrent > 7.5 * rp_I)
                    warnMsg += "High start-current ratio " + (startCurrent / rp_I).ToString("F1") + "x rated  ";
                if (maxTorque < 1.5 * rp_Tq)
                    warnMsg += "Low pull-out margin (T_max/T_rated=" + (maxTorque / rp_Tq).ToString("F2") + ")  ";
                if (rp_eff < 70.0 && rp_eff > 0.0)
                    warnMsg += "Low rated efficiency (" + rp_eff.ToString("F1") + "%)  ";
                if (warnMsg->Length > 0) ShowWarning(warnMsg);
                else HideWarning();

                // ── Report ────────────────────────────────────────
                txtLog->Clear();
                txtLog->SelectionColor = Color::FromArgb(0, 200, 200);
                txtLog->AppendText("====== MOTOR ANALYSIS REPORT ======\n");

                LogHeader("-- GENERAL --------------------------");
                LogLine("Sync Speed (Ns)      : ", ns.ToString("F0") + " RPM", Color::White);
                LogLine("Sync Ang. Speed (ws) : ", ws.ToString("F2") + " rad/s", Color::White);

                LogHeader("-- STARTING CONDITIONS --------------");
                LogLine("Starting Torque      : ", startTorque.ToString("F2") + " Nm", Color::OrangeRed);
                LogLine("Starting Current     : ", startCurrent.ToString("F2") + " A", Color::DeepSkyBlue);

                LogHeader("-- BREAKDOWN (MAX TORQUE) -----------");
                LogLine("Breakdown Torque     : ", maxTorque.ToString("F2") + " Nm", Color::OrangeRed);
                LogLine("Critical Slip        : ", (slipAtMaxT * 100).ToString("F2") + " %", Color::OrangeRed);
                LogLine("T_max / T_start      : ", torqueRatio.ToString("F2"), Color::Yellow);

                LogHeader("-- RATED OPERATING POINT ------------");
                LogLine("Rated Speed (n_r)    : ", rp_n.ToString("F0") + " RPM", Color::White);
                LogLine("Rated Slip (s_r)     : ", (rp_slip * 100).ToString("F3") + " %", Color::White);
                LogLine("Speed Regulation     : ", speedReg.ToString("F2") + " %", Color::Cyan);
                LogLine("Rated Torque         : ", rp_Tq.ToString("F2") + " Nm", Color::OrangeRed);
                LogLine("Rated Line Current   : ", rp_I.ToString("F2") + " A", Color::DeepSkyBlue);
                LogLine("Power Factor         : ", rp_pf.ToString("F4"), Color::Gold);
                LogLine("Input Power  (Pin)   : ", (rp_pin / 1000.0).ToString("F3") + " kW", Color::White);
                LogLine("Output Power (Pout)  : ", (rp_pout / 1000.0).ToString("F3") + " kW", Color::LimeGreen);
                LogLine("Efficiency at rated  : ", rp_eff.ToString("F2") + " %", Color::LimeGreen);
                LogLine("T_max / T_rated      : ", (maxTorque / rp_Tq).ToString("F2"), Color::Yellow);

                LogHeader("-- LOSS BREAKDOWN  @ rated ----------");
                LogLine("Stator Cu Loss P_cu1 : ", rp_pcu1.ToString("F1") + " W", Color::Tomato);
                LogLine("Rotor  Cu Loss P_cu2 : ", rp_pcu2.ToString("F1") + " W", Color::Tomato);
                LogLine("Iron (Core) Loss P_fe: ", rp_pfe.ToString("F1") + " W", Color::Salmon);
                LogLine("Friction+Windage P_fw: ", mechLoss.ToString("F1") + " W", Color::Salmon);
                LogLine("-------------------------------------", "", Color::FromArgb(44, 44, 44));
                LogLine("Total Losses         : ", totalLoss.ToString("F1") + " W", Color::OrangeRed);

                LogHeader("-- PEAK PERFORMANCE -----------------");
                LogLine("Peak Efficiency (Eta): ", maxEff.ToString("F2") + " %", Color::LimeGreen);
                LogLine("  at Speed           : ", nAtMaxEff.ToString("F0") + " RPM", Color::LimeGreen);
                LogLine("IEC Efficiency Class : ", ieClass, Color::Cyan);

                txtLog->SelectionColor = Color::FromArgb(0, 205, 0);
                txtLog->AppendText("\n* STATUS: SIMULATION COMPLETE\n");

                // ── Status bar ────────────────────────────────────
                lblStatus->Text =
                    "  Ns=" + ns.ToString("F0") + " RPM  |  "
                    + "T_rated=" + rp_Tq.ToString("F1") + " Nm  |  "
                    + "T_max=" + maxTorque.ToString("F1") + " Nm  |  "
                    + "Eta=" + rp_eff.ToString("F1") + "%  |  "
                    + "PF=" + rp_pf.ToString("F3") + "  |  "
                    + "Pout=" + (rp_pout / 1000.0).ToString("F2") + " kW  |  "
                    + "SR=" + speedReg.ToString("F2") + "%  |  "
                    + ieClass
                    + "  |  Scroll-wheel=zoom  Drag=zoom  Dbl-click=reset";
            }
            catch (Exception^ ex) {
                MessageBox::Show(ex->Message, "Input Error",
                    MessageBoxButtons::OK, MessageBoxIcon::Warning);
                lblStatus->Text = "  Error — check input values.";
            }
        }
    };
}
