#include <string>

namespace input{
	static enum device_type{Mouse_Type , KeyBoard_Type , HID_Type, ALL};

	class device_list{
		UINT device_num_;
		RAWINPUTDEVICELIST *device_list_;
	public:
		device_list();
		~device_list();
		UINT operator() (RAWINPUTDEVICELIST *list,size_t &size);
		int reset();
	};

}//namespace input end

namespace util{
	BOOL ChangeWindow(HWND hWnd,HWND AfterWnd);
	inline std::string KeyBdHID(HANDLE HID)
	{
		return (boost::format("HID of KeyBoard : %8X") % HID).str();
	}
	inline std::string BindListText(HANDLE HID,std::string& Window_Name)
	{
		return (boost::format("KeyBd:%8X Window:%s") % HID % Window_Name).str();
	}
	inline void KeyBdListUpdate(HWND hList)
	{
		static input::device_list devices;
		SendMessage(hList , CB_RESETCONTENT ,0 , 0);

		devices.reset();
			
		//サイズ取得
		size_t size;
		devices(NULL,size);
	
		//サイズにより領域確保および代入
		RAWINPUTDEVICELIST *raws = new RAWINPUTDEVICELIST[size];
		devices(raws,size);

		for(unsigned int i=0;i<size;++i)
		{
			if((raws+i)->dwType == input::KeyBoard_Type)
				SendMessage(hList , CB_ADDSTRING ,0 , (LPARAM)KeyBdHID((raws+i)->hDevice).c_str());
		}

		delete[] raws;

		return;
	}
}//namespace util end
