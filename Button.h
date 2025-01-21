#pragma once

#include <queue>

#include "BaseButton.h"

class Button;
class ButtonCollection;
class BeginGameButton;
class EndOfGameButton;
class ExitGameButton;
class PromotionButton;
class ChangeGameruleButton;
class TypingSelectionButton;
class ConfirmSelectionButton;
class RandomTypingButton;
class TextBoxDisplay;

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

	enum update_return_code : char {
		nothing = 0,
		suicide,
	};
public:

	Button(double tile_x_, double tile_y_, double tile_w_, double tile_h_);

	virtual update_return_code update() { return Button::nothing; }
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
	bool is_cleared;

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

public:
	TypingSelectionButton(double x_, double y_);

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

	virtual Button::update_return_code update();

	virtual void draw();

	virtual void effect(int, double, double);
};

class TextBoxDisplay : public Button {
	Surface sprite;
	short visual_index;
	short text_index;
	short timer;
	bool is_on_second_line : 1;
	piece_color side : 2;


	static std::queue<TextBoxDisplay*> queue;

	static inline const int constexpr BUFFER_SIZE = 84;
	char message[BUFFER_SIZE];

	static const short begin_x = 18;
	static const short begin_y = 32;
	static const short x_pixel_increment = 16;
	static const short y_pixel_increment = 32;
public:
	static const int duration = FPS;

	static void clear() {
		if (queue.empty())
			return;
		for (TextBoxDisplay* box = queue.front(); not queue.empty(); queue.pop()) {
			if (box != NULL) {
				box->kill();
				// delete box;
			}
		}
	}

	TextBoxDisplay(const char* text);

	virtual void activate();

	virtual Button::update_return_code update();

	virtual bool is_active();

	virtual void draw();
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

	virtual Button::update_return_code update();

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

	virtual Button::update_return_code update();

	virtual void draw();

	virtual void effect(int, double, double);
};


class DisableRNGButton : public Button {
public:
	DisableRNGButton(double x_, double y_);

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

class RandomBattleButton : public Button {
	static const int period = FPS / 8;
	short current_sprite;
	short counter;
public:
	RandomBattleButton(double x_, double y_);

	virtual Button::update_return_code update();

	virtual void draw();

	virtual void effect(int, double, double);
};

class BoardButton : public Button {
	Board& board;
public:
	BoardButton(Board& b);

	virtual void draw();
	virtual void effect(int, double, double);
	virtual void unhold(int, double, double);
};