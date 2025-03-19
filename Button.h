#pragma once

#include <queue>

#include "BaseButton.h"

class Button;
class ButtonCollection;
class BeginGameButton;
class EndOfGameButton;
class ExitGameButton;
class PromotionButton;
class TypingSelectionButton;
class ConfirmSelectionButton;
class RandomTypingButton;
class TextBoxDisplay;
class ChangeGameruleButton;

#include "assets.h"

#include "game.h"

class Button {
private:
	bool currently_active : 1;
	bool was_just_activated : 1;
protected:
	ButtonCollection* collection;

	double x;
	double y;
	double w;
	double h;
public:

	Button(double tile_x_, double tile_y_, double tile_w_, double tile_h_);

	virtual void draw() {}

	virtual bool is_active();
	virtual void resize();

	virtual void activate() {}
	virtual bool is_on_button(int mouse_button, double, double) { return mouse_button == SDL_BUTTON_LEFT; }

	virtual void effect(int mouse_button, double, double) {}

	virtual void hold(int mouse_button, double, double) {}

	virtual void unhold(int, double, double) {}

	virtual void kill();

	virtual ~Button() {}
	static void* useless;
	static void* useless5;
	static void* useless6;
	static Button* pressed_button;
	static void* useless2;
	static void* useless3;
	static void* useless4;
	static int pressed_button_mouse_button;

	friend class ButtonCollection;
	friend void handle_event(SDL_Event e);
	friend void loop();
};

class ButtonCollection {
private:
	bool is_cleared = false;

	class Node {
	public:
		Button* button;
		Node* next;
		Node(Button* b) {
			button = b;
			next = NULL;
		}
	};
	Node* first;
	Node* last;
	friend std::ostream& operator<<(std::ostream & os, ButtonCollection::Node * node);
public:
	ButtonCollection() {
		first = NULL;
		last = NULL;
	}

	// takes ownership of that button
	void add(Button* new_button); 


	// removes all the button of the collection
	void clear(); 

	// draws each active button
	void draw(); 

	// try to click on each button and return whether on click landed successfully on a button
	bool click(int, double x, double y);

	// calls Button::resize() on each buttons of the collection
	virtual void resize();

	~ButtonCollection() {
		clear();
	}

	friend class Button;
};

std::ostream& operator<<(std::ostream& os, ButtonCollection::Node* node);

class BeginGameButton : public Button {
public:
	BeginGameButton();

	virtual void draw();

	virtual bool is_active();

	virtual void effect(int click, double, double);
};

class EndOfGameButton : public Button {
	Color bg_color;
	Color txt_color;
	char message[16];
public:
	EndOfGameButton();

	virtual void draw();

	virtual void activate();

	virtual void effect(int, double, double);
};

class ExitGameButton : public Button {
public:
	ExitGameButton(double x_, double y_);

	virtual bool is_active();

	virtual void draw();

	virtual void effect(int, double, double);
};


class PromotionButton : public Button {
private:
	std::function<Piece* (Board&, piece_color, Square*, typing, PokeItem*)> piece_constructor;
	Surface sprite;
public:
	PromotionButton(std::function<Piece*(Board&, piece_color, Square*, typing, PokeItem*)>, double x_, double y_);

	virtual void activate();

	virtual void draw();

	virtual void effect(int, double, double);

	virtual void resize();
};

class SwitchSelectionButton : public Button {
public:
	SwitchSelectionButton(double x, double y);

	virtual void draw();

	virtual void effect(int, double, double);
};

class TypingSelectionButton : public Button {
	typing type;
public:
	TypingSelectionButton(double x_, double y_, typing);

	virtual bool is_active();

	virtual void draw();

	virtual void effect(int, double x_, double y_);
};

class ItemSelectionButton : public Button {
	ItemClass& Item;
	static Surface surface;
public:
	ItemSelectionButton(double x, double y, ItemClass&);

	virtual bool is_active();

	virtual bool is_on_button(int, double, double);

	virtual void draw();

	virtual void effect(int, double x_, double y_);
};

class ConfirmSelectionButton : public Button {

public:
	ConfirmSelectionButton(double x_, double y_);

	virtual void draw();

	virtual bool is_active();

	virtual void effect(int, double, double);
};

class RandomTypingButton : public Button {
	static const int period = FPS / 8;
	short current_sprite;
	short counter;
public:
	RandomTypingButton(double x_, double y_);

