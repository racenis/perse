#include "../../src/frontend/cpp/perse.h"
#include "../../src/frontend/cpp/hooks.h"

/*

	LIBPERSE EXAMPLE #01 -- Buttons
	
	This example will open a small window with 2 buttons stacked vertically.
	
	When clicking on the first button, it will make a third button appear and
	disappear, as well as change its text.

	The second button will change the alignment -- first to horizontal and then
	it will alternate between vertical and horizontal alignments.

	
*/

using namespace perse;

Widget Root() {
	SetContext("Root");
	
	auto [text_index, set_text_index] = UseState(0);
	auto [vertical, set_vertical] = UseState(true);
	
	auto button_text = [=]() -> std::string {
		switch (text_index % 3) {
			default:
			case 0: return "Last Button";
			case 1: return "This is The Last";
			case 2: return "Last...";
		}
	};

	return Window({
		.width = 480,
		.height = 240,
		.x = -1,
		.y = -1,
		.title = "My program"
	}) << Inside({
		(vertical ? VerticalLayout({}) : HorizontalLayout({})) << Inside({
			AbsoluteLayout({}),
			Button({
				.width = 128,
				.height = 24,
				.text = "Hide/Unhide Last",
				.onclick = [=](){
					set_text_index(text_index + 1);
				}
			}),
			AbsoluteLayout({.height=8}),
			Button({
				.width = 128,
				.height = 24,
				.text = "Switch Alignment",
				.onclick = [=](){
					set_vertical(!vertical);
				}
			}),
			AbsoluteLayout({.height=8}),
			(text_index % 2 == 1 ? Button({
				.width = 128,
				.height = 24,
				.text = button_text()
			}) : AbsoluteLayout({
				.width = 128,
				.height = 24
			})),
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
