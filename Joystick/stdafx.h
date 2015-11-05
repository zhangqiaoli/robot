// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once

#define WINVER 0x0501

#include <iostream>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// 某些 CString 构造函数将为显式的

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// 从 Windows 头中排除极少使用的资料
#endif

#include <afx.h>
#include <afxwin.h>         // MFC 核心组件和标准组件
#include <afxext.h>         // MFC 扩展
#include <afxdtctl.h>		// MFC 对 Internet Explorer 4 公共控件的支持
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC 对 Windows 公共控件的支持
#endif // _AFX_NO_AFXCMN_SUPPORT

// TODO: 在此处引用程序要求的附加头文件
#include <PicaSoftHead.h>
#include <PicaSoftAssistant.h>
#include <PicaSoftLog.h>

using namespace PicaSoft;
#define PCS_APP_IDENTIFICATION 0x04

#include <PicaSoftBoost.h>

//#define SOAP_DEBUG
//#define TEST NULL
//#define fdebug NULL
//#define DBGLOG(DBGFILE, CMD) CMD
//#define DBGHEX __noop
//#define DBGMSG(DBGFILE, MSG, LEN) GSoapLog(DBGFILE,MSG);
//
//void GSoapLog(LPVOID,LPCTSTR lpszText);