; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=PSDemoView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "afresource.h"
LastPage=0

ClassCount=7
Class1=CCameraDlg
Class2=PSDemoView
Class3=EnvironDlg
Class4=PSMFCFrame
Class5=PSMFCView
Class6=CProgressDlg
Class7=CTreeDlg

ResourceCount=7
Resource1=IDD_KEYDIALOG
Resource2=IDD_ORIENT
Resource3=IDM_AFMENU
Resource4="IDR_AFACCEL"
Resource5=IDD_CAMDIALOG
Resource6=IDD_TREEVIEW
Resource7=IDD_ENVIRON

[CLS:CCameraDlg]
Type=0
BaseClass=CDialog
HeaderFile=\psm\inc\mfc\CameraDlg.h
ImplementationFile=CameraDlg.cpp
LastObject=ID_NAV_ARCBALL
Filter=D
VirtualFilter=dWC

[CLS:PSDemoView]
Type=0
BaseClass=PSMFCView
HeaderFile=\psm\inc\mfc\demoview.h
ImplementationFile=demoview.cpp
Filter=C
VirtualFilter=VWC
LastObject=ID_NAV_DRIVER

[CLS:EnvironDlg]
Type=0
BaseClass=CDialog
HeaderFile=\psm\inc\mfc\EnvironDlg.h
ImplementationFile=EnvironDlg.cpp

[CLS:PSMFCFrame]
Type=0
BaseClass=CFrameWnd
HeaderFile=\psm\inc\mfc\mfcframe.h
ImplementationFile=mfcframe.cpp

[CLS:PSMFCView]
Type=0
BaseClass=CView
HeaderFile=\psm\inc\mfc\mfcview.h
ImplementationFile=mfcview.cpp

[CLS:CProgressDlg]
Type=0
BaseClass=CDialog
HeaderFile=\psm\inc\mfc\ProgDlg.h
ImplementationFile=ProgDlg.cpp

[CLS:CTreeDlg]
Type=0
BaseClass=CDialog
HeaderFile=\psm\inc\mfc\TreeDlg.h
ImplementationFile=TreeDlg.cpp

[DLG:IDD_CAMDIALOG]
Type=1
Class=CCameraDlg
ControlCount=31
Control1=IDC_STATIC,static,1342308866
Control2=IDC_HITHER_S,msctls_trackbar32,1342242821
Control3=IDC_YON_S,msctls_trackbar32,1342242821
Control4=IDC_HITHER,edit,1350631552
Control5=IDC_YON,edit,1350631552
Control6=IDC_STATIC,static,1342308866
Control7=IDC_VANGLE_S,msctls_trackbar32,1342242821
Control8=IDC_VANGLE,edit,1350631552
Control9=IDC_ZOOMTOSCENE,button,1342242816
Control10=IDC_STATIC,static,1342308866
Control11=IDC_FSPEED_S,msctls_trackbar32,1342242821
Control12=IDC_STATIC,static,1342308866
Control13=IDC_STATIC,static,1342308866
Control14=IDC_STATIC,static,1342308352
Control15=IDC_BUTTON_STORE1,button,1342242816
Control16=IDC_BUTTON_STORE2,button,1342242816
Control17=IDC_BUTTON_STORE3,button,1342242816
Control18=IDC_BUTTON_STORE4,button,1342242816
Control19=IDC_BUTTON_STORE5,button,1342242816
Control20=IDC_BUTTON_STORE6,button,1342242816
Control21=IDC_FSPEED,edit,1350631552
Control22=IDC_BUTTON_RECALL1,button,1342242816
Control23=IDC_BUTTON_RECALL2,button,1342242816
Control24=IDC_BUTTON_RECALL3,button,1342242816
Control25=IDC_BUTTON_RECALL4,button,1342242816
Control26=IDC_BUTTON_RECALL5,button,1342242816
Control27=IDC_BUTTON_RECALL6,button,1342242816
Control28=IDC_STATIC,button,1342177287
Control29=IDC_STATIC,static,1342308866
Control30=IDC_ASPECT,edit,1350631552
Control31=IDC_CUBEMAPVIEWS,button,1342242816

