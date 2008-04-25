#pragma once
#include "LogWindow.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;


namespace NeoStatsControlPanel {

	/// <summary>
	/// Summary for LogViewer
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class LogViewer : public System::Windows::Forms::Form
	{
	public:
		LogViewer(void)
		{
			InitializeComponent();
			DirectoryInfo^ di = gcnew DirectoryInfo( "c:\\windows\\temp\\" );
			// Get a reference to each file in that directory.
			array<FileInfo^>^fiArr = di->GetFiles();

			// Display the names of the files.
			toolStripFileList->Items->AddRange(di->GetFiles("*.log"));

			//
			//TODO: Add the constructor code here
			//
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~LogViewer()
		{
			if (components)
			{
				delete components;
			}
//			this->LWS->Values();
			for each(LogWindow^ mywin in this->LWS->Values) {
					mywin->Close();
			}
		
		}
	private: System::Windows::Forms::ToolStrip^  toolStrip1;
	private: System::Windows::Forms::ToolStripComboBox^  toolStripFileList;
	private: Hashtable^ LWS;
	protected: 


	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>
		System::ComponentModel::Container ^components;

#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->toolStrip1 = (gcnew System::Windows::Forms::ToolStrip());
			this->toolStripFileList = (gcnew System::Windows::Forms::ToolStripComboBox());
			this->toolStrip1->SuspendLayout();
			this->SuspendLayout();
			// 
			// toolStrip1
			// 
			this->toolStrip1->Items->AddRange(gcnew cli::array< System::Windows::Forms::ToolStripItem^  >(1) {this->toolStripFileList});
			this->toolStrip1->Location = System::Drawing::Point(0, 0);
			this->toolStrip1->Name = L"toolStrip1";
			this->toolStrip1->Size = System::Drawing::Size(610, 25);
			this->toolStrip1->TabIndex = 2;
			this->toolStrip1->Text = L"toolStrip1";
			// 
			// toolStripFileList
			// 
			this->toolStripFileList->DropDownStyle = System::Windows::Forms::ComboBoxStyle::DropDownList;
			this->toolStripFileList->Name = L"toolStripFileList";
			this->toolStripFileList->Size = System::Drawing::Size(75, 25);
			this->toolStripFileList->SelectedIndexChanged += gcnew System::EventHandler(this, &LogViewer::toolStripFileList_SelectedIndexChanged);
			// 
			// LogViewer
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(610, 470);
			this->Controls->Add(this->toolStrip1);
			this->IsMdiContainer = true;
			this->Name = L"LogViewer";
			this->Text = L"LogViewer";
			this->toolStrip1->ResumeLayout(false);
			this->toolStrip1->PerformLayout();
			this->LWS = gcnew Hashtable;
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion
	private: System::Void toolStripFileList_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
				 if (!this->LWS[toolStripFileList->SelectedItem]) {
					 LogWindow^ newwin = gcnew LogWindow();
					 this->LWS->Add(toolStripFileList->SelectedItem, newwin);
					 newwin->Tag = toolStripFileList->SelectedItem;
					 newwin->Text = toolStripFileList->SelectedItem->ToString();
					 newwin->MdiParent = this;
					 newwin->SetLogFile("c:\\windows\\temp\\" + toolStripFileList->SelectedItem->ToString());
					 newwin->Show();
				 } else {
					 LogWindow^ newwin = (LogWindow^ )this->LWS[toolStripFileList->SelectedItem];
					 newwin->BringToFront();
				 }
			 }
	};
}
