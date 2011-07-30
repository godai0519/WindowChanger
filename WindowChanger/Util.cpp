#include <Windows.h>
#include <string>
#include <boost/format.hpp>
#include "Util.hpp"

input::device_list::device_list()
{
	device_list_ = NULL;
}
input::device_list::~device_list()
{
	if(device_list_ != NULL) delete[] device_list_;
}
UINT input::device_list::operator() (RAWINPUTDEVICELIST *list,size_t &size)
{
	if(list != NULL)
	{
		for(unsigned int i=0;i<device_num_ && i<size;++i)
			*(list+i) = device_list_[i];
	}
	return size = device_num_;
}
int input::device_list::reset()
{
	if(device_list_ != NULL)
	{
		delete[] device_list_;
		device_list_ = NULL;
	}

	UINT new_device_num_;
	RAWINPUTDEVICELIST *new_device_list_;

	//接続されているデバイス数 >> new_device_num_
	if(GetRawInputDeviceList(NULL,&new_device_num_,sizeof(RAWINPUTDEVICELIST)) != 0) return 1;
	new_device_list_ = new RAWINPUTDEVICELIST[new_device_num_];
	if(new_device_list_ == NULL) return 2;
	
	UINT ntmp = new_device_num_;
	if (GetRawInputDeviceList(new_device_list_, &new_device_num_, sizeof(RAWINPUTDEVICELIST)) == -1) return 3;
	
	if(ntmp != new_device_num_)
	{
		delete[] new_device_list_;
		new_device_list_ = NULL;
		return 4;
	}

	delete[] device_list_;
	device_list_ = std::move(new_device_list_);
	device_num_ = new_device_num_;

	return 0;
}

BOOL util::ChangeWindow(HWND hWnd,HWND AfterWnd)
{
	BOOL res = FALSE;

	DWORD Thread1,Thread2,PID,Buf,nil;
	Thread1 = GetWindowThreadProcessId(GetForegroundWindow(),&PID);
	Thread2 = GetCurrentThreadId();

	AttachThreadInput(Thread1, Thread2, true);
	SystemParametersInfo(SPI_GETFOREGROUNDLOCKTIMEOUT, 0, &Buf, 0);
	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &nil, 0);

	if(IsIconic(hWnd)) ShowWindow(hWnd,SW_SHOW);
	res |= SetForegroundWindow(AfterWnd);
	res |= BringWindowToTop(AfterWnd);
	SetFocus(AfterWnd);

	SystemParametersInfo(SPI_SETFOREGROUNDLOCKTIMEOUT, 0, &Buf, 0);
	AttachThreadInput(Thread2, Thread1, false);
	return res;
}
