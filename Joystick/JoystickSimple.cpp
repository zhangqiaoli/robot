#include "stdafx.h"
#include <stdio.h>
#include <dinput.h>
#include "JoystickMaster.h"
#include "JoystickClient.h"
#pragma comment(lib,"dxerr.lib ")
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#pragma warning( disable : 4996 ) // disable deprecated warning 
#include <strsafe.h>
#pragma warning( default : 4996 )
#include "resource.h"
#define SAMPLE_BUFFER_SIZE 1024
extern "C" WINBASEAPI HWND WINAPI GetConsoleWindow();

LPDIRECTINPUT8       g_pDI = NULL;
LPDIRECTINPUTDEVICE8 g_pJoystick = NULL;
HRESULT ReadBufferedData_js();
void Process_DeviceStatus(LPDIJOYSTATE2 joystick);

BOOL IsInstanceRuning()
{
	PICASOFT_DEBUG_TRACK;
	//return g_bServiceRuning[m_nIndex];
	TSTLString sMutexName;
	sMutexName = _T("Global\\");
	sMutexName += "robot_joystickmaster";
	sMutexName += _T("_RUNING");

	HANDLE hMutex = CreateMutex(NULL, FALSE, sMutexName.data());//Open??????

	if (!hMutex || GetLastError() == ERROR_ALREADY_EXISTS)//已经运行了
	{
		if (hMutex)
		{
			ReleaseMutex(hMutex);
			CloseHandle(hMutex);
		}
		return TRUE;
	}
	else return FALSE;
}

BOOL SetInstanceRuning(BOOL bRuning)
{
	PICASOFT_DEBUG_TRACK;
	HANDLE hMutex = NULL;
	if (bRuning != IsInstanceRuning())
	{
		//g_bServiceRuning[m_nIndex]=bRuning;
		if (bRuning)
		{
			TSTLString sMutexName;
			sMutexName = _T("Global\\");
			sMutexName += "robot_joystickmaster";
			sMutexName += _T("_RUNING");
			hMutex = CreateMutex(NULL, FALSE, sMutexName.data());
			if (!hMutex) return FALSE;
		}
		else
		{
			if (!hMutex) return FALSE;
			if (hMutex)
			{
				ReleaseMutex(hMutex);
				CloseHandle(hMutex);
			}
		}
	}
	return TRUE;
}

