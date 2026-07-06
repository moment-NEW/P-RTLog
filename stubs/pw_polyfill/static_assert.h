// Stub: pw_polyfill/static_assert.h
// C11 and C++11 both provide static_assert natively.
#pragma once

// C11 <assert.h> provides static_assert as a macro.
// In C++11 it is a keyword. No extra definition needed.
#ifdef __cplusplus
#include <cassert>
#else
#include <assert.h>
#endif
