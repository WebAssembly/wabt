# string-view-lite

`string-view-lite` is an implementation of `std::string_view` that works in
C++98 and later compilers. The original source can be found at
https://github.com/martinmoene/string-view-lite.

This version copy has been modified as follows. See `string_view.hpp.patch` for
details:

- All `throw` statements have been replaced by a macro `nssv_throw`, so the
  library can be used with `-fno-exceptions`.
- Fix a bug in `find_last_of` and `find_last_not_of` with an empty
  `string_view`.
