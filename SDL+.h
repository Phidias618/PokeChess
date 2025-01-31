#pragma once

class Surface;
class Rect;
class Display;

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>
#include <SDL_mixer.h>
#include "SDL_gfx/SDL_rotozoom.h"

#include <optional>
#include <cstdio>
#include <iostream>
#include <string>


#include "Debugger.h"

#define self (*this)

enum anchor : char {
	top_left = 0b0000,
	top_middle = 0b0001,
	top_right = 0b0010,
	middle_left = 0b0100,
	center = 0b0101,
	middle_right = 0b0110,
	bottom_left = 0b1000,
	bottom_middle = 0b1001,
	bottom_right = 0b1010,
};

class Color { // Represents ARGB8888 colors
	Uint32 color;
public:
	constexpr inline Color() {
		color = 0;
	}

	constexpr inline Color(Uint32 value) {
		color = value;
	}

	constexpr inline Color(SDL_Color c) {
		color = (c.a << 24) | (c.r << 16) | (c.g << 8) | c.b;
	}

	inline constexpr operator Uint32() const {
		return color;
	}

	inline constexpr operator SDL_Color() const {
		return { (Uint8)(color >> 16), (Uint8)(color >> 8), (Uint8)color, (Uint8)(color >> 24) };
	}

	static const Color red;
	static const Color green;
	static const Color blue;
	static const Color black;
	static const Color white;
	static const Color aqua;
	static const Color yellow;
	static const Color magenta;

	static const Color silver;
	static const Color grey;
	static const Color beige;
	static const Color orange;
	static const Color pale_yellow;
	static const Color sky;
	
};

const inline constexpr Color Color::red = 0xFFFF0000;

const inline constexpr Color Color::green = 0xFF00FF00;
const inline constexpr Color Color::blue = 0xFF0000FF;
const inline constexpr Color Color::black = 0xFF000000;
const inline constexpr Color Color::white = 0xFFFFFFFF;
const inline constexpr Color Color::aqua = 0xFF00FFFF;
const inline constexpr Color Color::yellow = 0xFFFFFF00;
const inline constexpr Color Color::magenta = 0xFFFF00FF;

const inline constexpr Color Color::silver = 0xFFC0C0C0;
const inline constexpr Color Color::grey = 0xFF808080;
const inline constexpr Color Color::beige = 0xFFF5F5DC;
const inline constexpr Color Color::orange = 0xFFFFA500;
const inline constexpr Color Color::pale_yellow = 0xBBDFD869;
const inline constexpr Color Color::sky = 0xFF82CBE5;

inline constexpr SDL_Color Uint32_to_color(Color color) {
	return { (Uint8)((Uint32)color >> 16), (Uint8)((Uint32)color >> 8), (Uint8)color, (Uint8)((Uint32)color >> 24) };
}

template<typename T>
inline constexpr T true_mod(T x, T y) {
	return ((x % y) + y) % y;
}

bool SDL_plus_init(SDL_InitFlags sdl_flags, IMG_InitFlags img_flags=0, MIX_InitFlags mix_flags=0);

void SDL_plus_quit();

class Surface {
private:
	SDL_Surface* surface;
public:
	inline operator SDL_Surface* () {
		return surface;
	}

	inline Surface(SDL_Surface* sdl_surface) {
		surface = sdl_surface;
	}

	inline Surface() {
		surface = NULL;
	}
	
	Surface(const Surface& other) {
		surface = other.surface;
		INC_REF();
	}

	Surface& operator=(const Surface& other) {
		DEC_REF();
		surface = other.surface;
		INC_REF();
		return self;
	}

	Surface& operator=(SDL_Surface* other) {
		DEC_REF();
		surface = other;
		INC_REF();
		return self;
	}

	Surface(Surface&& other) noexcept {
		surface = other.surface;
		other.surface = NULL;
	}
	inline Surface copy() {
		Surface new_surface = SDL_CreateSurface(surface->w, surface->h, surface->format);
		SDL_SetSurfacePalette(new_surface, SDL_GetSurfacePalette(surface));
		blit(new_surface, NULL, NULL);
		return NULL;
	}

	Surface(const Surface& other, SDL_Rect* area) {
		surface = SDL_CreateSurface(area->w, area->h, other->format);
		SDL_SetSurfacePalette(surface, SDL_GetSurfacePalette(other.surface));
		SDL_Rect dest{ 0, 0, area->w, area->h };
		blit(other, &dest, area);
	}

	// use to manually free the ressources allocated to that surface, 
	// it is not necessary as the destructor calls free when called
	inline void free() { DEC_REF(); }

	inline SDL_PixelFormat get_format() {
		return surface->format;
	}

