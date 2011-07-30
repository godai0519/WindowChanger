#include <Windows.h>

namespace input{
	static enum device_type{Mouse_Type , KeyBoard_Type , HID_Type, ALL};
	
	template<device_type TYPE = ALL>
	class device_list{
		UINT device_num_;
		RAWINPUTDEVICELIST *device_list_;
	public:
		device_list()
		{
			device_list_ = NULL;
			reset();
		}
		~device_list()
		{
			if(device_list_ != NULL) delete[] device_list_;
		}
		UINT operator() (RAWINPUTDEVICELIST *list,size_t size)
		{
			if(list != NULL)
			{
				for(int i=0;i<device_num_ && i<size;++i)
					*(list+i) = device_list_[i];
			}
			return device_num_;
		}
		int reset()
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

			if(TYPE == ALL)
			{
				device_list_ = new_device_list_;
				device_num_ = new_device_num_;
			}
			else
			{
				//指定されたデバイスの抜き出し処理
				//TODO : さらに高速化が望める
				device_num_=0;
				for(UINT i=0;i<new_device_num_;++i) if(new_device_list_[i].dwType == TYPE) ++device_num_;
				device_list_ = new RAWINPUTDEVICELIST[device_num_];
				for(UINT i=0,j=0;i<new_device_num_;++i)
				{
					if(new_device_list_[i].dwType == TYPE)
					{
						*(device_list_+j) = new_device_list_[i];
						++j;
					}
				}
				//ここまで（TODO : さらに高速化が望める）
			}

			return 0;
		}
	};
}
