#include <Windows.h>
#include <string>
#include <vector>
//#include <boost/foreach.hpp>
//#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/xml_parser.hpp>

namespace manage{
	class BindingPair{
	public:
		BindingPair(){BindingPair(NULL,"",NULL);}
		BindingPair(HWND Binding_hWnd,std::string Binding_Name,HANDLE Binding_KeyBoard)
		{
			Binding_hWnd=Binding_hWnd;
			Binding_Name=Binding_Name;
			Binding_KeyBoard=Binding_KeyBoard;
		}
		HWND Binding_hWnd;
		std::string Binding_Name;
		HANDLE Binding_KeyBoard;
	};

	class BindedMng{
		typedef std::vector<BindingPair> Vector_Type;
		Vector_Type v_;
	public:
		BindedMng(){}
		inline Vector_Type* operator() ()
		{
			return &v_;
		}
		inline BindingPair* operator[] (unsigned int n)
		{
			return &v_[n];
		}

/*		void serialize(const std::string file)
		{
			//http://sites.google.com/site/boostjp/tips/xml
			using namespace boost::property_tree;
			ptree pt;
			ptree item_list = pt.add<int>("Item",v_.size());

			BOOST_FOREACH(const BindingPair bind,v_)
			{
				ptree& child = pt.add("BindList.Bind", "");
				child.put<int>("Settings.<xmlattr>.hWnd",(int)bind.Binding_hWnd);
				child.put<std::string>("Settings.<xmlattr>.WindowName",bind.Binding_Name);
				child.put<int>("Settings.<xmlattr>.KeyBoard",(int)bind.Binding_KeyBoard);
				child.put<std::string>("Settings.<xmlattr>.BindName",bind.Disp);
			}
			
			using namespace boost::property_tree::xml_parser;
			const int indent = 2;
			write_xml(file, pt, std::locale(),xml_writer_make_settings(' ', indent, widen<char>("utf-8")));
		}
		void deserialize(const std::string file)
		{
			v_.clear();

			using namespace boost::property_tree;
			ptree pt;
			read_xml(file, pt);
			BOOST_FOREACH (const ptree::value_type& child_, pt.get_child("BindList.Bind"))
			{
				const ptree& child = child_.second;
				//HWND Binding_hWnd = (HWND)child.get<int>("Settings.<xmlattr>.hWnd");
				//std::string Binding_Name = child.get<std::string>("Settings.<xmlattr>.WindowName");
				//HANDLE Binding_KeyBoard = (HWND)child.get<int>("Settings.<xmlattr>.KeyBoard");
				//std::string Disp = child.get<std::string>("Settings.<xmlattr>.BindName");
				//v_.push_back(BindingPair(Disp,Binding_hWnd,Binding_Name,Binding_KeyBoard));
			}			
		}*/
	};

}
