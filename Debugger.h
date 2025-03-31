#pragma once
#define IN_DEBUG false
#include <iostream>

#if IN_DEBUG
static void PRINT_DEBUG() {
	std::cout << '\n';
}

template<typename T1, typename... Args>
static inline void PRINT_DEBUG(T1 val, Args... other) {
	std::cout << val << ' ';
	PRINT_DEBUG(other...);
}

#define PRINT_VAR(var) std::cout << #var << " = " << var << '\n';

#else

#define PRINT_DEBUG(...)
#define PRINT_VAR(var)

#endif

#define ENABLE_SAFETY_CHECKS true

#define SHOW_DRAWING_TIME false
#define SHOW_BUTTON_HITBOX false