int main()
{
	JoystickMaster* client = JoystickMaster::instance();
	if (IsInstanceRuning())
	{
		PLOG(ELL_INFORMATION, _T("Joystick has already run..."));
		return 0;
	}
	else
	{
		SetInstanceRuning(TRUE);
	}

	PLOG(ELL_INFORMATION, _T("Joystick start..."));
	client->ReloadSystemParameter();

	/////////////////////////////////////////////////////////////////////////////////////////////
	//创建事件，为自动型（使用完自动置为无信号状态），初始化为无信号状态
	CPicaSoftEvent evtJoystick(FALSE, FALSE);

	/////////////////////////////////////////////////////////////////////////////////////////////
	//创建DirectInput8对象
	HRESULT hr;
	if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
		IID_IDirectInput8, (VOID**)&g_pDI, NULL)))
	{
	}

	DIPROPDWORD dipdw;

	//创建DirectInput8设备（鼠标），一下过程和键盘设置相同，不再注释
	if (FAILED(hr = g_pDI->CreateDevice(GUID_Joystick, &g_pJoystick, NULL)))
	{
		PLOG(ELL_INFORMATION, _T("CreateDevice failed,Joystick exit!"));
		return hr;
	}
	if (FAILED(hr = g_pJoystick->SetDataFormat(&c_dfDIJoystick2)))
	{
		PLOG(ELL_INFORMATION, _T("SetDataFormat failed,Joystick exit!"));
		return hr;
	}
	hr = g_pJoystick->SetCooperativeLevel(GetConsoleWindow(), DISCL_EXCLUSIVE |
		DISCL_FOREGROUND);

	dipdw.diph.dwSize = sizeof(DIPROPDWORD);
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
	dipdw.diph.dwObj = 0;
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.dwData = SAMPLE_BUFFER_SIZE;

	if (FAILED(hr = g_pJoystick->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
	{
		PLOG(ELL_INFORMATION, _T("SetProperty failed,Joystick exit!"));
		return hr;
	}
	JoystickClientPtr spJSClient = JoystickClientPtr(new JoystickClient("joystick"));
	client->m_spJoystick = spJSClient;
	// zql add
	//JoystickClient* client = JoystickClient::instance();
	client->m_tcpRobotAgent.m_sMonitorAddress = g_Setting.sServerIP;
	client->m_tcpRobotAgent.m_nMonitorPort = g_Setting.nPort;
	client->m_tcpRobotAgent.Start();
	client->Start();
	// zql add end

	g_pJoystick->SetEventNotification(evtJoystick);
	g_pJoystick->Acquire();
	//////////////////////////////////////////////////////////////////////////////////////

	//创建DirectInput8设备(joystick)
	//hr = g_pDI->CreateDevice(pdidInstance->guidInstance, &g_pJoystick, NULL);

	//////////////////////////////////////////////////////////////////////////////////////
	//主线程
	DWORD dwResult = 0;
	while (1)
	{
		if (evtJoystick.Lock(1000))
		{
			ReadBufferedData_js();
		}

	}
	//////////////////////////////////////////////////////////////////////////////////////
}

HRESULT ReadBufferedData_js()
{
	PLOG(ELL_DEBUG, "ReadBufferedData_js start...");
	TCHAR strText[512] = { 0 };
	HRESULT            hr;

	if (NULL == g_pJoystick)
		return S_OK;
	DIJOYSTATE2 js;
	hr = g_pJoystick->GetDeviceState(sizeof(DIJOYSTATE2), &js);
	if (hr != DI_OK)
	{
		hr = g_pJoystick->Acquire();
		while (hr == DIERR_INPUTLOST)
			hr = g_pJoystick->Acquire();

		// Update the dialog text
		if (hr == DIERR_OTHERAPPHASPRIO ||
			hr == DIERR_NOTACQUIRED)
		{
		}
		return S_OK;
	}

	if (FAILED(hr))
		return hr;
	StringCchPrintf(strText, 512, TEXT("%ld"), js.lX);
	StringCchPrintf(strText, 512, TEXT("%ld"), js.lY);
	StringCchPrintf(strText, 512, TEXT("%ld"), js.lZ);
	StringCchPrintf(strText, 512, TEXT("%ld"), js.lRx);
	StringCchPrintf(strText, 512, TEXT("%ld"), js.lRy);
	StringCchPrintf(strText, 512, TEXT("%ld"), js.lRz);
	
	// Slider controls
	StringCchPrintf(strText, 512, TEXT("%ld"), js.rglSlider[0]);
	StringCchPrintf(strText, 512, TEXT("%ld"), js.rglSlider[1]);
	
	// Points of view
	StringCchPrintf(strText, 512, TEXT("%ld"), js.rgdwPOV[0]);
	// zql add
	Process_DeviceStatus(&js);
	// zql add end
	StringCchPrintf(strText, 512, TEXT("%ld"), js.rgdwPOV[1]);
	StringCchPrintf(strText, 512, TEXT("%ld"), js.rgdwPOV[2]);
	StringCchPrintf(strText, 512, TEXT("%ld"), js.rgdwPOV[3]);
	
	
	// Fill up text with which buttons are pressed
	StringCchCopy(strText, 512, TEXT(""));
	for (int i = 0; i < 128; i++)
	{
		if (js.rgbButtons[i] & 0x80)
		{
			TCHAR sz[128];
			StringCchPrintf(sz, 128, TEXT("%02d "), i);
			if (i == 3)
			{
				PLOG(ELL_DEBUG, "gears change");
			}
			StringCchCat(strText, 512, sz);
		}
	}
	PLOG(ELL_DEBUG, "ReadBufferedData_js end");
	return S_OK;
}

void Process_DeviceStatus(LPDIJOYSTATE2 joystick)
{
	PLOG(ELL_DEBUG, "Process_DeviceStatus start...");
	DWORD loc = joystick->rgdwPOV[0];
	CString sOperation = "";
	if (loc == -1)
	{
		sOperation = "stop";
	}
	else if (loc == 0)
	{
		sOperation = "up";
	}
	else if (loc == 9000)
	{
		sOperation = "spin_right";
	}
	else if (loc == 18000)
	{
		sOperation = "down";
	}
	else if (loc == 27000)
	{
		sOperation = "spin_left";
	}
	int speed = 0;
	JoystickMaster* client = JoystickMaster::instance();
	for (int i = 0; i < 128; i++)
	{
		if (joystick->rgbButtons[i] & 0x80)
		{
			if (i == 3)
			{
				speed = client->m_spJoystick->GetJoystickSpeed();
				client->m_spJoystick->SetJoystickSpeed(speed % 3 + 1);
				PLOG(ELL_DEBUG, "gears change,current gear %d change to %d", (speed - 1) % 3 + 1, speed % 3 + 1);
			}
		}
	}
	//g_JoystickStatus.sDirect = sOperation;
	client->m_spJoystick->SetJoystickStatus(sOperation);
	client->m_evtChange.SetEvent();
	PLOG(ELL_DEBUG, "Process_DeviceStatus end");
}


