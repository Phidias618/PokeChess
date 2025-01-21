#pragma once
#define IN_DEBUG true
#include <iostream>

#if IN_DEBUG
#define PRINT_DEBUG(truc) std::cout << truc << '\n';

#define PRINT_VAR(var) std::cout << #var << " = " << var << '\n';

#else
#define PRINT_DEBUG(truc)
#define PRINT_VAR(var)
#endif