	virtual void draw();

	virtual void effect(int, double, double);
};

class TextBoxDisplay : public Button {
	Surface sprite;
	short visual_index;
	short text_index;
	short timer;
	bool is_on_second_line : 1;
	int shift_timer;
	int shift_delay;
	piece_color side : 2;

	bool is_first;
	TextBoxDisplay* next = NULL;

	static inline const int constexpr BUFFER_SIZE = 1024;
	char message[BUFFER_SIZE];

	static const short begin_x = 18;
	static const short begin_y = 32;
	static const short char_per_line = 18;
	static const short char_width = 16;
	static const short y_pixel_increment = 32;
public:

	static const int duration = FPS / 2;

	void destroy_all();

	TextBoxDisplay(const char* text, bool);

	virtual void activate();

	virtual bool is_active();

	virtual void draw();

	virtual ~TextBoxDisplay();

	void add(TextBoxDisplay* _next) { next = _next; }
};

class SkipBonusMoveButton : public Button {
public:
	SkipBonusMoveButton(double x_, double y_);

	virtual bool is_active();

	virtual void draw();

	virtual void effect(int, double, double);
};

class StatBoostDisplay : public Button {
private:
	int counter;
	double arrow_offset;
	int delay;
	pokestat stat;
	int boost_or_debuff;
	Piece* piece;
public:
	StatBoostDisplay(Piece* boosted_piece, pokestat boosted_stat, int delay_, int boost_or_debuff);

	virtual void draw();
};

class ShowTypechartButton : public Button {
	Surface sprite;
public:
	ShowTypechartButton(double x_, double y_);

	virtual bool is_active();

	virtual void draw();

	virtual void effect(int, double, double);

	virtual void resize();
};

class TypechartDisplay : public Button {
public:
	TypechartDisplay();

	virtual bool is_active();

	virtual void draw();
};

class SettingsButton : public Button {
	static const int period = FPS / 5;
	short current_sprite;
	short counter;
public:
	SettingsButton(double x_, double y_);

	virtual void draw();

	virtual void effect(int, double, double);
};


class DisableSoundsButton : public Button {
public:
	DisableSoundsButton(double x_, double y_);

	virtual void draw();

	virtual bool is_on_button(int, double, double);

	virtual void effect(int, double, double);
};

class VolumeSlider : public Button {
public:
	VolumeSlider(double x_, double y_);

	virtual void draw();
	
	virtual bool is_on_button(int, double, double);

	virtual void hold(int, double, double);
};

class BoardButton : public Button {
	Board& board;
	bool is_first_unhold;
	bool no_unclick;
public:
	BoardButton(Board& b);

	virtual void draw();
	virtual bool is_active();
	virtual void effect(int, double, double);
	virtual void unhold(int, double, double);
};


class InformationDisplay : public Button {
	Piece* displayed_piece;
	typing displayed_type;
	ItemClass* displayed_item;
	int displayed_page;

	Surface screen;
	LANGUAGE language;
	int edge;
	void re_draw();

public:
	InformationDisplay(double x, double y);

	virtual void draw();

	virtual bool is_active();

	virtual void resize();


	friend class PhoneSwitchPage;
};

class PhoneSwitchPage : public Button {
	bool is_right;
public:
	PhoneSwitchPage(double x, double y, bool is_right);

	virtual void draw();

	virtual bool is_active();

	virtual void effect(int, double, double);
};

class ToggleInformationDisplay : public Button {
public:
	ToggleInformationDisplay(double x, double y);

	virtual void draw();

	virtual bool is_active();

	virtual void effect(int, double, double);
};

class ChangeGameruleButton : public Button {
	bool on_foreground;
	bool* const gamerule_ptr;
	Surface* const sprite;

	char const* message[(int)LANGUAGE::NB_OF_LANGUAGE];

	const short animation_length = 1;
	const short period;
	short current_sprite = 0;
	short animation_counter = 0;
public:
	ChangeGameruleButton(double x, double y, bool* gamerule_ptr, Surface* sprite, bool on_foreground, short animation_length, short period, char const* msg0...);

	virtual void draw();

	virtual void effect(int, double, double);
};


class TeraButton : public Button {
	typing new_type;
	Piece* piece;

public:
	TeraButton(double x, double y, Piece* piece, typing type);

	virtual void draw();

	virtual void effect(int, double, double);
};