[DLG:IDD_ENVIRON]
Type=1
Class=EnvironDlg
ControlCount=21
Control1=IDOK,button,1342242817
Control2=IDC_STATIC,button,1342308359
Control3=IDC_START2,msctls_trackbar32,1342242821
Control4=IDC_STATIC,static,1342177282
Control5=IDC_START1,edit,1350631552
Control6=IDC_END2,msctls_trackbar32,1342242821
Control7=IDC_STATIC,static,1342177282
Control8=IDC_END1,edit,1350631552
Control9=IDC_FOGCOLOR,button,1342242816
Control10=IDC_STATIC,static,1342177280
Control11=IDC_BGCOLOR,button,1342242816
Control12=IDC_STATIC,static,1342308352
Control13=IDC_STATIC,static,1342308352
Control14=IDC_LINEAR,button,1342308361
Control15=IDC_EXPONENTIAL,button,1342177289
Control16=IDC_EXPONENTIAL2,button,1342177289
Control17=IDC_STATIC,static,1342308352
Control18=IDC_DENSITY1,edit,1350631552
Control19=IDC_DENSITY2,msctls_trackbar32,1342242821
Control20=IDC_STATIC,static,1342308352
Control21=IDC_AMBIENT,button,1342242816

[DLG:CG_IDD_PROGRESS]
Type=1
Class=CProgressDlg

[DLG:IDD_TREEVIEW]
Type=1
Class=CTreeDlg
ControlCount=4
Control1=IDC_TREE1,SysTreeView32,1350631431
Control2=IDC_ZOOMTONODE,button,1476460544
Control3=IDC_ENABLED,button,1342242819
Control4=IDC_PLAYANIM,button,1476460544

[MNU:IDM_AFMENU]
Type=1
Class=?
Command1=ID_VIEW_SCENEGRAPH
Command2=ID_VIEW_CAMERA
Command3=ID_VIEW_ENVIRON
Command4=ID_NAV_FLYER
Command5=ID_NAV_ARCBALL
Command6=ID_NAV_DRIVER
Command7=ID_VIEW_FRAMERATECOUNTER
Command8=ID_VIEW_FULLSCREEN
Command9=ID_VIEW_ZOOMALL
Command10=ID_VIEW_WIREFRAME
Command11=ID_VIEW_DISABLELIGHTING
Command12=ID_SCPT_OPEN
Command13=ID_SCPT_RECORD
Command14=ID_SCPT_RUN
Command15=ID_SCPT_STOP
Command16=ID_SCPT_REPLAY
Command17=ID_SCPT_REVERSE
Command18=ID_SCPT_FASTER
Command19=ID_SCPT_SLOWER
CommandCount=19

[ACL:"IDR_AFACCEL"]
Type=1
Class=?
Command1=ID_VIEW_ZOOMALL
Command2=ID_SCPT_FASTER
Command3=ID_SCPT_SLOWER
Command4=ID_VIEW_WIREFRAME
CommandCount=4

[DLG:IDD_KEYDIALOG]
Type=1
Class=?
ControlCount=6
Control1=IDC_STATIC,static,1342308864
Control2=IDC_KPOINTS,edit,1350633600
Control3=IDC_STATIC,static,1342308864
Control4=IDC_KTYPE,edit,1350633600
Control5=IDC_KVISIBLE,button,1342242819
Control6=IDC_PLAYCAM,button,1342242816

[DLG:IDD_ORIENT]
Type=1
Class=?
ControlCount=7
Control1=IDOK,button,1342242817
Control2=IDC_PITCH,msctls_trackbar32,1342242840
Control3=IDC_YAW,msctls_trackbar32,1342242840
Control4=IDC_STATIC,static,1342308352
Control5=IDC_STATIC,static,1342308352
Control6=IDC_STATIC,static,1342308352
Control7=IDC_STATIC,static,1342308352

