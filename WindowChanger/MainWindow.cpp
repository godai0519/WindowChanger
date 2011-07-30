#include <Windows.h>
#include <fstream>
#include <map>
#include <string>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "Util.hpp"
#include "SettingData.hpp"
#include "resource.h"

HINSTANCE g_hInstance;

namespace gui{
	#define WINDOW_NAME "WindowChanger"
	#define WINDOW_TITLE WINDOW_NAME

	#define WM_NOTIFY_CLICKED (WM_USER+1)
	#define WM_USER_APPLY_BUTTON (WM_USER+2)
	#define WM_USER_UPDATE_BUTTON (WM_USER+3)
	#define WM_USER_KEYBD_CHK (WM_USER+4)
	#define WM_USER_CREATE_BUTTON (WM_USER+5)
	#define WM_USER_DELETE_BUTTON (WM_USER+6)
	#define WM_USER_BINDUP_BUTTON (WM_USER+7)
	#define WM_USER_BINDDOWN_BUTTON (WM_USER+8)
	#define WM_USER_BIND_LIST (WM_USER+9)

	HWND g_hWnd;

	namespace proc{
		BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam )
		{
			char buff[256] = "";
			GetWindowText( hWnd,buff, sizeof(buff));
			if(buff[0] != 0x00) ((std::map<std::string,HWND> *)lParam)->insert(std::pair<std::string,HWND>(buff,hWnd));
			return 1;
		}
	}//namespace proc end

	#define LABEL_1 ("キーを打ち込むことで束縛するキーボードを選択できます。")
	#define LABEL_2 ("キーボードに束縛するウィンドウを指定してください。")

	/*struct BindData{
		BindData()
		{
			BindData("",NULL,"",NULL);
		}
		BindData(std::string Disp,HWND Binding_hWnd,std::string Binding_Name,HANDLE Binding_KeyBoard)
		{
			Disp=Disp;
			Binding_hWnd=Binding_hWnd;
			Binding_Name=Binding_Name;
			Binding_KeyBoard=Binding_KeyBoard;
		}
		std::string Disp;
		HWND Binding_hWnd;
		std::string Binding_Name;
		HANDLE Binding_KeyBoard;
	};*/
	LRESULT CALLBACK WinProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
	{	
		static const HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		static HDC hDC;
		static RECT rect;
		static NOTIFYICONDATA nid;
		static HWND hBindingList;
		static HWND hCreate,hDelete,hBIndUp,hBindDown;
		static HWND hNotifyMenu;
		static HWND hApplyButton,hKeybdUpdateButton;
		static HWND hWindowSelect,hKeyBdSelect;
		static HWND hKeyBdChk;
		static HANDLE Latest_Device_HID = NULL;
		static std::map<std::string,HWND> WindowList;
		static manage::BindedMng BindList;

		switch(Msg)
		{
		case WM_PAINT:		
				//ラベル
				TextOut(hDC , 10 , 150 , LABEL_1 , lstrlen(LABEL_1));
				TextOut(hDC , 10 , 215 , LABEL_2 , lstrlen(LABEL_2));

				break;
		case WM_CREATE:
			{
				g_hWnd = hWnd;
				hDC = GetDC(hWnd);
				SelectObject(hDC,hFont);

				//常駐
				nid.cbSize = sizeof(NOTIFYICONDATA);
				nid.hWnd = hWnd;
				nid.uID = 1;
				nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
				nid.uCallbackMessage = WM_NOTIFY_CLICKED;
				nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
				strcpy_s(nid.szTip,0x80,WINDOW_TITLE);
				Shell_NotifyIcon(NIM_ADD, &nid);

				//RawInputにてWM_INPUTでcatchできるように登録
				RAWINPUTDEVICE Rid[1];
				Rid[0].usUsagePage = 01; 
				Rid[0].usUsage = 06; 
				Rid[0].dwFlags = RIDEV_INPUTSINK;
				Rid[0].hwndTarget = hWnd;
				if(!RegisterRawInputDevices(Rid, 1, sizeof(Rid[0]))) break;
			
				hBindingList = CreateWindow(
					"LISTBOX",
					NULL,
					WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
					10,10,
					370,100,
					hWnd,
					(HMENU)WM_USER_BIND_LIST,
					g_hInstance,
					NULL
					);

				hCreate = CreateWindow(
					"BUTTON",
					"Create",
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					10, 110,
					110, 30,
					hWnd,
					(HMENU)WM_USER_CREATE_BUTTON,
					g_hInstance,
					NULL
				);
						
				hDelete = CreateWindow(
					"BUTTON",
					"Delete",
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					270, 110,
					110, 30,
					hWnd,
					(HMENU)WM_USER_DELETE_BUTTON,
					g_hInstance,
					NULL
				);
				hBIndUp = CreateWindow(
					"BUTTON",
					"↑",
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					130, 110,
					60, 30,
					hWnd,
					(HMENU)WM_USER_BINDUP_BUTTON,
					g_hInstance,
					NULL
				);
				hBindDown = CreateWindow(
					"BUTTON",
					"↓",
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					200, 110,
					60, 30,
					hWnd,
					(HMENU)WM_USER_BINDDOWN_BUTTON,
					g_hInstance,
					NULL
				);
				//ボタンの登録
				hApplyButton = CreateWindow(
					"BUTTON",
					"Apply",
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					20, 260,
					60, 40,
					hWnd,
					(HMENU)WM_USER_APPLY_BUTTON,
					g_hInstance,
					NULL
				);
				hKeybdUpdateButton = CreateWindow(
					"BUTTON",
					"Update",
					WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
					90, 260,
					60, 40, 
					hWnd,
					(HMENU)WM_USER_UPDATE_BUTTON,
					g_hInstance,
					NULL
				);
			
				//Window列挙
				hWindowSelect = CreateWindow(
					"COMBOBOX",
					"WindowSelect",
					WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP | CBS_HASSTRINGS | CBS_AUTOHSCROLL | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_SORT | CBS_SIMPLE,
					10,230,
					370,300,
					hWnd,
					NULL,
					g_hInstance,
					NULL
					);
				EnumWindows(&gui::proc::EnumWindowsProc,(LPARAM)&WindowList);
				for(std::map<std::string,HWND>::iterator it = WindowList.begin();it != WindowList.end(); ++it)
				{
					if(IsWindowVisible(it->second))
						SendMessage(hWindowSelect , CB_ADDSTRING ,0 , (LPARAM)it->first.c_str());
				}

				//キーボード選択
				hKeyBdSelect = CreateWindow(
					"COMBOBOX",
					"WindowSelect",
					WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_TABSTOP | CBS_HASSTRINGS | CBS_AUTOHSCROLL | CBS_DROPDOWNLIST | WS_VSCROLL | CBS_SORT | CBS_SIMPLE,
					10,185,
					370,300,
					hWnd,
					NULL,
					g_hInstance,
					NULL
					);

				//キーボードチェッカー用エディットボックス
				hKeyBdChk = CreateWindowEx(
					WS_EX_CLIENTEDGE | SWP_FRAMECHANGED,
					"EDIT",
					"",
					WS_CHILD | WS_VISIBLE,
					10, 165,
					370,20,
					hWnd,
					(HMENU)WM_USER_KEYBD_CHK,
					g_hInstance,
					NULL
				);

				//キーボードの一覧を取得し、表示
				SendMessage(hKeyBdSelect , CB_RESETCONTENT ,0 , 0);
				util::KeyBdListUpdate(hKeyBdSelect);

				//フォント切り替え
				SendMessage(hCreate, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
				SendMessage(hDelete, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
				SendMessage(hKeyBdSelect, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
				SendMessage(hWindowSelect, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
				SendMessage(hApplyButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
				SendMessage(hKeybdUpdateButton, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
				SendMessage(hKeyBdChk, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
				SendMessage(hBindingList, WM_SETFONT, (WPARAM)hFont, MAKELPARAM(FALSE, 0));
				break;
			}
		case WM_NOTIFY_CLICKED:
			{
				switch (lParam)
				{
				case WM_LBUTTONDBLCLK:
					ShowWindow(hWnd,SW_SHOW);
					break;
				case WM_RBUTTONUP:
					HMENU hMenu, hSubMenu;
					POINT pt;
					GetCursorPos( &pt );

					// リソースよりメニューをロード
					hMenu = LoadMenu( g_hInstance, MAKEINTRESOURCE(IDR_MENU1) );
					hSubMenu = GetSubMenu( hMenu, 0 );
					SetForegroundWindow( hWnd );

					// ポップアップメニューを表示
					TrackPopupMenu( hSubMenu, TPM_BOTTOMALIGN, pt.x, pt.y, 0, hWnd, NULL );
					DestroyMenu( hMenu );
				}
				break;
			}
		case WM_CLOSE:
			{
				//可視状態ならタスクトレイに収納
				if(IsWindowVisible(hWnd)==TRUE)
				{
					ShowWindow(hWnd,SW_HIDE);
					return 0;
				}
				break;
			}
		case WM_COMMAND:
			{
				switch(LOWORD(wParam)) {
				case WM_USER_APPLY_BUTTON:
					{
						static char str[256];
						static int vec_index,begin_index;
						vec_index = begin_index = SendMessage(hBindingList, LB_GETCURSEL ,NULL ,NULL);

						if(vec_index == -1)
						{
							vec_index = SendMessage(hBindingList, LB_GETCOUNT   ,NULL,NULL);
							BindList()->push_back(manage::BindingPair());
						}

						int win_index = (int)SendMessage(hWindowSelect, CB_GETCURSEL ,NULL,NULL);
						int keybd_index = (int)SendMessage(hKeyBdSelect, CB_GETCURSEL ,NULL,NULL);
						if(win_index==-1 || keybd_index==-1) 
						{
							MessageBox(hWnd,"キーボードとウィンドウを選択してください。","注意",MB_OK | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
							break;
						}

						//選択ウィンドウを抽出
						SendMessage(hWindowSelect, CB_GETLBTEXT,win_index,(LPARAM)str);
						BindList[vec_index]->Binding_Name = str;
						BindList[vec_index]->Binding_hWnd = WindowList[str];

						//選択キーボードを抽出
						SendMessage(hKeyBdSelect, CB_GETLBTEXT,keybd_index,(LPARAM)str);
						BindList[vec_index]->Binding_KeyBoard = (HANDLE)std::strtol(((std::string)str).substr(((std::string)"HID of KeyBoard : ").size()).c_str(), NULL, 16);

						if(begin_index != -1)
						{
							//選択されてたのなら古い物を消さなくちゃ。
							SendMessage(hBindingList,LB_DELETESTRING,vec_index,NULL);
						}
						SendMessage(hBindingList,LB_INSERTSTRING,vec_index,(LPARAM)util::BindListText(BindList[vec_index]->Binding_KeyBoard,BindList[vec_index]->Binding_Name).c_str());
						//ShowWindow(hWnd,SW_HIDE);

						break;
					}
				case WM_USER_UPDATE_BUTTON:
					{
						WindowList.clear();
						EnumWindows(&gui::proc::EnumWindowsProc,(LPARAM)&WindowList);
						SendMessage(hWindowSelect , CB_RESETCONTENT ,0 , 0);
						for(std::map<std::string,HWND>::iterator it = WindowList.begin();it != WindowList.end(); ++it)
						{
							if(IsWindowVisible(it->second))
								SendMessage(hWindowSelect , CB_ADDSTRING ,0 , (LPARAM)it->first.c_str());
						}
						SendMessage(hKeyBdSelect , CB_RESETCONTENT ,0 , 0);
						util::KeyBdListUpdate(hKeyBdSelect);
						break;
					}
				case WM_USER_CREATE_BUTTON:
					{
						SendMessage(hBindingList,LB_SETCURSEL,SendMessage(hBindingList, LB_ADDSTRING ,0 ,(LPARAM)"NEW"),NULL);
						BindList()->push_back(manage::BindingPair());
						break;
					}
				case WM_USER_DELETE_BUTTON:
					{
						int index = SendMessage(hBindingList, LB_GETCURSEL ,NULL ,NULL);
						if(index == -1) MessageBox(hWnd,"選択してください。","注意",MB_OK | MB_ICONEXCLAMATION | MB_DEFBUTTON1);
						else
						{
							BindList()->erase(BindList()->begin()+index);
							SendMessage(hBindingList, LB_DELETESTRING ,index,NULL);
						}
						if(index == SendMessage(hBindingList, LB_GETCOUNT ,NULL ,NULL))
						{
							SendMessage(hBindingList, LB_SETCURSEL ,index-1 ,NULL);
						}
						else SendMessage(hBindingList, LB_SETCURSEL ,index ,NULL);
						break;
					}
				case WM_USER_BINDUP_BUTTON:
					{
						int index = SendMessage(hBindingList, LB_GETCURSEL ,NULL ,NULL);
						if(index < 1) return 0;
					
						SendMessage(hBindingList, LB_DELETESTRING ,index,NULL);
						SendMessage(hBindingList, LB_INSERTSTRING,index-1,(LPARAM)util::BindListText(BindList[index]->Binding_KeyBoard,BindList[index]->Binding_Name).c_str());

						manage::BindingPair temp = *BindList[index];
						BindList()->erase(BindList()->begin()+index);
						BindList()->insert(BindList()->begin()+index-1,temp);

						SendMessage(hBindingList,LB_SETCURSEL,index-1,NULL);
						break;
					}
				case WM_USER_BINDDOWN_BUTTON:
					{ 
						int index = SendMessage(hBindingList, LB_GETCURSEL ,NULL ,NULL);
						if(index > SendMessage(hBindingList, LB_GETCOUNT ,NULL ,NULL)-2 || index < 0) return 0;
					
						manage::BindingPair temp= *BindList[index];

						SendMessage(hBindingList, LB_DELETESTRING ,index,NULL);
						BindList()->erase(BindList()->begin()+index);
						SendMessage(hBindingList, LB_INSERTSTRING,index+1,(LPARAM)util::BindListText(temp.Binding_KeyBoard,temp.Binding_Name).c_str());
						BindList()->insert(BindList()->begin()+index+1,temp);					
					
						SendMessage(hBindingList,LB_SETCURSEL,index+1,NULL);
						break;
					}
				case WM_USER_BIND_LIST:
					{
						switch(HIWORD(wParam)){
						case LBN_SELCHANGE:
							{
								int index = SendMessage(hBindingList, LB_GETCURSEL ,NULL ,NULL);
							
								if(SendMessage(hKeyBdSelect,CB_SELECTSTRING,0,(LPARAM)util::KeyBdHID(BindList[index]->Binding_KeyBoard).c_str()) == CB_ERR )
									SendMessage(hKeyBdSelect,CB_SETCURSEL,-1,NULL);

								if(SendMessage(hWindowSelect,CB_SELECTSTRING,0,(LPARAM)BindList[index]->Binding_Name.c_str()) == CB_ERR )
									SendMessage(hWindowSelect,CB_SETCURSEL,-1,NULL);

								break;
							}
						}
						break;
					}
				case WM_USER_KEYBD_CHK:
					{
						switch(HIWORD(wParam))
						{
						case EN_UPDATE:
							static bool flags = true; //ループによるスタックオーバーフローを回避する
							if(flags)
							{
								flags=false;

								SendMessage(hKeyBdSelect,CB_SELECTSTRING,0,(LPARAM)util::KeyBdHID(Latest_Device_HID).c_str()); //キーボードのIDでhKeyBdSelectをセレクト
								SendMessage(hKeyBdChk,WM_SETTEXT,0,(LPARAM)"") ; //文字入力の無効化

								flags=true;
							}
						}
						break;
					}
				case ID_EXIT:
					{
						DestroyWindow(hWnd); //Windowを閉じる
					}
				}
				break;
			}
		case WM_INPUT_DEVICE_CHANGE:
			{
				util::KeyBdListUpdate(hKeyBdSelect);
				break;
			}
		case WM_INPUT:
			{
				static bool FoundBindPair = false;
				static HWND OldWnd=NULL;

				//RawInputにより入力されたキーボードを取得
				UINT dwSize = 40;
				BYTE lpb[40];		
				GetRawInputData((HRAWINPUT)lParam, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER));
				Latest_Device_HID = ((RAWINPUT*)lpb)->header.hDevice;

				//表示されてればウィンドウ切り替え処理をしない。
				//TODO:「某ボックスにフォーカスがあるとき」に変えたほうが良いかも
				if(!IsWindowVisible(hWnd))
				{
					for(std::vector<manage::BindingPair>::iterator it = BindList()->begin();it!=BindList()->end();++it)
					{
						if(it->Binding_KeyBoard == Latest_Device_HID)
						{
							if(OldWnd==NULL) OldWnd = GetForegroundWindow();
							util::ChangeWindow(hWnd,it->Binding_hWnd);
							FoundBindPair=true;
							break;
						}
					}
					if(!FoundBindPair)
					{
						util::ChangeWindow(hWnd,OldWnd);
						OldWnd = NULL;
					}
				}
				FoundBindPair = false;

				break;
			}
		case WM_DESTROY:
			{
				//std::ofstream ofs("text.txt");
				//boost::archive::text_oarchive oa(ofs);
				//oa << BindList;

				ReleaseDC(hWnd , hDC);
				Shell_NotifyIcon(NIM_DELETE, &nid);
				DeleteObject(hFont);
				PostQuitMessage(0);
				break;
			}
		}
		return DefWindowProc(hWnd, Msg, wParam, lParam);
	}

	int Create(HINSTANCE hInstance)
	{
		WNDCLASSEX wc;
		wc.cbSize = sizeof(wc);
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = gui::WinProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra= 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = WINDOW_NAME;
		wc.hIconSm = NULL;
		if(!RegisterClassEx(&wc)) return 1;

		HWND hWnd;
		hWnd = CreateWindowEx(
			WS_EX_APPWINDOW, 
			WINDOW_NAME,
			WINDOW_TITLE, 
			WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX & ~WS_MINIMIZEBOX, 
			((GetSystemMetrics( SM_CXSCREEN ) - 400 ) / 2), 
			((GetSystemMetrics( SM_CYSCREEN ) - 400 ) / 2), 
			400, 
			400, 
			NULL, 
			NULL, 
			hInstance, 
			NULL
			);

		ShowWindow(hWnd,SW_SHOW);
		UpdateWindow(hWnd);

		MSG msg;
		while(GetMessage(&msg, NULL, 0, 0) != 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return msg.wParam;
	}
}

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	)
{	
	g_hInstance = hInstance;
	return gui::Create(hInstance);
}
