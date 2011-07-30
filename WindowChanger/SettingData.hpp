#include <Windows.h>
#include <string>
#include <vector>

namespace manage{
	//class BindingPair{
	//public:
	//	BindingPair(){BindingPair("",NULL,"",NULL);}
	//	BindingPair(std::string Disp,HWND Binding_hWnd,std::string Binding_Name,HANDLE Binding_KeyBoard)
	//	{
	//		Disp=Disp;
	//		Binding_hWnd=Binding_hWnd;
	//		Binding_Name=Binding_Name;
	//		Binding_KeyBoard=Binding_KeyBoard;
	//	}
	//	std::string Disp;
	//	HWND Binding_hWnd;
	//	std::string Binding_Name;
	//	HANDLE Binding_KeyBoard;
	//};

	//class BindedMng{
	//	typedef std::vector<BindingPair> Data_Type;
	//	Data_Type v_;
	//public:
	//	inline Data_Type* operator() () const
	//	{
	//		return (Data_Type*)&v_;
	//	}
	//	inline BindingPair* operator[] (unsigned int num)
	//	{
	//		return &v_[num];
	//	}
	//};

	class BindedMng{
	public:
		BindedMng() {}
	};

}
