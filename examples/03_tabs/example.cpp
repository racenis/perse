#include "../../src/frontend/cpp/perse.h"
#include "../../src/frontend/cpp/hooks.h"

/*

	LIBPERSE EXAMPLE #03 -- Tabs
	
	This example contains a window with 3 tabs.
	
	None of the controls actually have any logic and are intended for viewing
	only.
	
*/

using namespace perse;

Widget Root() {
	SetContext("Root");
	
	return Window({
		.width = 320,
		.height = 240,
		.x = -1,
		.y = -1,
		.title = "My program"
	}) << Inside({
		TabGroup({}) << Inside({
			TabPanel({.text = "Question"}) << Inside({
				VerticalLayout({
					.width = 320,
					.height = 220,
					.y = 80
				}) << Inside({
					Label({
						.width = 128,
						.height = 24,
						.text = "Do you ever?"
					}),
					Button({
						.width = 128,
						.height = 24,
						.text = "Nope!"
					})
				})
			}),
			TabPanel({.text = "Recipe?"}) << Inside({
				VerticalLayout({
					.width = 320,
					.height = 220,
					.y = 80
				}) << Inside({
					ListBox({
						.width = 128,
						.height = 96
					}) << Inside({
						Item({.title = "800g dough"}),
						Item({.title = "200g pizza sauce"}),
						Item({.title = "250g cheese"}),
						Item({.title = "1 dead rat, minced"})
					}),
					Label({
						.width = 128,
						.height = 24,
						.text = "Could this be a recipe?"
					})
				})
			}),
			TabPanel({.text = "Info"}) << Inside({
				Label({
					.width = 100,
					.height = 100,
					.x = 100,
					.y = 100,
					.text = "Example #3 (C) 2025"
				}),
				Label({
					.width = 100,
					.height = 100,
					.x = 100,
					.y = 124,
					.text = "Available under MIT license"
				})
			})
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
