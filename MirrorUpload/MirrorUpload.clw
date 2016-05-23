; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CMirrorUploadDlg
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "MirrorUpload.h"

ClassCount=4
Class1=CMirrorUploadApp
Class2=CMirrorUploadDlg
Class3=CAboutDlg

ResourceCount=4
Resource1=IDD_ABOUTBOX
Resource2=IDR_MAINFRAME
Resource3=IDD_MIRRORUPLOAD_DIALOG
Class4=CEditProfile
Resource4=IDD_EDIT_PROFILE

[CLS:CMirrorUploadApp]
Type=0
HeaderFile=MirrorUpload.h
ImplementationFile=MirrorUpload.cpp
Filter=N

[CLS:CMirrorUploadDlg]
Type=0
HeaderFile=MirrorUploadDlg.h
ImplementationFile=MirrorUploadDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=IDC_BUTTON7

[CLS:CAboutDlg]
Type=0
HeaderFile=MirrorUploadDlg.h
ImplementationFile=MirrorUploadDlg.cpp
Filter=D

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=4
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889

[DLG:IDD_MIRRORUPLOAD_DIALOG]
Type=1
Class=CMirrorUploadDlg
ControlCount=7
Control1=IDC_LIST1,SysListView32,1350631425
Control2=IDC_BUTTON1,button,1342242816
Control3=IDC_BUTTON2,button,1342242816
Control4=IDC_BUTTON3,button,1342242816
Control5=IDC_BUTTON7,button,1342242816
Control6=IDC_STATIC,static,1342308352
Control7=IDC_BUTTON8,button,1342242816

[DLG:IDD_EDIT_PROFILE]
Type=1
Class=CEditProfile
ControlCount=20
Control1=IDC_EDIT4,edit,1350631552
Control2=IDC_EDIT8,edit,1350631552
Control3=IDC_BUTTON2,button,1342242816
Control4=IDC_EDIT1,edit,1350631552
Control5=IDC_BUTTON1,button,1342242816
Control6=IDC_EDIT2,edit,1350631552
Control7=IDC_EDIT3,edit,1350631552
Control8=IDC_EDIT5,edit,1350631552
Control9=IDC_EDIT6,edit,1350631584
Control10=IDC_EDIT7,edit,1350631584
Control11=IDOK,button,1342242817
Control12=IDCANCEL,button,1342242816
Control13=IDC_STATIC,static,1342308352
Control14=IDC_STATIC,static,1342308352
Control15=IDC_STATIC,static,1342308352
Control16=IDC_STATIC,static,1342308352
Control17=IDC_STATIC,static,1342308352
Control18=IDC_STATIC,static,1342308352
Control19=IDC_STATIC,static,1342308352
Control20=IDC_STATIC,static,1342308352

[CLS:CEditProfile]
Type=0
HeaderFile=EditProfile.h
ImplementationFile=EditProfile.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=IDC_EDIT8

