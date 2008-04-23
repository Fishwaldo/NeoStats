// NeoStats Control Panel.cpp : main project file.

#include "stdafx.h"
#include "Form1.h"

using namespace NeoStatsControlPanel;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	Form1 ^ NeoStatsGui;
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	NeoStatsGui = gcnew Form1();
	NeoStatsGui->Hide();

	Application::Run();
	//	Application::Run(gcnew Form1());
	return 0;
}