	inline int blit(Surface source, SDL_Rect* dest, const SDL_Rect* area=NULL, anchor c=top_left) {
		if (dest != NULL) {
			dest->x -= ((area == NULL)? source->w: area->w) * (c & 0b11) / 2;
			dest->y -= ((area == NULL) ? source->h : area->h) * (c >> 2) / 2;
		}
		return SDL_BlitSurface(source, area, surface, dest);
	}

	inline SDL_Surface* operator->() const {
		return surface;
	}

	inline Surface convertTo(SDL_PixelFormat fmt) const {
		return (Surface)SDL_ConvertSurface(surface, fmt);
	}

	static Surface createRGBA(int width, int height) {
		return (Surface)SDL_CreateSurface(width, height, SDL_PIXELFORMAT_ARGB8888);
	}

	inline static Surface createRGB(int width, int height) {
		return (Surface)SDL_CreateSurface(width, height, SDL_PIXELFORMAT_XRGB8888);
	}

	~Surface() {
		DEC_REF();
	}

	void inline clear() {
		SDL_FillSurfaceRect(*this, NULL, 0x0);
	}

	inline void fill(Uint32 color) {
		SDL_FillSurfaceRect(surface, NULL, color);
	}

	inline void lock() {
		SDL_LockSurface(surface);
	}
	
	inline bool MUSTLOCK() {
		return SDL_MUSTLOCK(surface);
	}

	inline void unlock() {
		SDL_UnlockSurface(surface);
	}
	
	inline Uint32 map_rgb(Uint8 r, Uint8 g, Uint8 b) const {
		return SDL_MapRGB(SDL_GetPixelFormatDetails(surface->format), SDL_GetSurfacePalette(surface), r, g, b);
	}

	inline Uint32 map_rgba(Uint8 r, Uint8 g, Uint8 b, Uint8 a) const {
		return SDL_MapRGBA(SDL_GetPixelFormatDetails(surface->format), SDL_GetSurfacePalette(surface), r, g, b, a);
	}

	inline void set_colorkey(Uint32 key, bool enabled=true) {
		SDL_SetSurfaceColorKey(surface, enabled, key);
	}

	inline void set_alpha(Uint8 alpha) {
		SDL_SetSurfaceAlphaMod(surface, alpha);
	}

	inline std::optional<Uint8> get_alpha() {
		Uint8 alpha;
		if (SDL_GetSurfaceAlphaMod(surface, &alpha))
			return alpha;
	}

	inline std::optional<Uint32> get_colorkey() {
		Uint32 ret;
		if (SDL_GetSurfaceColorKey(surface, &ret) == 0)
			return ret;
	}

	inline SDL_Rect get_rect(int x=0, int y=0) {
		return { x, y, surface->w, surface->h };
	}

	Surface scale_to(int width, int height, bool smooth);

	Surface scale_by(double factor, bool smooth);

	Surface chop(SDL_Rect rect) const {
		Surface new_surface = SDL_CreateSurface(rect.w, rect.h, surface->format);
		SDL_SetSurfacePalette(new_surface, SDL_GetSurfacePalette(surface));

		new_surface.blit(self, NULL, &rect);

		return new_surface;
	}

	Surface& inplace_scroll(int dx, int dy);

	Surface& inplace_toric_scroll(int dx, int dy);

	Surface rotate90(); // return a new surface rotated 90° counter-clockwise
	inline Surface rotate180() { return copy().rotate180_inplace(); } // return a new surface rotated 180°
	Surface& rotate180_inplace(); // rotate the internal surface 180° and returns itself
	Surface rotate270(); // return a new surface rotated 270° counter-clockwise

	Surface& h_flip_inplace();
	Surface& v_flip_inplace();
private:
	template<int PIXEL_SIZE> Surface __rotate90();
	template<int PIXEL_SIZE> Surface& __rotate180_inplace();
	template<int PIXEL_SIZE> Surface __rotate270();
	template<int PIXEL_SIZE> Surface& __h_flip_inplace();
	template<int PIXEL_SIZE> Surface& __v_flip_inplace();

	inline void INC_REF() { 
		if (surface != NULL) {
			surface->refcount++;
		}
	}
	inline void DEC_REF() { 
		if (surface != NULL) {
			SDL_DestroySurface(surface);
			surface = NULL;
		}
	}
public:

	void draw_disk(int center_x, int center_y, int radius, Uint32 color);

	friend class Display;
	friend inline Surface load_img(const char* path);


};

class Display {
private:
	SDL_Window* window;
	size_t* refcount;
public:

	inline Display() {
		refcount = NULL;
		window = NULL;
	}

	inline Display(SDL_Window* SDL_window) {
		window = SDL_window;
		refcount = new size_t(1);
	}

	inline Display(const char* title, int width, int height, Uint32 flags = 0) {
		window = SDL_CreateWindow(title, width, height, flags);
		refcount = new size_t(1);
	}

	inline Display(const Display& other) {
		window = other.window;
		refcount = other.refcount;
		INC_REF();
	}

