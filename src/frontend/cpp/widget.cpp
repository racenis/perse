
#include <map>

#include "widget.h"

extern "C" {
#include "../../library/widget.h"
#include "../../library/perse.h"
}

namespace perse {

Widget::Widget(Property<int>& min_w, Property<int>& min_h,
	           Property<int>& max_w, Property<int>& max_h,
		       Property<int>& w, Property<int>& h,
		       Property<int>& x, Property<int>& y, void** ptr) {
	perse_widget* widget = perse_AllocateWidget();
	
	if (min_w.set()) widget->constraint_size.min.w = min_w;
	if (min_h.set()) widget->constraint_size.min.h = min_h;
	if (max_w.set()) widget->constraint_size.max.w = max_w;
	if (max_h.set()) widget->constraint_size.max.h = max_h;
	
	if (w.set()) {
		widget->constraint_size.min.w = w;
		widget->constraint_size.max.w = w;
	}
	
	if (h.set()) {
		widget->constraint_size.min.h = h;
		widget->constraint_size.max.h = h;
	}
	
	if (x.set()) widget->position.x = x;
	if (y.set()) widget->position.y = y;
	
	this->ptr = widget;
	*ptr = widget;
}

Widget::Widget() {
	ptr = nullptr;
}

Widget Widget::Null() {
	return Widget();
}

Widget Null = Widget::Null();

Widget& Widget::operator<<(std::initializer_list<Widget> children){
	for (const auto& widget : children) {
		if (!widget.ptr) continue;
		//perse_SetParent((perse_widget*)widget.ptr, (perse_widget*)this->ptr);
		perse_AddChild((perse_widget*)this->ptr, (perse_widget*)widget.ptr);
	}
	
	return *this;
}

// TODO: check if we actually need this???
Widget& Widget::operator<<(std::vector<Widget> children) {
	for (auto widget = children.rbegin(); widget != children.rend(); ++widget) {
		if (!widget->ptr) continue;
		perse_SetParent((perse_widget*)widget->ptr, (perse_widget*)this->ptr);
		//perse_AddChild((perse_widget*)this->ptr, (perse_widget*)widget->ptr);
	}
	/*for (const auto& widget : children) {
		perse_AddChild((perse_widget*)this->ptr, (perse_widget*)widget.ptr);
	}*/
	
	return *this;
}

#define INIT_WIDGET(WIDGTYPE) \
	perse_widget* widget; \
	Widget widget_class(props.min_width, props.min_height, \
	                    props.max_width, props.max_height, \
	                    props.width, props.height, props.x, props.y, (void**)&widget); \
	widget->type = WIDGTYPE;

static void add_prop(perse_widget* widget, perse_name_t name,
                     Property<int> value) {
	if (!value.set()) return;
	perse_property_t* p = perse_CreatePropertyInteger(value);
	p->name = name;
	perse_AddProperty(widget, p);
}

static void add_prop(perse_widget* widget, perse_name_t name,
                     Property<bool> value) {
	if (!value.set()) return;
	perse_property_t* p = perse_CreatePropertyBoolean(value);
	p->name = name;
	perse_AddProperty(widget, p);
}

static void add_prop(perse_widget* widget, perse_name_t name,
                     Property<std::string> value) {
	if (!value.set()) return;
	perse_property_t* p = perse_CreatePropertyString(value.get().c_str());
	p->name = name;
	perse_AddProperty(widget, p);
}

struct UserInfo {
	OnClickCallback onclick;
};

static void add_prop(perse_widget* widget, perse_name_t name,
                     Property<OnClickCallback> value) {
	if (!value.set()) return;
	
	UserInfo* info;
	if (widget->user) {
		info = (UserInfo*)widget->user;
	} else {
		info = new UserInfo;
		widget->user = info;
		widget->destroy = [](void* user){
			delete (UserInfo*)user;
		};
	}
	
	perse_property_t* p;
	
	switch (name) {
		case PERSE_NAME_ON_CLICK:
			info->onclick = value;
			p = perse_CreatePropertyCallback([](perse_widget_t* w){
				((UserInfo*)w->user)->onclick();
			});
		break;
		default:
			perse_Log("CPP:: unknwn CALLBACK enum: %i\n", name);
			abort();
	}
	
	p->name = name;
	perse_AddProperty(widget, p);
}


Widget ArrowButton(ArrowButtonProps props) {
	INIT_WIDGET(PERSE_WIDGET_ARROW_BUTTON)
	
	return widget_class;
}

Widget Button(ButtonProps props) {
	INIT_WIDGET(PERSE_WIDGET_TEXT_BUTTON)
	
	add_prop(widget, PERSE_NAME_TEXT, props.text);
	add_prop(widget, PERSE_NAME_ENABLED, props.enabled);
	add_prop(widget, PERSE_NAME_ON_CLICK, props.onclick);
	
	return widget_class;
}

Widget ImageButton(ImageButtonProps props) {
	INIT_WIDGET(PERSE_WIDGET_IMAGE_BUTTON)
	
	return widget_class;
}

Widget TextField(TextFieldProps props) {
	INIT_WIDGET(PERSE_WIDGET_TEXT_BOX)
	
	return widget_class;
}

Widget TextArea(TextAreaProps props) {
	INIT_WIDGET(PERSE_WIDGET_TEXT_AREA)
	
	return widget_class;
}

Widget Label(LabelProps props) {
	INIT_WIDGET(PERSE_WIDGET_LABEL)
	
	return widget_class;
}

Widget CheckBox(CheckBoxProps props) {
	INIT_WIDGET(PERSE_WIDGET_CHECK_BOX)
	
	return widget_class;
}

Widget RadioButton(RadioButtonProps props) {
	INIT_WIDGET(PERSE_WIDGET_RADIO_BUTTON)
	
	return widget_class;
}

Widget ComboBox(ComboBoxProps props) {
	INIT_WIDGET(PERSE_WIDGET_COMBO_BOX)
	
	return widget_class;
}

Widget ListBox(ListBoxProps props) {
	INIT_WIDGET(PERSE_WIDGET_LIST_BOX)
	
	return widget_class;
}

Widget TabGroup(TabGroupProps props) {
	INIT_WIDGET(PERSE_WIDGET_TAB_GROUP)
	
	return widget_class;
}

Widget TabPanel(TabPanelProps props) {
	perse_widget* widget;
	Property<int> unset;
	Widget widget_class(unset, unset, unset, unset,
	                    unset, unset, unset, unset,
	                    (void**)&widget);
	widget->type = PERSE_WIDGET_TAB_PANEL;
	
	return widget_class;
}

Widget GroupPanel(GroupPanelProps props) {
	INIT_WIDGET(PERSE_WIDGET_GROUP_PANEL)
	
	return widget_class;
}



Widget AbsoluteLayout(AbsoluteLayoutProps props) {
	INIT_WIDGET(PERSE_WIDGET_ABSOLUTE_LAYOUT)
	return widget_class;
}

Widget HorizontalLayout(AbsoluteLayoutProps props) {
	INIT_WIDGET(PERSE_WIDGET_HORIZONTAL_LAYOUT)
	return widget_class;
}
Widget VerticalLayout(AbsoluteLayoutProps props) {
	INIT_WIDGET(PERSE_WIDGET_VERTICAL_LAYOUT)
	return widget_class;
}

void temp_resize_callback(perse_widget*);

Widget Window(WindowProps props) {
	perse_widget* widget;
	Widget widget_class(props.width, props.height, props.width, props.height,
	                    props.width, props.height, props.x, props.y,
	                    (void**)&widget);
	widget->type = PERSE_WIDGET_WINDOW;
	
	add_prop(widget, PERSE_NAME_TITLE, props.title);
	
	// for now we'll try this approach 
	perse_property_t* p = perse_CreatePropertyCallback(temp_resize_callback);
	p->name = PERSE_NAME_ON_RESIZE;
	perse_AddProperty(widget, p);
	
	return widget_class;
}


}