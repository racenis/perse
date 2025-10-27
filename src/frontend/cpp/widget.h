#ifndef PERSE_CPP_WIDGET
#define PERSE_CPP_WIDGET

#include <string>
#include <vector>
#include <functional>

#include "property.h"

namespace perse {

bool Wait();

class Widget {
public:
	Widget(Property<int>&, Property<int>&, Property<int>&, Property<int>&,
		   Property<int>&, Property<int>&, Property<int>&, Property<int>&, void**);
	Widget& operator<<(std::initializer_list<Widget> children);
	Widget& operator<<(std::vector<Widget> children);
	static Widget Null();
protected:
	Widget();
	void* ptr = nullptr;
	std::vector<Widget> children;
	friend bool Wait();
};

inline std::vector<Widget> Inside(std::vector<Widget> children) {
    return children;
}

typedef std::function<void()> OnClickCallback;

extern Widget Null;

enum Direction {
	LEFT,
	RIGHT,
	UP,
	DOWN
};

struct ArrowButtonProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<Direction> dir;
	
	Property<std::string> text;
	Property<bool> enabled;
	Property<OnClickCallback> onclick;
};

struct ButtonProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::string> text;
	Property<bool> enabled;
	Property<OnClickCallback> onclick;
};

struct ImageButtonProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::string> text;
	Property<bool> enabled;
	Property<std::string> image;
	Property<OnClickCallback> onclick;
};

struct TextFieldProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::string> text;
	Property<std::string> hint;
	
	Property<bool> enabled;
	Property<bool> readonly;
	
	Property<OnClickCallback> onchange;
	Property<OnClickCallback> onsubmit;	
};

struct TextAreaProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::string> text;
	Property<std::string> hint;
	
	Property<bool> enabled;
	Property<bool> readonly;
	
	Property<OnClickCallback> onchange;
	Property<OnClickCallback> onsubmit;
};

struct LabelProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::string> text;
};

struct CheckBoxProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::string> text;
	Property<bool> value;
	Property<bool> enabled;
	Property<OnClickCallback> onclick;
};

struct RadioButtonProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::string> text;
	Property<bool> value;
	Property<bool> enabled;
	Property<int> index;
	Property<int> group;
	Property<OnClickCallback> onclick;
};

struct ComboBoxProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::vector<std::string>> items;
	Property<std::vector<void*>> value;
	Property<std::vector<OnClickCallback>> onselect;
};

struct ListBoxProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::vector<OnClickCallback>> onselect;
};

struct TabGroupProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
};


struct TabPanelProps {
	Property<std::string> text;
};

struct GroupPanelProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::string> text;
};


struct ItemProps {
	Property<std::string> title;
};


struct AbsoluteLayoutProps {
	Property<int> min_width;
	Property<int> min_height;
	
	Property<int> max_width;
	Property<int> max_height;
	
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
};




struct WindowProps {
	Property<int> width;
	Property<int> height;
	
	Property<int> x;
	Property<int> y;
	
	Property<std::string> title;
};

Widget ArrowButton(ArrowButtonProps);
Widget Button(ButtonProps);
Widget ImageButton(ImageButtonProps);
Widget TextField(TextFieldProps);
Widget TextArea(TextAreaProps);
Widget Label(LabelProps);
Widget CheckBox(CheckBoxProps);
Widget RadioButton(RadioButtonProps);
Widget ComboBox(ComboBoxProps);
Widget ListBox(ListBoxProps);
Widget TabGroup(TabGroupProps);
Widget TabPanel(TabPanelProps);
Widget GroupPanel(GroupPanelProps);

Widget Item(ItemProps);

Widget AbsoluteLayout(AbsoluteLayoutProps);
Widget HorizontalLayout(AbsoluteLayoutProps);
Widget VerticalLayout(AbsoluteLayoutProps);

Widget Window(WindowProps);

}

#endif // PERSE_CPP_WIDGET