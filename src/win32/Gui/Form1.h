#pragma once


namespace NeoStatsControlPanel {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;

	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//import System::ServiceProcess::ServiceControllerStatus;
			//
			//TODO: Add the constructor code here
			//
			notifyIcon1->BalloonTipText = "NeoStats Stopped";
			notifyIcon1->BalloonTipTitle = "Status";
			notifyIcon1->ContextMenuStrip = contextMenuStrip1;
			notifyIcon1->ShowBalloonTip(10);

		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
			Application::Exit();
		}
	private: System::Windows::Forms::NotifyIcon^  notifyIcon1;
	private: System::Windows::Forms::ContextMenuStrip^  contextMenuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  openControlPanelToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  exitToolStripMenuItem;
	private: System::ServiceProcess::ServiceController^  serviceController1;
	private: System::Diagnostics::PerformanceCounter^  performanceCounter1;
	private: System::Windows::Forms::Timer^  PerfUpdate;
	private: System::Windows::Forms::ProgressBar^  PerfCPU;
	private: System::Windows::Forms::Label^  label1;
	private: System::Windows::Forms::Label^  label2;
	private: System::Diagnostics::PerformanceCounter^  performanceCounter2;
	private: System::Windows::Forms::Label^  label3;
	private: System::Windows::Forms::Label^  label4;
	private: System::Windows::Forms::MenuStrip^  menuStrip1;
	private: System::Windows::Forms::ToolStripMenuItem^  serviceToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  startToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  stopToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  debugToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  installToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  unInstallToolStripMenuItem;
	private: System::Windows::Forms::ToolStripSeparator^  toolStripSeparator1;
	private: System::Windows::Forms::ToolStripMenuItem^  quitNeoStatsControlPanelToolStripMenuItem;
	private: System::Windows::Forms::ToolStripMenuItem^  logViewerToolStripMenuItem;



	protected: 
	private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			System::ComponentModel::ComponentResourceManager^  resources = (gcnew System::ComponentModel::ComponentResourceManager(Form1::typeid));
			this->notifyIcon1 = (gcnew System::Windows::Forms::NotifyIcon(this->components));
			this->contextMenuStrip1 = (gcnew System::Windows::Forms::ContextMenuStrip(this->components));
			this->openControlPanelToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->exitToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->serviceController1 = (gcnew System::ServiceProcess::ServiceController());
			this->performanceCounter1 = (gcnew System::Diagnostics::PerformanceCounter());
			this->PerfUpdate = (gcnew System::Windows::Forms::Timer(this->components));
			this->PerfCPU = (gcnew System::Windows::Forms::ProgressBar());
			this->label1 = (gcnew System::Windows::Forms::Label());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->performanceCounter2 = (gcnew System::Diagnostics::PerformanceCounter());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->menuStrip1 = (gcnew System::Windows::Forms::MenuStrip());
			this->serviceToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->startToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->stopToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->debugToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->installToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->unInstallToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->toolStripSeparator1 = (gcnew System::Windows::Forms::ToolStripSeparator());
			this->quitNeoStatsControlPanelToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->logViewerToolStripMenuItem = (gcnew System::Windows::Forms::ToolStripMenuItem());
			this->contextMenuStrip1->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->performanceCounter1))->BeginInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->performanceCounter2))->BeginInit();
			this->menuStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// notifyIcon1
			// 
			this->notifyIcon1->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"notifyIcon1.Icon")));
			this->notifyIcon1->Text = L"NeoStats Control Panel";
			this->notifyIcon1->Visible = true;
			this->notifyIcon1->MouseDoubleClick += gcnew System::Windows::Forms::MouseEventHandler(this, &Form1::notifyIcon1_MouseDoubleClick);
			// 
			// contextMenuStrip1
			// 
			this->contextMenuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->openControlPanelToolStripMenuItem, 
				this->exitToolStripMenuItem});
			this->contextMenuStrip1->Name = L"contextMenuStrip1";
			this->contextMenuStrip1->Size = System::Drawing::Size(179, 48);
			// 
			// openControlPanelToolStripMenuItem
			// 
			this->openControlPanelToolStripMenuItem->Name = L"openControlPanelToolStripMenuItem";
			this->openControlPanelToolStripMenuItem->Size = System::Drawing::Size(178, 22);
			this->openControlPanelToolStripMenuItem->Text = L"&Open Control Panel";
			this->openControlPanelToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::openControlPanelToolStripMenuItem_Click);
			// 
			// exitToolStripMenuItem
			// 
			this->exitToolStripMenuItem->Name = L"exitToolStripMenuItem";
			this->exitToolStripMenuItem->Size = System::Drawing::Size(178, 22);
			this->exitToolStripMenuItem->Text = L"E&xit";
			this->exitToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::exitToolStripMenuItem_Click);
			// 
			// serviceController1
			// 
			this->serviceController1->ServiceName = L"McShield";
			// 
			// performanceCounter1
			// 
			this->performanceCounter1->CategoryName = L"Process";
			this->performanceCounter1->CounterName = L"% Processor Time";
			this->performanceCounter1->InstanceName = L"taskmgr";
			// 
			// PerfUpdate
			// 
			this->PerfUpdate->Interval = 1000;
			this->PerfUpdate->Tick += gcnew System::EventHandler(this, &Form1::PerfUpdate_Tick);
			// 
			// PerfCPU
			// 
			this->PerfCPU->Location = System::Drawing::Point(2, 238);
			this->PerfCPU->Name = L"PerfCPU";
			this->PerfCPU->Size = System::Drawing::Size(283, 23);
			this->PerfCPU->TabIndex = 1;
			// 
			// label1
			// 
			this->label1->AutoSize = true;
			this->label1->Location = System::Drawing::Point(80, 222);
			this->label1->Name = L"label1";
			this->label1->Size = System::Drawing::Size(13, 13);
			this->label1->TabIndex = 2;
			this->label1->Text = L"0";
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(8, 221);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(66, 13);
			this->label2->TabIndex = 3;
			this->label2->Text = L"CPU Usage:";
			// 
			// performanceCounter2
			// 
			this->performanceCounter2->CategoryName = L"Process";
			this->performanceCounter2->CounterName = L"Working Set";
			this->performanceCounter2->InstanceName = L"taskmgr";
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(99, 222);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(81, 13);
			this->label3->TabIndex = 4;
			this->label3->Text = L"Memory Usage:";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(187, 222);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(13, 13);
			this->label4->TabIndex = 5;
			this->label4->Text = L"0";
			// 
			// menuStrip1
			// 
			this->menuStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(2) {this->serviceToolStripMenuItem, 
				this->logViewerToolStripMenuItem});
			this->menuStrip1->Location = System::Drawing::Point(0, 0);
			this->menuStrip1->Name = L"menuStrip1";
			this->menuStrip1->Size = System::Drawing::Size(284, 24);
			this->menuStrip1->TabIndex = 6;
			this->menuStrip1->Text = L"menuStrip1";
			// 
			// serviceToolStripMenuItem
			// 
			this->serviceToolStripMenuItem->DropDownItems->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(7) {this->startToolStripMenuItem, 
				this->stopToolStripMenuItem, this->debugToolStripMenuItem, this->installToolStripMenuItem, this->unInstallToolStripMenuItem, 
				this->toolStripSeparator1, this->quitNeoStatsControlPanelToolStripMenuItem});
			this->serviceToolStripMenuItem->Name = L"serviceToolStripMenuItem";
			this->serviceToolStripMenuItem->Size = System::Drawing::Size(56, 20);
			this->serviceToolStripMenuItem->Text = L"Service";
			// 
			// startToolStripMenuItem
			// 
			this->startToolStripMenuItem->Enabled = false;
			this->startToolStripMenuItem->Name = L"startToolStripMenuItem";
			this->startToolStripMenuItem->Size = System::Drawing::Size(222, 22);
			this->startToolStripMenuItem->Text = L"Start";
			this->startToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::startToolStripMenuItem_Click);
			// 
			// stopToolStripMenuItem
			// 
			this->stopToolStripMenuItem->Enabled = false;
			this->stopToolStripMenuItem->Name = L"stopToolStripMenuItem";
			this->stopToolStripMenuItem->Size = System::Drawing::Size(222, 22);
			this->stopToolStripMenuItem->Text = L"Stop";
			this->stopToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::stopToolStripMenuItem_Click);
			// 
			// debugToolStripMenuItem
			// 
			this->debugToolStripMenuItem->Name = L"debugToolStripMenuItem";
			this->debugToolStripMenuItem->Size = System::Drawing::Size(222, 22);
			this->debugToolStripMenuItem->Text = L"Debug";
			// 
			// installToolStripMenuItem
			// 
			this->installToolStripMenuItem->Enabled = false;
			this->installToolStripMenuItem->Name = L"installToolStripMenuItem";
			this->installToolStripMenuItem->Size = System::Drawing::Size(222, 22);
			this->installToolStripMenuItem->Text = L"Install";
			// 
			// unInstallToolStripMenuItem
			// 
			this->unInstallToolStripMenuItem->Enabled = false;
			this->unInstallToolStripMenuItem->Name = L"unInstallToolStripMenuItem";
			this->unInstallToolStripMenuItem->Size = System::Drawing::Size(222, 22);
			this->unInstallToolStripMenuItem->Text = L"Uninstall";
			// 
			// toolStripSeparator1
			// 
			this->toolStripSeparator1->Name = L"toolStripSeparator1";
			this->toolStripSeparator1->Size = System::Drawing::Size(219, 6);
			// 
			// quitNeoStatsControlPanelToolStripMenuItem
			// 
			this->quitNeoStatsControlPanelToolStripMenuItem->Name = L"quitNeoStatsControlPanelToolStripMenuItem";
			this->quitNeoStatsControlPanelToolStripMenuItem->Size = System::Drawing::Size(222, 22);
			this->quitNeoStatsControlPanelToolStripMenuItem->Text = L"Quit NeoStats Control Panel";
			this->quitNeoStatsControlPanelToolStripMenuItem->Click += gcnew System::EventHandler(this, &Form1::quitNeoStatsControlPanelToolStripMenuItem_Click);
			// 
			// logViewerToolStripMenuItem
			// 
			this->logViewerToolStripMenuItem->Name = L"logViewerToolStripMenuItem";
			this->logViewerToolStripMenuItem->Size = System::Drawing::Size(77, 20);
			this->logViewerToolStripMenuItem->Text = L"Log Viewer";
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 264);
			this->Controls->Add(this->menuStrip1);
			this->Controls->Add(this->label4);
			this->Controls->Add(this->label3);
			this->Controls->Add(this->label2);
			this->Controls->Add(this->PerfCPU);
			this->Controls->Add(this->label1);
			this->Icon = (cli::safe_cast<System::Drawing::Icon^  >(resources->GetObject(L"$this.Icon")));
			this->MainMenuStrip = this->menuStrip1;
			this->MinimizeBox = false;
			this->Name = L"Form1";
			this->Text = L"NeoStats Control Panel";
			this->contextMenuStrip1->ResumeLayout(false);
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->performanceCounter1))->EndInit();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->performanceCounter2))->EndInit();
			this->menuStrip1->ResumeLayout(false);
			this->menuStrip1->PerformLayout();
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void openControlPanelToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
				 if (this->Visible == true) {
					 this->Hide();
					 this->openControlPanelToolStripMenuItem->Text = L"&Open Control Panel";
					 this->PerfUpdate->Enabled = false;
				 } else {
					 this->openControlPanelToolStripMenuItem->Text = L"&Close Control Panel";
					 this->Show();
					 this->PerfUpdate->Enabled = true;
				 }
	}
	private: System::Void exitToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
			this->Close();
			Application::Exit();
			 };
