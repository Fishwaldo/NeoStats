#pragma once
#include "stdafx.h"
#include <time.h>
#include <sys/stat.h>
#include <stdio.h>
#include "windows.h"

using namespace System;
using namespace System::ComponentModel;
using namespace System::Collections;
using namespace System::Windows::Forms;
using namespace System::Data;
using namespace System::Drawing;
using namespace System::IO;


namespace NeoStatsControlPanel {

	/// <summary>
	/// Summary for LogWindow
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>
	public ref class LogWindow : public System::Windows::Forms::Form
	{
	public:
		LogWindow(void)
		{
			InitializeComponent();
			//
			//TODO: Add the constructor code here
			//
		}
		void SetLogFile(String^ file)
		{

			this->fileSystemWatcher1->Path = Path::GetDirectoryName(file);
			this->fileSystemWatcher1->Filter = Path::GetFileName(file);
			this->din = gcnew FileStream(file, FileMode::Open, FileAccess::Read, FileShare::ReadWrite);
			this->din2 = gcnew StreamReader(this->din);
			String^ str;
			while ((str = this->din2->ReadLine()) != nullptr) 
			{
		        if (this->LogRT->TextLength > 0) 
					this->LogRT->Text = this->LogRT->Text + "\n" + str;
				else 
					this->LogRT->Text = str;
			}
			
		}




	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~LogWindow()
		{
			if (components)
			{
				delete components;
			}
		}
	private: System::Windows::Forms::RichTextBox^  LogRT;
	private: System::IO::FileSystemWatcher^  fileSystemWatcher1;
	private: FileStream^ din;
	private: StreamReader ^din2;
	protected: 

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
			this->LogRT = (gcnew System::Windows::Forms::RichTextBox());
			this->fileSystemWatcher1 = (gcnew System::IO::FileSystemWatcher());
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fileSystemWatcher1))->BeginInit();
			this->SuspendLayout();
			// 
			// LogRT
			// 
			this->LogRT->Location = System::Drawing::Point(12, 12);
			this->LogRT->Name = L"LogRT";
			this->LogRT->ReadOnly = true;
			this->LogRT->Size = System::Drawing::Size(260, 240);
			this->LogRT->TabIndex = 0;
			this->LogRT->Text = L"";
			// 
			// fileSystemWatcher1
			// 
			this->fileSystemWatcher1->EnableRaisingEvents = true;
			this->fileSystemWatcher1->NotifyFilter = System::IO::NotifyFilters::LastWrite;
			this->fileSystemWatcher1->SynchronizingObject = this;
			this->fileSystemWatcher1->Changed += gcnew System::IO::FileSystemEventHandler(this, &LogWindow::fileSystemWatcher1_Changed);
			// 
			// LogWindow
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(284, 264);
			this->Controls->Add(this->LogRT);
			this->Name = L"LogWindow";
			this->Text = L"LogWindow";
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->fileSystemWatcher1))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
	private: System::Void fileSystemWatcher1_Changed(System::Object^  sender, System::IO::FileSystemEventArgs^  e) {
			int i;
			Char b;
			while ((i = this->din2->Read()) != -1) 
			{
				b = (char)i;
				this->LogRT->AppendText(b.ToString(i));
			}
			this->LogRT->ScrollToCaret();
			 }

	};
}
