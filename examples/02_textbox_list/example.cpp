#include "../../src/frontend/cpp/perse.h"
#include "../../src/frontend/cpp/hooks.h"

/*

	LIBPERSE EXAMPLE #02 -- Listbox and Textbox
	
	This example demonstrates the use of listboxes and textboxes.
	
	When entering text into the textbox and clicking on the 'Add' button, the
	text will be added to the listbox and the textbox will be cleared.

	The same behavior will occur when pressing the 'enter' key after inputting
	text into the textbox.
	
	When selecting a text in the listbox and clicking on the 'Remove' button,
	the selected listbox item will be removed from the listbox.

*/

using namespace perse;

#include <iostream>

Widget Root() {
	SetContext("Root");
	
	auto list_box = ListBox({
		.width = 128,
		.height = 96
	});
	
	auto [texts, set_texts] = UseState<std::vector<std::string>>({
		"item #1",
		"item #2",
		"item #4",
	});
	auto [selected, set_selected] = UseState(-1);
	auto [new_text, set_new_text] = UseState(std::string("item #7"));
	
	std::vector<Widget> items;
	
	for (size_t i = 0; i < texts.size(); i++) {
		items.push_back({Item({
			.title = texts[i],
			.onclick = [=](){
				set_selected(i);
			}
		})});
	}

	list_box << items;
	
	return Window({
		.width = 240,
		.height = 240,
		.x = -1,
		.y = -1,
		.title = "List Manager v1.0"
	}) << Inside({
		VerticalLayout({}) << Inside({
			AbsoluteLayout({}),
			TextField({
				.width = 128,
				.height = 24,
				.text = new_text,
				.onchange = [=](std::string new_value){
					set_new_text(new_value);
				},
				.onsubmit = [=]() mutable {
					texts.push_back(new_text);
					set_texts(texts);
					
					set_new_text("");
				}
			}),
			AbsoluteLayout({.height=8}),
			HorizontalLayout({
				.width = 128,
				.height = 24, //commenting this out breaks everything? TODO:fix
			}) << Inside({
				Button({
					.height = 24,
					.text = "Add",
					.onclick = [=]() mutable {
						texts.push_back(new_text);
						set_texts(texts);
						
						set_new_text("");
					}
				}),
				Button({
					.height = 24,
					.text = "Remove",
					.onclick = [=](){
						if (selected < 0) {
							std::cout << "cannot!!!" << std::endl;
							return;
						}
						
						auto text_cpy = texts;
						text_cpy.erase(text_cpy.begin() + selected);
						set_texts(text_cpy);
						
						set_selected(-1);
					}
				}),
			}),
			AbsoluteLayout({.height=8}),
			list_box,
			AbsoluteLayout({})
		})
	});
}

int main(int argc, const char** argv) {
	Init();

	SetRoot(Root);

	while (Wait()) {
		
	}
	
	return 0;
}
