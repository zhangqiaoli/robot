// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���ǳ��õ��������ĵ���Ŀ�ض��İ����ļ�
//

#pragma once

#define WINVER 0x0501

#include <iostream>
#include <tchar.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	// ĳЩ CString ���캯����Ϊ��ʽ��

#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN		// �� Windows ͷ���ų�����ʹ�õ�����
#endif

#include <afx.h>
#include <afxwin.h>         // MFC ��������ͱ�׼���
#include <afxext.h>         // MFC ��չ
#include <afxdtctl.h>		// MFC �� Internet Explorer 4 �����ؼ���֧��
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC �� Windows �����ؼ���֧��
#endif // _AFX_NO_AFXCMN_SUPPORT

// TODO: �ڴ˴����ó���Ҫ��ĸ���ͷ�ļ�
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