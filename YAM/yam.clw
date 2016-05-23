; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CChatWindow
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "yam.h"

ClassCount=3
Class1=CYamApp
Class2=CYamDlg

ResourceCount=6
Resource2=IDR_SYSTRAY_MENU
Resource3=IDD_yam_DIALOG
Resource4=IDD_CHATWINDOW_DIALOG
Resource1=IDR_MAINFRAME
Resource5=IDD_yam_DIALOG (English (U.S.))
Class3=CChatWindow
Resource6=IDR_SYSTRAY_MENU (English (U.S.))

[CLS:CYamApp]
Type=0
HeaderFile=yam.h
ImplementationFile=yam.cpp
Filter=N

[CLS:CYamDlg]
Type=0
HeaderFile=yamDlg.h
ImplementationFile=yamDlg.cpp
Filter=D
LastObject=IDCANCEL
BaseClass=CResizingDialog
VirtualFilter=dWC



[DLG:IDD_yam_DIALOG]
Type=1
Class=CYamDlg
ControlCount=1
Control1=IDCANCEL,button,1342242816

[MNU:IDR_SYSTRAY_MENU]
Type=1
Command1=IDC_ST_RESTORE
Command2=IDC_ST_EXIT
CommandCount=2

[DLG:IDD_yam_DIALOG (English (U.S.))]
Type=1
Class=CYamDlg
ControlCount=2
Control1=IDC_STATUS,static,1342308352
Control2=IDC_BUTTON1,button,1342242816

[MNU:IDR_SYSTRAY_MENU (English (U.S.))]
Type=1
Class=?
Command1=IDC_ST_RESTORE
Command2=IDC_ST_EXIT
CommandCount=2

[DLG:IDD_CHATWINDOW_DIALOG]
Type=1
Class=CChatWindow
ControlCount=1
Control1=IDC_STATIC,button,1342177287

[CLS:CChatWindow]
Type=0
HeaderFile=ChatWindow.h
ImplementationFile=ChatWindow.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC

