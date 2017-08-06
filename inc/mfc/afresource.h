//
// application framework resource include file - afresource.h
//
// These are the IDs for the resources used by AppFrame dialogs,
// menus and controls. Your application must include this file
// to be able to access the Vixen resources from programs or
// resource files. See MFC Tech Note #TN035 for details on how
// to use multiple resource files.
//
// Note: the resources in this file must be manually maintained.
// You cannot rely on DevStudio to correctly update these.
// Make sure their IDs do not overlap with what DevStudio
// outputs for the calling application
//

#if !defined (IDR_MAINFRAME)
// file types
#define IDR_MAINFRAME					128
#endif

#if !defined (AFRESOURCES)
#define AFRESOURCES

// menus
#define IDM_AFMENU						20000

// accelerators
#define IDA_AFACCEL						20000

// bitmaps
#define IDB_TREEIMAGES                  20107

// dialogs
#define IDD_TREEVIEW                    20101
#define IDD_CAMDIALOG                   20102
#define IDD_KEYDIALOG                   20103
#define IDD_ORIENT                      20104
#define IDD_ENVIRON                     20105


// dialog controls
#define IDC_TREE1                       21000
#define IDC_HITHER_S                    21001
#define IDC_YON_S                       21002
#define IDC_HITHER                      21003
#define IDC_YON                         21004
#define IDC_VANGLE_S                    21005
#define IDC_VANGLE                      21006
#define IDC_ZOOMTOSCENE                 21007
#define IDC_FSPEED                      21008
#define IDC_ZOOMTONODE                  21009
#define IDC_ENABLED                     21011
#define IDC_CUBEMAPVIEWS                21012
#define IDC_KPOINTS                     21013
#define IDC_KVISIBLE                    21014
#define IDC_KTYPE                       21015
#define IDC_ASPECT                      21016
#define IDC_PLAYCAM                     21017
#define IDC_VPLIST                      21018

#define IDC_BUTTON_STORE1               21020
#define IDC_BUTTON_STORE2               21021
#define IDC_BUTTON_STORE3               21022
#define IDC_BUTTON_STORE4               21023
#define IDC_BUTTON_STORE5               21024
#define IDC_BUTTON_STORE6               21025
#define IDC_BUTTON_RECALL1              21026
#define IDC_BUTTON_RECALL2              21027
#define IDC_BUTTON_RECALL3              21028
#define IDC_BUTTON_RECALL4              21029
#define IDC_BUTTON_RECALL5              21030
#define IDC_BUTTON_RECALL6              21031

#define IDC_FSPEED_S                    21040
#define IDC_PITCH                       21041
#define IDC_YAW                         21042
#define	IDC_PLAYANIM					21043

#define IDC_START2                      21201
#define IDC_START1                      21202
#define IDC_END2                        21203
#define IDC_END1                        21204
#define IDC_FOGCOLOR                    21206
#define IDC_BGCOLOR                     21207
#define IDC_LINEAR                      21208
#define IDC_EXPONENTIAL                 21209
#define IDC_EXPONENTIAL2                21210
#define IDC_DENSITY1                    21211
#define IDC_DENSITY2                    21212
#define IDC_AMBIENT                     21214

// menu commands
#define ID_VIEW_SCENEGRAPH              21372
#define ID_VIEW_FULLSCREEN              21373
#define ID_VIEW_DISABLELIGHTING         21374
#define ID_VIEW_CAMERA					21375
#define ID_VIEW_ZOOMALL                 21376
#define ID_VIEW_NAVIGATION				21377
#define ID_VIEW_FRAMERATECOUNTER        21387
#define ID_VIEW_WIREFRAME               21393
#define ID_VIEW_ENVIRON                 21394

#define ID_NAV_FLYER                    21500
#define ID_NAV_DRIVER                   21501
#define ID_NAV_ARCBALL                  21502

#define ID_SCPT_SAVE                    21400
#define ID_SCPT_OPEN                    21402
#define ID_SCPT_RECORD                  21403
#define ID_SCPT_STOP	                21404
#define ID_SCPT_RUN                     21405
#define ID_SCPT_STOPRUN                 21406
#define ID_SCPT_REPLAY                  21407
#define ID_SCPT_FASTER                  21408
#define ID_SCPT_SLOWER                  21409
#define ID_SCPT_REVERSE                 21412

// progress
#define CG_IDD_PROGRESS                 102
#define CG_IDC_PROGDLG_PROGRESS         1003
#define CG_IDC_PROGDLG_STATUS           1005

#endif
