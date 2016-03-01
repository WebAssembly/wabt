#pragma once

#ifdef __GNUC__
# include <alloca.h>
#elif defined _MSC_VER
# include <malloc.h>
# define alloca _alloca
#endif