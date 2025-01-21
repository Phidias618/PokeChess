#pragma once

class BaseButton;
//template<class val_type> class Slider;

#include "SDL+.h"

class BaseButton {
protected:
	SDL_Rect rect;
	bool currently_active : 1;
	bool was_just_activated : 1;

public:
	BaseButton(SDL_Rect r) {
		rect = r;
		currently_active = was_just_activated = is_active();
	};

	virtual bool is_active() { return false; } //call each fram to know wether the button is active

	virtual void update() {} //call each time the button is drawn

	virtual void effect(int x, int y) {} //call uppon clicking the button

	virtual void draw(Surface s) {}

	virtual bool is_on_button(int x, int y) { return true; } // use to make button with other shape than a rectangle

	virtual ~BaseButton() {};
};

/*template<class val_type>
class Slider : public BaseButton {
	val_type value;
	val_type min_val;
	val_type max_val;
public:
	virtual void draw(Surface s) {
		Button.draw(s);
	}
};*/