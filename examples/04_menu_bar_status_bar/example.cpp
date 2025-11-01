#include "../../src/frontend/cpp/perse.h"
#include "../../src/frontend/cpp/hooks.h"

/*

	LIBPERSE EXAMPLE #04 -- Menu Bar and Status Bar
	
	This example demonstrates the use of the menu bar and the status bar
	widgets.
	
*/

using namespace perse;

Widget Root() {
	SetContext("Root");
	
	return Window({
		.width = 320,
		.height = 240,
		.x = -1,
		.y = -1,
		.title = "Menu Bar and Status Bar Example"
	}) << Inside({
		MenuBar({}) << Inside({
			Item({.title = "File"}) << Inside({
				Item({.title = "New"}),
				Item({.title = "Open"}),
			}),
			Item({.title = "Menu"})  << Inside({
				Item({.title = "Rat Pizza\t8.95 euro"}),
				Item({.title = "Gutter Oil Fries\t2.20 euro"}),
				Item({.title = "Moldy Burger\t4.30 euro"}),
				Item({.title = "Soapwater\t1.00 euro"}),
			}),
			Item({.title = "Help"})
		}),
		StatusBar({}) << Inside({
			Item({.title = "Status", .width = 50}),
			Item({.title = "ENCHUNGIFIED", .width = 120}),
			Item({.title = "10 days remaining..."})
		}),
		VerticalLayout({}) << Inside({
			AbsoluteLayout({}),
			TextField({
				.width = 128,
				.height = 24,
				.text = "Sample input"
			}),
			AbsoluteLayout({.height=8}),
			Button({
				.width = 128,
				.height = 24,
				.text = "Do nothing with it"
			}),
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