private: System::Void notifyIcon1_MouseDoubleClick(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) {
		openControlPanelToolStripMenuItem_Click(sender, e);
		 }
private: System::Void PerfUpdate_Tick(System::Object^  sender, System::EventArgs^  e) {
			try {
				this->PerfCPU->Value = this->performanceCounter1->NextValue();
			} catch(...) {
				this->PerfCPU->Value = 0;
			}
			try {
				this->label4->Text = (this->performanceCounter2->NextValue()/1024/1024).ToString("N01") + "Mb";
			} catch(...) {
				this->label4->Text = "0";
			}
			this->label1->Text= this->PerfCPU->Value.ToString();
			switch (serviceController1->Status) {
				case System::ServiceProcess::ServiceControllerStatus::Running:
						notifyIcon1->BalloonTipText = "NeoStats Running";
						this->PerfUpdate->Enabled = true;
						this->startToolStripMenuItem->Enabled = false;
						this->stopToolStripMenuItem->Enabled = true;
						this->debugToolStripMenuItem->Enabled = false;
						this->installToolStripMenuItem->Enabled = false;
						this->unInstallToolStripMenuItem->Enabled = false;
						break;
				case System::ServiceProcess::ServiceControllerStatus::StartPending:
						notifyIcon1->BalloonTipText = "NeoStats Starting";
						this->PerfUpdate->Enabled = true;
						this->startToolStripMenuItem->Enabled = false;
						this->stopToolStripMenuItem->Enabled = true;
						this->debugToolStripMenuItem->Enabled = false;
						this->installToolStripMenuItem->Enabled = false;
						this->unInstallToolStripMenuItem->Enabled = false;
						break;
				default:
						notifyIcon1->BalloonTipText = "NeoStats Stopped";
						this->startToolStripMenuItem->Enabled = true;
						this->stopToolStripMenuItem->Enabled = false;
						this->debugToolStripMenuItem->Enabled = true;
						this->installToolStripMenuItem->Enabled = false;
						this->unInstallToolStripMenuItem->Enabled = true;
						this->PerfUpdate->Enabled = false;
			}
		 }
	

private: System::Void quitNeoStatsControlPanelToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
			exitToolStripMenuItem_Click(sender, e);
		 }
private: System::Void stopToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
			 try {		
			 serviceController1->Stop();
			 } catch(InvalidOperationException^ e ) {
				 MessageBox::Show(e->Message + ":\r\n" + e->InnerException->Message, "Error Stopping Service");	 
			 }
		}
private: System::Void startToolStripMenuItem_Click(System::Object^  sender, System::EventArgs^  e) {
			 try {		
				serviceController1->Start();
			 } catch(InvalidOperationException^ e ) {
				 MessageBox::Show(e->Message + ":\r\n" + e->InnerException->Message, "Error Starting Service");				 
			 }
		 }
};
}