	inline Display(Display&& other) noexcept {
		window = other.window;
		refcount = other.refcount;

		other.window = NULL;
		other.refcount = NULL;
	}

	inline Display& operator=(const Display& other) {
		DEC_REF();
		window = other.window;
		refcount = other.refcount;
		INC_REF();
		return self;
	}


	inline operator SDL_Window* () {
		return window;
	}

	inline bool operator==(const SDL_Window* other) {
		return window == other;
	}


	inline Surface get_surface() {
		return SDL_GetWindowSurface(window);
	}

	inline void refresh(){ 
		SDL_UpdateWindowSurface(window); 
	}

	inline void set_title(const char* title) {
		SDL_SetWindowTitle(window, title);
	}

	inline void destroy() {
		if (window != NULL) {
			SDL_DestroyWindow(window);
			window = NULL;
			delete refcount;
			refcount = NULL;
		}
	}

	~Display() {
		DEC_REF();
	}

private:
	inline void INC_REF() {
		if (refcount != NULL)
			++*refcount;
	}

	inline void DEC_REF() {
		if (window != NULL and refcount != NULL) {
			--*refcount;
			if (*refcount == 0) {
				SDL_DestroyWindow(window);
				delete refcount;
			}
			window = NULL;
			refcount = NULL;
		}
	}
};

Surface load_img(const char* path) {
	Surface ret = IMG_Load(path);
	std::cout << "load img at: " << path << '\n';

	if (ret == NULL)
		throw std::exception("null");
	return (Surface)ret;
}

class Font {
private:
	static const constexpr inline size_t TTF_QUIT_FLAG = (((size_t)1) << (8 * (sizeof size_t) - 1));
	static const constexpr inline size_t REFCOUNT_MASK = ~TTF_QUIT_FLAG;
	TTF_Font* font;
	size_t* refcount; // stores wether this font need to call TTF_Quit when deleted in the 64th bit of *ref_count (no pointer is tagged)
public:
	inline Font() {
		font = NULL;
		refcount = NULL;
	}

	inline Font(TTF_Font* other) {
		font = other;
		refcount = new size_t(1);
	}

	inline Font(const char* path, float ptsize) {
		TTF_Init();
		font = TTF_OpenFont(path, ptsize);
		refcount = new size_t(1|TTF_QUIT_FLAG);
	}

	inline Font(const Font& other) {
		font = other.font;
		refcount = other.refcount;
		INC_REF();
	}

	inline Font(Font&& other) noexcept {
		font = other.font;
		refcount = other.refcount;

		other.font = NULL;
		other.refcount = NULL;
	}

	inline Font& operator=(const Font& other) {
		DEC_REF();
		font = other.font;
		refcount = other.refcount;
		INC_REF();
		return self;
	}

	inline Font& operator=(TTF_Font* other) {
		DEC_REF();
		font = other;
		refcount = new size_t(1);
		return self;
	}

	inline bool operator==(TTF_Font* other) {
		return font == other;
	}

	inline Surface render_solid(const char* txt, SDL_Color fg) {
		return TTF_RenderText_Solid(font, txt, 0, fg);
	}

	inline Surface render_shaded(const char* txt, SDL_Color fg, SDL_Color bg) {
		return TTF_RenderText_Shaded(font, txt, 0, fg, bg);
	}

	inline Surface render_blended(const char* txt, SDL_Color fg) {
		return TTF_RenderText_Blended(font, txt, 0, fg);
	}

	inline Surface render_solid_wrapped(const char* txt, SDL_Color fg, Uint32 wrapLength) {
		return TTF_RenderText_Solid_Wrapped(font, txt, 0, fg, wrapLength);
	}

	inline Surface render_shaded_wrapped(const char* txt, SDL_Color fg, SDL_Color bg, Uint32 wrapLength) {
		return TTF_RenderText_Shaded_Wrapped(font, txt, 0, fg, bg, wrapLength);
	}

	inline Surface render_blended_wrapped(const char* txt, SDL_Color fg, Uint32 wrapLength) {
		return TTF_RenderText_Blended_Wrapped(font, txt, 0, fg, wrapLength);
	}

	inline operator TTF_Font* () {
		return font;
	}

	inline void free() {
		DEC_REF();
	}

	inline ~Font() {
		DEC_REF();
	}

private:
	inline void INC_REF() {
		if (refcount != NULL) {
			++*refcount;
		}
	}

	inline void DEC_REF() {
		if (refcount != NULL) {
			--*refcount;

			if ((*refcount & REFCOUNT_MASK) == 0) {
				TTF_CloseFont(font);
				if ((*refcount & TTF_QUIT_FLAG) != 0)
					TTF_Quit();
				delete refcount;
			}

			font = NULL;
			refcount = NULL;
		}
	}
};