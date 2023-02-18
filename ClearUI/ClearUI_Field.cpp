#include "ClearUI_Field.h"


bool Field::render(bool force) {
	if (!force && !isOutOfDate())
		return false;

	display.fillRect(x, y, w, h, backColor());
	redraw();
	selectedAsDrawn = selected;
	return true;
}

void Field::select(bool s) {
	selected = s;
}

void Field::enter(bool alternate) { }
void Field::exit() { }

bool Field::click(Button::State s) { return false; }
void Field::update(Encoder::Update update) { }

bool Field::isOutOfDate() { return selectedAsDrawn != selected; }
