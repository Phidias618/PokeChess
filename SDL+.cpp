#include <SDL.h>
#include <stdio.h>
#include <optional>
#include "SDL+.h"

#define self (*this)

#define implicit

#define useless0 1
#define useless1 useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0 + useless0
#define useless2 useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1 + useless1
#define useless3 useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2 + useless2

int inline constexpr MAX(int a, int b) {
	return (a >= b) ? a : b;
}

int inline constexpr MIN(int a, int b) {
	return (a <= b) ? a : b;
}

bool SDL_plus_init(SDL_InitFlags sdl_flags, IMG_InitFlags img_flags, MIX_InitFlags mix_flags) {
	if (not SDL_Init(sdl_flags)) { // check if any flag you wanted to initialzed is not
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	if ((mix_flags & ~Mix_Init(mix_flags)) != 0) {
		return false;
	}

	if ((img_flags & ~IMG_Init(img_flags)) != 0) {
		return false;
	}

	//SDL_AudioSpec audio_spec = { SDL_AUDIO_UNKNOWN, 4, 44100 };

	if (not Mix_OpenAudio(0, NULL)) {
		return false;
	}

	if (not TTF_Init()) {
		printf("SDL_ttf could not initialize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	return true;
}

void SDL_plus_quit() {
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}


auto Surface::scale_to(int w2, int h2, bool smooth) -> Surface {
	if (surface == NULL)
		return NULL;
	double w1 = surface->w;
	double h1 = surface->h;

	Surface result = zoomSurface(surface, (double)w2 / w1, (double)h2 / h1, smooth);
	
	if (result == NULL) {
		PRINT_DEBUG("scale_to qui marche pas");
		throw std::exception("rotozoom failed");
	}

	return result;
}

Surface Surface::scale_by(double factor, bool smooth) {
	if (surface == NULL)
		return NULL;
	Surface result = zoomSurface(surface, factor, factor, smooth);
	if (result == NULL) {
		PRINT_DEBUG("scale_by qui marche pas");
		throw std::exception("rotozoom failed");
	}

	return result;
}

void Surface::draw_disk(int center_x, int center_y, int radius, Uint32 color) {
	int radius_squared = radius * radius;
	int x = MAX(-center_x, -radius);
	int x_end = MIN(surface->w - 1 - center_x, radius);
	int y_begin = MAX(-center_y, -radius);
	int y_end = MIN(surface->h - 1 - center_y, radius);
	if (SDL_MUSTLOCK(surface))
		lock();
	Uint32* arr = (Uint32*)surface->pixels;
	for (; x <= x_end; x++) {
		int x_squared = x * x;
		for (int y = y_begin; y <= y_end; y++) {
			if (x_squared + y * y <= radius_squared) {
				arr[(x + center_x) + (y + center_y) * surface->w] = color;
			}
		}
	}
	if (SDL_MUSTLOCK(surface))
		unlock();
}


Surface& Surface::inplace_scroll(int dx, int dy) {
	int w = surface->w;
	int h = surface->h;
	if (abs(dx) >= w or abs(dy) >= h)
		fill(0x00000000);
	else {
		if (SDL_MUSTLOCK(surface))
			lock();

		Uint8* arr = (Uint8*)surface->pixels;
		dx *= SDL_BYTESPERPIXEL(surface->format) ; // accounts for the fact that the pixel array is of type Uint8, while the surface can use Uint32 as colors

		const int row_size = surface->pitch;
		
		if (dx > 0) {
			// shift all pixels to the right
			int y_offset = 0;
			for (int y = 0; y < h; y++) {
				int x = row_size;
				while (x-->dx) { // downto operator
					arr[x + y_offset] = arr[x - dx + y_offset]; // shift all pixels to the right
				}
				memset(arr+y_offset, 0, dx); // fill the gap created in the left with black pixels

				y_offset += row_size;
			}
		} 
		else if (dx < 0) {
			// shift all pixels to the left
			int y_offset = 0;
			for (int y = 0; y < h; y++) {
				int x_end = w + dx;
				for (int x = 0; x < x_end; x++) {
					arr[x + y_offset] = arr[x - dx + y_offset]; // shift all pixels to the left
				}
				memset(arr + x_end + y_offset, 0, -dx); // fill the gap created in the right with black pixels

				y_offset += row_size;
			}
		} // no case for dx == 0 as it would not change anything to the surface


		if (dy > 0) {
			//shift all pixels down
			int y = h;
			Uint8* dest = arr + row_size * h;
			Uint8* source = dest - row_size * dy;
			while (source != arr) {
				dest -= row_size;
				source -= row_size;
				memcpy(dest, source, row_size);
			}

			memset(arr, 0, dy * row_size); // fill the gap created at the top with black pixels
		}
		else if (dy < 0) {
			// shift all pixels up

			Uint8* source = arr - row_size * dy;
			Uint8* dest = arr;
			const Uint8* end = arr + row_size * h;
			while (source != end) {
				memcpy(dest, source, row_size);
				dest += row_size;
				source += row_size;
			}
			memset(dest, 0, -row_size * dy);
		}

		if (SDL_MUSTLOCK(surface))
			unlock();
	}

	return self;
}




Surface& Surface::inplace_toric_scroll(int dx, int dy) {
	int w = surface->w;
	int h = surface->h;
	dx = true_mod(dx, surface->w);
	dy = true_mod(dy, surface->h);
	if (dy > 0) {
		Surface bottom_part = chop({ 0, h - dy, w, dy });
		inplace_scroll(0, dy);
		blit(bottom_part, NULL);
	}
	else if (dy < 0) {
		Surface top_part = chop({ 0, 0, w, -dy });
		inplace_scroll(0, dy);
		SDL_Rect r = {0, h+dy, 0, 0};
		blit(top_part, &r);
	}

	if (dx > 0) {
		Surface right_part = chop({w-dx, 0, dx, h});
		inplace_scroll(dx, 0);
		blit(right_part, NULL);
	}
	else if (dx < 0) {
		Surface left_part = chop({ 0, 0, -dx, h });
		inplace_scroll(dx, 0);
		SDL_Rect r = { w + dx, 0, 0, 0 };
		blit(left_part, &r);
	}

	return self;
}

template<int PIXEL_SIZE> Surface Surface::__rotate90() {
	static_assert(PIXEL_SIZE == 1 or PIXEL_SIZE == 4, "Quoi-feur\n");
	using UINT = std::conditional_t<PIXEL_SIZE == 1, Uint8, Uint32>;

	if (MUSTLOCK())
		lock();
	Surface new_surface = SDL_CreateSurface(surface->h, surface->w, surface->format);
	SDL_SetSurfacePalette(new_surface, SDL_GetSurfacePalette(surface));

	if (new_surface.MUSTLOCK())
		new_surface.lock();

	UINT* src = (UINT*)surface->pixels;
	UINT* dest = (UINT*)new_surface->pixels;

	for (int x1 = 0, x2 = surface->w - 1; x2 >= 0; x1++, x2--) {
		for (int y = 0; y < surface->h; y++) {
			dest[x2 * new_surface->pitch / PIXEL_SIZE + y] = src[y * surface->pitch / PIXEL_SIZE + x1];
		}
	}

	if (new_surface.MUSTLOCK())
		new_surface.unlock();

	if (MUSTLOCK())
		unlock();
	return new_surface;
}

template<typename T>
inline void swap(T* v1, T* v2) {
	T tmp = *v1;
	*v1 = *v2;
	*v2 = tmp;
}

template<const int PIXEL_SIZE>
Surface& Surface::__rotate180_inplace() {
	static_assert(PIXEL_SIZE == 1 or PIXEL_SIZE == 4, "Quoi-feur\n");
	using UINT = std::conditional_t<PIXEL_SIZE == 1, Uint8, Uint32>;

	if (MUSTLOCK())
		lock();

	UINT* pixels = (UINT*)surface->pixels;

	for (int x1 = 0, x2 = surface->w - 1; x2 >= x1; x1++, x2--) {
		for (int y1 = 0, y2 = surface->h - 1; y2 >= 0; y1++, y2--) {
			swap(&pixels[y1 * surface->pitch / PIXEL_SIZE + x1], &pixels[y2 * surface->pitch / PIXEL_SIZE + x2]);
		}
	}

	if (MUSTLOCK())
		unlock();

	return self;
}

Surface Surface::rotate90() {
	switch (SDL_GetPixelFormatDetails(get_format())->bytes_per_pixel) {
	case 1: 
		return __rotate90<1>();
	case 4:
		return __rotate90<4>();
	}
}

Surface& Surface::rotate180_inplace() {
	switch (SDL_GetPixelFormatDetails(get_format())->bytes_per_pixel) {
	case 1:
		return __rotate180_inplace<1>();
	case 4:
		return __rotate180_inplace<4>();
	}
}

template<int PIXEL_SIZE>
Surface Surface::__rotate270() {
	static_assert(PIXEL_SIZE == 1 or PIXEL_SIZE == 4, "Quoi-feur\n");
	using UINT = std::conditional_t<PIXEL_SIZE == 1, Uint8, Uint32>;

	if (MUSTLOCK())
		lock();
	Surface new_surface = SDL_CreateSurface(surface->h, surface->w, surface->format);
	SDL_SetSurfacePalette(new_surface, SDL_GetSurfacePalette(surface));

	if (new_surface.MUSTLOCK())
		new_surface.lock();

	UINT* src = (UINT*)surface->pixels;
	UINT* dest = (UINT*)new_surface->pixels;

	for (int x = 0; x < surface->w; x++) {
		for (int y1 = 0, y2 = surface->h-1; y2>0; y1++, y2--) {
			dest[x * new_surface->pitch / PIXEL_SIZE + y2] = src[y1 * surface->pitch / PIXEL_SIZE + x];
		}
	}

	if (new_surface.MUSTLOCK())
		new_surface.unlock();

	if (MUSTLOCK())
		unlock();
	return new_surface;
}

Surface Surface::rotate270() {
	switch (SDL_GetPixelFormatDetails(get_format())->bytes_per_pixel) {
	case 1:
		return __rotate270<1>();
	case 4:
		return __rotate270<4>();
	}
}

template<int PIXEL_SIZE>
Surface& Surface::__h_flip_inplace() {
	static_assert(PIXEL_SIZE == 1 or PIXEL_SIZE == 4, "Quoi-feur\n");
	using UINT = std::conditional_t<PIXEL_SIZE == 1, Uint8, Uint32>;

	if (MUSTLOCK())
		lock();

	UINT* pixels = (UINT*)surface->pixels;

	const int step = surface->pitch / PIXEL_SIZE;

	for (int x = 0; x < surface->w; x++, pixels++) {
		for (UINT *begin = pixels, *end=(pixels + step * (surface->h-1)); begin < end; begin += step, end -= step) {
			swap(begin, end);
		}
	}

	if (MUSTLOCK())
		unlock();

	return self;
}

template<int PIXEL_SIZE>
Surface& Surface::__v_flip_inplace() {
	static_assert(PIXEL_SIZE == 1 or PIXEL_SIZE == 4, "Quoi-feur\n");
	using UINT = std::conditional_t<PIXEL_SIZE == 1, Uint8, Uint32>;

	if (MUSTLOCK())
		lock();

	UINT* pixels = (UINT*)surface->pixels;

	const int step = surface->pitch / PIXEL_SIZE;

	for (int y = 0; y < surface->h; y++, pixels+=step) {
		for (UINT *begin = pixels, *end = (pixels + surface->w - 1); begin < end; begin++, end--) {
			swap(begin, end);
		}
	}

	if (MUSTLOCK())
		unlock();

	return self;
}

Surface& Surface::h_flip_inplace() {
	switch (SDL_GetPixelFormatDetails(get_format())->bytes_per_pixel) {
	case 1:
		return __h_flip_inplace<1>();
	case 4:
		return __h_flip_inplace<4>();
	}
}

Surface& Surface::v_flip_inplace() {
	switch (SDL_GetPixelFormatDetails(get_format())->bytes_per_pixel) {
	case 1:
		return __v_flip_inplace<1>();
	case 4:
		return __v_flip_inplace<4>();
	}
}