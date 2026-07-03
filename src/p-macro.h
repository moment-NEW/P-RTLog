#pragma once

// p-macro.h - Consolidated preprocessor macros (renamed from P_ prefix)
// Original source: pigweed pw_preprocessor module (Apache 2.0)


// ========================================
// === from boolean.h ===
// ========================================

// Copyright 2019 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.



// Preprocessor boolean operation macros that evaluate to 0 or 1.
//
// These macros perform boolean operations in the C preprocessor that evaluate
// to a literal 1 or 0. They can be used for a few purposes:
//
//   - Generate other macros that evaluate to a 1 or 0, instead of a
//     parenthesized boolean expression.
//   - Ensure that the operands are defined and evaluate to 1 or 0 themselves.
//   - Write macros that conditionally use other macros by token pasting the
//     resulting 1 or 0 to form a new macro name.
//
// These macros should not be used outside of macro definitions. Use normal C
// operators (&&, ||, !, ==, !=) instead. For example, to check whether two
// flags are set, the C operators are the best choice:
//
//   #if RELEASE && OPTIMIZED
//
// However, there are cases when a literal 0 or 1 is required. For example:
//
//   #define SELECT_ALGORITHM() P_CONCAT(ALGO_, P_AND(RELEASE, OPTIMIZED))
//
// SELECT_ALGORITHM evaluates to ALGO_0 or ALGO_1, depending on whether RELEASE
// and OPTIMIZED are set to 1.

// Boolean AND of two preprocessor expressions that evaluate to 0 or 1.
#define P_AND(a, b) _P_AND(a, b)      // Expand the macro an extra time to
#define _P_AND(a, b) _P_AND_##a##b()  // allow macro substitution to occur.
#define _P_AND_00() 0
#define _P_AND_01() 0
#define _P_AND_10() 0
#define _P_AND_11() 1

// Boolean OR of two preprocessor expressions that evaluate to 0 or 1.
#define P_OR(a, b) _P_OR(a, b)
#define _P_OR(a, b) _P_OR_##a##b()
#define _P_OR_00() 0
#define _P_OR_01() 1
#define _P_OR_10() 1
#define _P_OR_11() 1

// Boolean NOT of a preprocessor expression that evaluates to 0 or 1.
#define P_NOT(value) _P_NOT(value)
#define _P_NOT(value) _P_NOT_##value()
#define _P_NOT_0() 1
#define _P_NOT_1() 0

// Boolean XOR of two preprocessor expressions that evaluate to 0 or 1.
#define P_XOR(a, b) _P_XOR(a, b)
#define _P_XOR(a, b) _P_XOR_##a##b()
#define _P_XOR_00() 0
#define _P_XOR_01() 1
#define _P_XOR_10() 1
#define _P_XOR_11() 0

// Boolean NAND, NOR, and XNOR of expressions that evaluate to 0 or 1.
#define P_NAND(a, b) P_NOT(P_AND(a, b))
#define P_NOR(a, b) P_NOT(P_OR(a, b))
#define P_XNOR(a, b) P_NOT(P_XOR(a, b))


// ========================================
// === from util.h ===
// ========================================

// Copyright 2019 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.
//
// Small, general preprocessor macros for C and C++ code.


// Returns the number of elements in a C array.
#define P_ARRAY_SIZE(array) (sizeof(array) / sizeof(*array))

// Returns a string literal of the arguments after expanding macros.
#define P_STRINGIFY(...) _P_STRINGIFY(__VA_ARGS__)
#define _P_STRINGIFY(...) #__VA_ARGS__

#ifdef __cplusplus

// Macro for inline extern "C" declarations. The following will compile
// correctly for C and C++:
//
//   P_EXTERN_C ThisFunctionHasCLinkage(void);
//
#define P_EXTERN_C extern "C"

// Macros for opening and closing an extern "C" block. This avoids the need for
// an #ifdef __cplusplus check around the extern "C" { and closing }. Example:
//
//   P_EXTERN_C_START
//
//   void FunctionDeclarationForCppAndC(void);
//
//   void AnotherFunctionDeclarationForCppAndC(int, char);
//
//   P_EXTERN_C_END
//
#define P_EXTERN_C_START extern "C" {
#define P_EXTERN_C_END }  // extern "C"

#else  // extern "C" is removed from C code

#define P_EXTERN_C
#define P_EXTERN_C_START
#define P_EXTERN_C_END

#endif  // __cplusplus


// ========================================
// === from compiler.h ===
// ========================================

// Copyright 2019 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.
//
// Preprocessor macros that wrap compiler-specific features.
// This file is used by both C++ and C code.


// TODO: b/234877280 - compiler.h should be refactored out of pw_preprocessor as
// the scope is outside of the module. Perhaps it should be split up and placed
// under pw_compiler, e.g. pw_compiler/attributes.h & pw_compiler/builtins.h.

#include "pw_polyfill/static_assert.h"

/// @submodule{pw_preprocessor,compiler}

/// Marks a struct or class as packed.
///
/// Use packed structs with extreme caution! Packed structs are rarely needed.
/// Instead, define the struct and `static_assert` to verify that the size and
/// alignement are as expected.
///
/// Packed structs should only be used to avoid standard padding or to force
/// unaligned members when describing in-memory or wire format data structures.
/// Packed struct members should NOT be accessed directly because they may be
/// unaligned. Instead, `memcpy` the fields into variables. For example:
///
/// @code{.cpp}
///   P_PACKED(struct) PackedStruct {
///     uint8_t a;
///     uint32_t b;
///     uint16_t c;
///   };
///
///   void UsePackedStruct(const PackedStruct& packed_struct) {
///     uint8_t a;
///     uint32_t b;
///     uint16_t c;
///     std::memcpy(&a, &packed_struct.a, sizeof(a));
///     std::memcpy(&b, &packed_struct.b, sizeof(b));
///     std::memcpy(&c, &packed_struct.c, sizeof(c));
///   }
/// @endcode
#define P_PACKED(declaration) declaration __attribute__((packed))

/// Marks a function or object as used, ensuring code for it is generated.
#define P_USED __attribute__((used))

/// Prevents generation of a prologue or epilogue for a function. This is
/// helpful when implementing the function in assembly.
#define P_NO_PROLOGUE __attribute__((naked))

/// Marks that a function declaration takes a printf-style format string and
/// variadic arguments. This allows the compiler to perform check the validity
/// of the format string and arguments. This macro must only be on the function
/// declaration, not the definition.
///
/// The format_index is index of the format string parameter and parameter_index
/// is the starting index of the variadic arguments. Indices start at 1. For C++
/// class member functions, add one to the index to account for the implicit
/// this parameter.
///
/// This example shows a function where the format string is argument 2 and the
/// varargs start at argument 3.
///
/// @code{.cpp}
///   int PrintfStyleFunction(char* buffer, const char* fmt, ...)
///   P_PRINTF_FORMAT(2,3);
///
///   int PrintfStyleFunction(char* buffer, const char* fmt, ...) { ...
///   implementation here ...  }
/// @endcode
#define P_PRINTF_FORMAT(format_index, parameter_index) \
  __attribute__((format(_P_PRINTF_FORMAT_TYPE, format_index, parameter_index)))

/// When compiling for host using MinGW, use gnu_printf() rather than printf()
/// to support %z format specifiers only if available (non-clang compilers).
/// @ingroup pw_preprocessor_internal
#if defined(__USE_MINGW_ANSI_STDIO) && !defined(__clang__)
#define _P_PRINTF_FORMAT_TYPE gnu_printf
#else
#define _P_PRINTF_FORMAT_TYPE printf
#endif  // defined(__USE_MINGW_ANSI_STDIO) && !defined(__clang__)

/// Places a variable in the specified linker section.
#ifdef __APPLE__
#define P_PLACE_IN_SECTION(name) __attribute__((section("__DATA," name)))
#else
#define P_PLACE_IN_SECTION(name) __attribute__((section(name)))
#endif  // __APPLE__

/// Places a variable in the specified linker section and directs the compiler
/// to keep the variable, even if it is not used. Depending on the linker
/// options, the linker may still remove this section if it is not declared in
/// the linker script and marked `KEEP`.
#ifdef __APPLE__
#define P_KEEP_IN_SECTION(name) __attribute__((section("__DATA," name), used))
#else
#define P_KEEP_IN_SECTION(name) __attribute__((section(name), used))
#endif  // __APPLE__

/// Indicate to the compiler that the annotated function won't return. Example:
///
/// @code{.cpp}
///   P_NO_RETURN void HandleAssertFailure(ErrorCode error_code);
/// @endcode
#define P_NO_RETURN __attribute__((noreturn))

/// Prevents the compiler from inlining a fuction.
#define P_NO_INLINE __attribute__((noinline))

/// Indicate to the compiler that the given section of code will not be reached.
/// Example:
///
/// @code{.cpp}
///   int main() {
///     InitializeBoard();
///     vendor_StartScheduler();  // Note: vendor forgot noreturn attribute.
///     P_UNREACHABLE;
///   }
/// @endcode
#define P_UNREACHABLE __builtin_unreachable()

/// Indicate to a sanitizer compiler runtime to skip the named check in the
/// associated function.
/// Example:
///
/// @code{.cpp}
///   uint32_t djb2(const void* buf, size_t len)
///       P_NO_SANITIZE("unsigned-integer-overflow") {
///     uint32_t hash = 5381;
///     const uint8_t* u8 = static_cast<const uint8_t*>(buf);
///     for (size_t i = 0; i < len; ++i) {
///       hash = (hash * 33) + u8[i]; /* hash * 33 + c */
///     }
///     return hash;
///   }
/// @endcode
#ifdef __clang__
#define P_NO_SANITIZE(check) __attribute__((no_sanitize(check)))
#else
#define P_NO_SANITIZE(check)
#endif  // __clang__

/// Wrapper around `__has_attribute`, which is defined by GCC 5+ and Clang and
/// evaluates to a non zero constant integer if the attribute is supported or 0
/// if not.
#ifdef __has_attribute
#define P_HAVE_ATTRIBUTE(x) __has_attribute(x)
#else
#define P_HAVE_ATTRIBUTE(x) 0
#endif  // __has_attribute

/// A function-like feature checking macro that accepts C++11 style attributes.
/// It is a wrapper around `__has_cpp_attribute`, which was introduced in the <a
/// href="https://en.cppreference.com/w/cpp/feature_test">C++20 standard</a>. It
/// is supported by compilers even if C++20 is not in use. Evaluates to a
/// non-zero constant integer if the C++ attribute is supported or 0 if not.
///
/// This is a copy of `ABSL_HAVE_CPP_ATTRIBUTE`.
#if defined(__cplusplus) && defined(__has_cpp_attribute)
#define P_HAVE_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define P_HAVE_CPP_ATTRIBUTE(x) 0
#endif  // defined(__cplusplus) && defined(__has_cpp_attribute)

/// @ingroup pw_preprocessor_internal
#define _P_REQUIRE_SEMICOLON \
  static_assert(1, "This macro must be terminated with a semicolon")

/// Starts a new group of @c_macro{P_MODIFY_DIAGNOSTIC} statements. A
/// @c_macro{P_MODIFY_DIAGNOSTICS_POP} statement must follow.
#define P_MODIFY_DIAGNOSTICS_PUSH() \
  _Pragma("GCC diagnostic push") _P_REQUIRE_SEMICOLON

/// @c_macro{P_MODIFY_DIAGNOSTIC} statements since the most recent
/// @c_macro{P_MODIFY_DIAGNOSTICS_PUSH} no longer apply after this statement.
#define P_MODIFY_DIAGNOSTICS_POP() \
  _Pragma("GCC diagnostic pop") _P_REQUIRE_SEMICOLON

/// Changes how a diagnostic (warning or error) is handled. Most commonly used
/// to disable warnings. ``P_MODIFY_DIAGNOSTIC`` should be used between
/// @c_macro{P_MODIFY_DIAGNOSTICS_PUSH} and @c_macro{P_MODIFY_DIAGNOSTICS_POP}
/// statements to avoid applying the modifications too broadly.
///
/// ``kind`` may be ``warning``, ``error``, or ``ignored``.
#define P_MODIFY_DIAGNOSTIC(kind, option) \
  P_PRAGMA(GCC diagnostic kind option) _P_REQUIRE_SEMICOLON

/// Applies ``P_MODIFY_DIAGNOSTIC`` only for GCC. This is useful for warnings
/// that aren't supported by or don't need to be changed in other compilers.
#ifdef __clang__
#define P_MODIFY_DIAGNOSTIC_GCC(kind, option) _P_REQUIRE_SEMICOLON
#else
#define P_MODIFY_DIAGNOSTIC_GCC(kind, option) \
  P_MODIFY_DIAGNOSTIC(kind, option)
#endif  // __clang__

/// Applies ``P_MODIFY_DIAGNOSTIC`` only for Clang. This is useful for warnings
/// that aren't supported by or don't need to be changed in other compilers.
#ifdef __clang__
#define P_MODIFY_DIAGNOSTIC_CLANG(kind, option) \
  P_MODIFY_DIAGNOSTIC(kind, option)
#else
#define P_MODIFY_DIAGNOSTIC_CLANG(kind, option) _P_REQUIRE_SEMICOLON
#endif  // __clang__

/// Expands to a `_Pragma` with the contents as a string. `_Pragma` must take a
/// single string literal; this can be used to construct a `_Pragma` argument.
#define P_PRAGMA(contents) _Pragma(#contents)

/// Marks a function or object as weak, allowing the definition to be overriden.
///
/// This can be useful when supporting third-party SDKs which may conditionally
/// compile in code, for example:
///
/// @code{.cpp}
///   P_WEAK void SysTick_Handler(void) {
///     // Default interrupt handler that might be overriden.
///   }
/// @endcode
#define P_WEAK __attribute__((weak))

/// Marks a weak function as an alias to another, allowing the definition to
/// be given a default and overriden.
///
/// This can be useful when supporting third-party SDKs which may conditionally
/// compile in code, for example:
///
/// @code{.cpp}
///   // Driver handler replaced with default unless overridden.
///   void USART_DriverHandler(void) P_ALIAS(DefaultDriverHandler);
/// @endcode
#define P_ALIAS(aliased_to) __attribute__((weak, alias(#aliased_to)))

/// `P_ATTRIBUTE_LIFETIME_BOUND` indicates that a resource owned by a function
/// parameter or implicit object parameter is retained by the return value of
/// the annotated function (or, for a parameter of a constructor, in the value
/// of the constructed object). This attribute causes warnings to be produced if
/// a temporary object does not live long enough.
///
/// When applied to a reference parameter, the referenced object is assumed to
/// be retained by the return value of the function. When applied to a
/// non-reference parameter (for example, a pointer or a class type), all
/// temporaries referenced by the parameter are assumed to be retained by the
/// return value of the function.
///
/// See also the upstream documentation:
/// https://clang.llvm.org/docs/AttributeReference.html#lifetimebound
///
/// This is a copy of `ABSL_ATTRIBUTE_LIFETIME_BOUND`.
#if P_HAVE_CPP_ATTRIBUTE(clang::lifetimebound)
#define P_ATTRIBUTE_LIFETIME_BOUND [[clang::lifetimebound]]
#elif P_HAVE_ATTRIBUTE(lifetimebound)
#define P_ATTRIBUTE_LIFETIME_BOUND __attribute__((lifetimebound))
#else
#define P_ATTRIBUTE_LIFETIME_BOUND
#endif  // P_ATTRIBUTE_LIFETIME_BOUND

/// Internal attribute; name and documentation TBD.
///
/// This is a copy of `ABSL_INTERNAL_ATTRIBUTE_CAPTURED_BY`.
#if P_HAVE_CPP_ATTRIBUTE(clang::lifetime_capture_by)
#define P_INTERNAL_ATTRIBUTE_CAPTURED_BY(Owner) \
  [[clang::lifetime_capture_by(Owner)]]
#else
#define P_INTERNAL_ATTRIBUTE_CAPTURED_BY(Owner)
#endif

/// `P_ATTRIBUTE_VIEW` indicates that a type is solely a "view" of data that it
/// points to, similarly to a span, string_view, or other non-owning reference
/// type.
/// This enables diagnosing certain lifetime issues similar to those enabled by
/// `P_ATTRIBUTE_LIFETIME_BOUND`, such as:
///
///   struct P_ATTRIBUTE_VIEW StringView {
///     template<class R>
///     StringView(const R&);
///   };
///
///   StringView f(std::string s) {
///     return s;  // warning: address of stack memory returned
///   }
///
/// This is a copy of `ABSL_ATTRIBUTE_VIEW`.
#if P_HAVE_CPP_ATTRIBUTE(gsl::Pointer)
#define P_ATTRIBUTE_VIEW [[gsl::Pointer]]
#else
#define P_ATTRIBUTE_VIEW
#endif  // P_ATTRIBUTE_VIEW

/// `P_ATTRIBUTE_OWNER` indicates that a type is a container, smart pointer, or
/// similar class that owns all the data that it points to.
/// This enables diagnosing certain lifetime issues similar to those enabled by
/// P_ATTRIBUTE_LIFETIME_BOUND, such as:
///
///   struct P_ATTRIBUTE_VIEW StringView {
///     template<class R>
///     StringView(const R&);
///   };
///
///   struct P_ATTRIBUTE_OWNER String {};
///
///   StringView f(String s) {
///     return s;  // warning: address of stack memory returned
///   }
///
/// This is a copy of `ABSL_ATTRIBUTE_OWNER`.
#if P_HAVE_CPP_ATTRIBUTE(gsl::Owner)
#define P_ATTRIBUTE_OWNER [[gsl::Owner]]
#else
#define P_ATTRIBUTE_OWNER
#endif  // P_ATTRIBUTE_OWNER

/// Evaluates to 1 if `__VA_OPT__` is supported, regardless of the C or C++
/// standard in use.
#if (defined(__clang_major__) && __clang_major__ < 9) || \
    (defined(__GNUC__) && __GNUC__ < 12)
#define P_VA_OPT_SUPPORTED() 0  // Don't bother checking on old compilers.
#else
#define P_VA_OPT_SUPPORTED() _P_VA_OPT_SUPPORTED()

#define _P_VA_OPT_SUPPORTED(...) _P_VA_OPT_SUPPORTED_##__VA_OPT__()
#define _P_VA_OPT_SUPPORTED_ 1
#define _P_VA_OPT_SUPPORTED___VA_OPT__() 0

#endif  // __clang_major__ < 9 || __GNUC__ < 12

/// `P_NO_UNIQUE_ADDRESS` indicates that an object can have no unique
/// address. This is usually used to force empty member types to occupy
/// 0 bytes instead of 1 byte to have a unique address.
#if P_HAVE_CPP_ATTRIBUTE(no_unique_address)
#define P_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#define P_NO_UNIQUE_ADDRESS
#endif  // P_NO_UNIQUE_ADDRESS

/// @endsubmodule


// ========================================
// === from internal\arg_count_impl.h ===
// ========================================

// Copyright 2020 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.


// Expands to the passed arguments.
#define _P_EXPAND(...) __VA_ARGS__

// If-like macro for internal use.
#define _P_IF(boolean, true_expr, false_expr) \
  _P_PASTE2(_P_IF_, boolean)(true_expr, false_expr)

#define _P_IF_0(true_expr, false_expr) false_expr
#define _P_IF_1(true_expr, false_expr) true_expr

// Token pasting macro that doesn't rely on concat.h
#define _P_PASTE2(a1, a2) _P_PASTE2_EXPANDED(a1, a2)
#define _P_PASTE2_EXPANDED(a1, a2) _P_PASTE2_IMPL(a1, a2)
#define _P_PASTE2_IMPL(a1, a2) a1##a2

/*
for i in range(2, 33):
  args = ', '.join(f'a{arg}' for arg in range(1, i))
  print(f'#define _P_LAST_ARG_{i}({args}, a{i}) a{i}')
*/
// clang-format off
#define _P_LAST_ARG_0()
#define _P_LAST_ARG_1(a1) a1
#define _P_LAST_ARG_2(a1, a2) a2
#define _P_LAST_ARG_3(a1, a2, a3) a3
#define _P_LAST_ARG_4(a1, a2, a3, a4) a4
#define _P_LAST_ARG_5(a1, a2, a3, a4, a5) a5
#define _P_LAST_ARG_6(a1, a2, a3, a4, a5, a6) a6
#define _P_LAST_ARG_7(a1, a2, a3, a4, a5, a6, a7) a7
#define _P_LAST_ARG_8(a1, a2, a3, a4, a5, a6, a7, a8) a8
#define _P_LAST_ARG_9(a1, a2, a3, a4, a5, a6, a7, a8, a9) a9
#define _P_LAST_ARG_10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) a10
#define _P_LAST_ARG_11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) a11
#define _P_LAST_ARG_12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) a12
#define _P_LAST_ARG_13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) a13
#define _P_LAST_ARG_14(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) a14
#define _P_LAST_ARG_15(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) a15
#define _P_LAST_ARG_16(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) a16
#define _P_LAST_ARG_17(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) a17
#define _P_LAST_ARG_18(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18) a18
#define _P_LAST_ARG_19(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19) a19
#define _P_LAST_ARG_20(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20) a20
#define _P_LAST_ARG_21(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21) a21
#define _P_LAST_ARG_22(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22) a22
#define _P_LAST_ARG_23(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23) a23
#define _P_LAST_ARG_24(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24) a24
#define _P_LAST_ARG_25(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25) a25
#define _P_LAST_ARG_26(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26) a26
#define _P_LAST_ARG_27(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27) a27
#define _P_LAST_ARG_28(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28) a28
#define _P_LAST_ARG_29(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29) a29
#define _P_LAST_ARG_30(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30) a30
#define _P_LAST_ARG_31(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31) a31
#define _P_LAST_ARG_32(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32) a32

/*
for i in range(2, 33):
  args = ', '.join(f'a{arg}' for arg in range(1, i))
  print(f'#define _P_DROP_LAST_ARG_{i}({args}, a{i}) {args}')
*/
#define _P_DROP_LAST_ARG_0()
#define _P_DROP_LAST_ARG_1(a1)
#define _P_DROP_LAST_ARG_2(a1, a2) a1
#define _P_DROP_LAST_ARG_3(a1, a2, a3) a1, a2
#define _P_DROP_LAST_ARG_4(a1, a2, a3, a4) a1, a2, a3
#define _P_DROP_LAST_ARG_5(a1, a2, a3, a4, a5) a1, a2, a3, a4
#define _P_DROP_LAST_ARG_6(a1, a2, a3, a4, a5, a6) a1, a2, a3, a4, a5
#define _P_DROP_LAST_ARG_7(a1, a2, a3, a4, a5, a6, a7) a1, a2, a3, a4, a5, a6
#define _P_DROP_LAST_ARG_8(a1, a2, a3, a4, a5, a6, a7, a8) a1, a2, a3, a4, a5, a6, a7
#define _P_DROP_LAST_ARG_9(a1, a2, a3, a4, a5, a6, a7, a8, a9) a1, a2, a3, a4, a5, a6, a7, a8
#define _P_DROP_LAST_ARG_10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) a1, a2, a3, a4, a5, a6, a7, a8, a9
#define _P_DROP_LAST_ARG_11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10
#define _P_DROP_LAST_ARG_12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11
#define _P_DROP_LAST_ARG_13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12
#define _P_DROP_LAST_ARG_14(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13
#define _P_DROP_LAST_ARG_15(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14
#define _P_DROP_LAST_ARG_16(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15
#define _P_DROP_LAST_ARG_17(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16
#define _P_DROP_LAST_ARG_18(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17
#define _P_DROP_LAST_ARG_19(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18
#define _P_DROP_LAST_ARG_20(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19
#define _P_DROP_LAST_ARG_21(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20
#define _P_DROP_LAST_ARG_22(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21
#define _P_DROP_LAST_ARG_23(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22
#define _P_DROP_LAST_ARG_24(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23
#define _P_DROP_LAST_ARG_25(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24
#define _P_DROP_LAST_ARG_26(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25
#define _P_DROP_LAST_ARG_27(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26
#define _P_DROP_LAST_ARG_28(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27
#define _P_DROP_LAST_ARG_29(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28
#define _P_DROP_LAST_ARG_30(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29
#define _P_DROP_LAST_ARG_31(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30
#define _P_DROP_LAST_ARG_32(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32) a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31
// clang-format on


// ========================================
// === from arguments.h ===
// ========================================

// Copyright 2020 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.

// Macros for working with arguments to function-like macros.






// Expands to a comma followed by __VA_ARGS__, if __VA_ARGS__ is non-empty.
// Otherwise, expands to nothing. If the final argument is empty, it is omitted.
// This is useful when passing __VA_ARGS__ to a variadic function or template
// parameter list, since it removes the extra comma when no arguments are
// provided. P_COMMA_ARGS must NOT be used when invoking a macro from another
// macro.
//
// This is a more flexible, standard-compliant version of ##__VA_ARGS__. Unlike
// ##__VA_ARGS__, this can be used to eliminate an unwanted comma when
// __VA_ARGS__ expands to an empty argument because an outer macro was called
// with __VA_ARGS__ instead of ##__VA_ARGS__. Also, since P_COMMA_ARGS drops
// the last argument if it is empty, both MY_MACRO(1, 2) and MY_MACRO(1, 2, )
// can work correctly.
//
// P_COMMA_ARGS must NOT be used to conditionally include a comma when invoking
// a macro from another macro. P_COMMA_ARGS only functions correctly when the
// macro expands to C or C++ code! Using it with intermediate macros can result
// in out-of-order parameters. When invoking one macro from another, simply pass
// __VA_ARGS__. Only the final macro that expands to C/C++ code should use
// P_COMMA_ARGS.
//
// For example, the following does NOT work:
/*
     #define MY_MACRO(fmt, ...) \
         NESTED_MACRO(fmt P_COMMA_ARGS(__VA_ARGS__))  // BAD! Do not do this!
*/
// Instead, only use P_COMMA_ARGS when the macro expands to C/C++ code:
/*
     #define MY_MACRO(fmt, ...) \
         NESTED_MACRO(fmt, __VA_ARGS__)  // Pass __VA_ARGS__ to nested macros

     #define NESTED_MACRO(fmt, ...) \
         printf(fmt P_COMMA_ARGS(__VA_ARGS__))  // P_COMMA_ARGS is OK here
*/
#define P_COMMA_ARGS(...)                                       \
  _P_IF(P_EMPTY_ARGS(__VA_ARGS__), _P_EXPAND, _P_COMMA_ARGS) \
  (P_DROP_LAST_ARG_IF_EMPTY(__VA_ARGS__))

#define _P_COMMA_ARGS(...) , __VA_ARGS__

// Allows calling a different function-like macros based on the number of
// arguments. For example:
//
//   #define ARG_PRINT(...)  P_DELEGATE_BY_ARG_COUNT(_ARG_PRINT, __VA_ARGS__)
//   #define _ARG_PRINT1(a)        LOG_INFO("1 arg: %s", a)
//   #define _ARG_PRINT2(a, b)     LOG_INFO("2 args: %s, %s", a, b)
//   #define _ARG_PRINT3(a, b, c)  LOG_INFO("3 args: %s, %s, %s", a, b, c)
//
// This can the be called from C/C++ code:
//
//    ARG_PRINT("a");            // Outputs: 1 arg: a
//    ARG_PRINT("a", "b");       // Outputs: 2 args: a, b
//    ARG_PRINT("a", "b", "c");  // Outputs: 3 args: a, b, c
//
#define P_DELEGATE_BY_ARG_COUNT(function, ...)                 \
  _P_DELEGATE_BY_ARG_COUNT(                                    \
      _P_PASTE2(function, P_FUNCTION_ARG_COUNT(__VA_ARGS__)), \
      P_DROP_LAST_ARG_IF_EMPTY(__VA_ARGS__))

#define _P_DELEGATE_BY_ARG_COUNT(function, ...) function(__VA_ARGS__)

// P_MACRO_ARG_COUNT counts the number of arguments it was called with. It
// evalulates to an integer literal in the range 0 to 64. Counting more than 64
// arguments is not currently supported.
//
// P_MACRO_ARG_COUNT is most commonly used to count __VA_ARGS__ in a variadic
// macro. For example, the following code counts the number of arguments passed
// to a logging macro:
//
/*   #define LOG_INFO(format, ...) {                                   \
         static const int kArgCount = P_MACRO_ARG_COUNT(__VA_ARGS__); \
         SendLog(kArgCount, format, ##__VA_ARGS__);                    \
       }
*/
// The macro argument lists were generated with a Python script:
/*
COUNT = 256

for i in range(COUNT, 0, -1):
    if i % 8 == 0:
        print('\ \n', end='')
    print(f"{i:3}, ", end='')

for i in range(COUNT, 0, -1):
    if i % 8 == 0:
        print('\ \n', end='')
    print(f"a{i:03}, ", end='')
*/
// clang-format off
#define P_MACRO_ARG_COUNT(...)               \
  _P_MACRO_ARG_COUNT_IMPL(__VA_ARGS__,       \
      256, 255, 254, 253, 252, 251, 250, 249, \
      248, 247, 246, 245, 244, 243, 242, 241, \
      240, 239, 238, 237, 236, 235, 234, 233, \
      232, 231, 230, 229, 228, 227, 226, 225, \
      224, 223, 222, 221, 220, 219, 218, 217, \
      216, 215, 214, 213, 212, 211, 210, 209, \
      208, 207, 206, 205, 204, 203, 202, 201, \
      200, 199, 198, 197, 196, 195, 194, 193, \
      192, 191, 190, 189, 188, 187, 186, 185, \
      184, 183, 182, 181, 180, 179, 178, 177, \
      176, 175, 174, 173, 172, 171, 170, 169, \
      168, 167, 166, 165, 164, 163, 162, 161, \
      160, 159, 158, 157, 156, 155, 154, 153, \
      152, 151, 150, 149, 148, 147, 146, 145, \
      144, 143, 142, 141, 140, 139, 138, 137, \
      136, 135, 134, 133, 132, 131, 130, 129, \
      128, 127, 126, 125, 124, 123, 122, 121, \
      120, 119, 118, 117, 116, 115, 114, 113, \
      112, 111, 110, 109, 108, 107, 106, 105, \
      104, 103, 102, 101, 100,  99,  98,  97, \
       96,  95,  94,  93,  92,  91,  90,  89, \
       88,  87,  86,  85,  84,  83,  82,  81, \
       80,  79,  78,  77,  76,  75,  74,  73, \
       72,  71,  70,  69,  68,  67,  66,  65, \
       64,  63,  62,  61,  60,  59,  58,  57, \
       56,  55,  54,  53,  52,  51,  50,  49, \
       48,  47,  46,  45,  44,  43,  42,  41, \
       40,  39,  38,  37,  36,  35,  34,  33, \
       32,  31,  30,  29,  28,  27,  26,  25, \
       24,  23,  22,  21,  20,  19,  18,  17, \
       16,  15,  14,  13,  12,  11,  10,   9, \
        8,   7,   6,   5,   4,   3,   2, P_HAS_ARGS(__VA_ARGS__))


#define _P_MACRO_ARG_COUNT_IMPL(                   \
    a256, a255, a254, a253, a252, a251, a250, a249, \
    a248, a247, a246, a245, a244, a243, a242, a241, \
    a240, a239, a238, a237, a236, a235, a234, a233, \
    a232, a231, a230, a229, a228, a227, a226, a225, \
    a224, a223, a222, a221, a220, a219, a218, a217, \
    a216, a215, a214, a213, a212, a211, a210, a209, \
    a208, a207, a206, a205, a204, a203, a202, a201, \
    a200, a199, a198, a197, a196, a195, a194, a193, \
    a192, a191, a190, a189, a188, a187, a186, a185, \
    a184, a183, a182, a181, a180, a179, a178, a177, \
    a176, a175, a174, a173, a172, a171, a170, a169, \
    a168, a167, a166, a165, a164, a163, a162, a161, \
    a160, a159, a158, a157, a156, a155, a154, a153, \
    a152, a151, a150, a149, a148, a147, a146, a145, \
    a144, a143, a142, a141, a140, a139, a138, a137, \
    a136, a135, a134, a133, a132, a131, a130, a129, \
    a128, a127, a126, a125, a124, a123, a122, a121, \
    a120, a119, a118, a117, a116, a115, a114, a113, \
    a112, a111, a110, a109, a108, a107, a106, a105, \
    a104, a103, a102, a101, a100, a099, a098, a097, \
    a096, a095, a094, a093, a092, a091, a090, a089, \
    a088, a087, a086, a085, a084, a083, a082, a081, \
    a080, a079, a078, a077, a076, a075, a074, a073, \
    a072, a071, a070, a069, a068, a067, a066, a065, \
    a064, a063, a062, a061, a060, a059, a058, a057, \
    a056, a055, a054, a053, a052, a051, a050, a049, \
    a048, a047, a046, a045, a044, a043, a042, a041, \
    a040, a039, a038, a037, a036, a035, a034, a033, \
    a032, a031, a030, a029, a028, a027, a026, a025, \
    a024, a023, a022, a021, a020, a019, a018, a017, \
    a016, a015, a014, a013, a012, a011, a010, a009, \
    a008, a007, a006, a005, a004, a003, a002, a001, \
    count, ...)                                     \
  count

// clang-format on

// Argument count for using with a C/C++ function or template parameter list.
// The difference from P_MACRO_ARG_COUNT is that the last argument is not
// counted if it is empty. This makes it easier to drop the final comma when
// expanding to C/C++ code.
#define P_FUNCTION_ARG_COUNT(...) \
  _P_FUNCTION_ARG_COUNT(P_LAST_ARG(__VA_ARGS__), __VA_ARGS__)

#define _P_FUNCTION_ARG_COUNT(last_arg, ...) \
  _P_PASTE2(_P_FUNCTION_ARG_COUNT_, P_EMPTY_ARGS(last_arg))(__VA_ARGS__)

#define _P_FUNCTION_ARG_COUNT_0 P_MACRO_ARG_COUNT
#define _P_FUNCTION_ARG_COUNT_1(...) \
  P_MACRO_ARG_COUNT(P_DROP_LAST_ARG(__VA_ARGS__))

// Evaluates to the last argument in the provided arguments.
#define P_LAST_ARG(...) \
  _P_PASTE2(_P_LAST_ARG_, P_MACRO_ARG_COUNT(__VA_ARGS__))(__VA_ARGS__)

// Evaluates to the provided arguments, excluding the final argument.
#define P_DROP_LAST_ARG(...) \
  _P_PASTE2(_P_DROP_LAST_ARG_, P_MACRO_ARG_COUNT(__VA_ARGS__))(__VA_ARGS__)

// Evaluates to the arguments, excluding the final argument if it is empty.
#define P_DROP_LAST_ARG_IF_EMPTY(...)                                       \
  _P_IF(                                                                    \
      P_EMPTY_ARGS(P_LAST_ARG(__VA_ARGS__)), P_DROP_LAST_ARG, _P_EXPAND) \
  (__VA_ARGS__)

// Expands to 1 if one or more arguments are provided, 0 otherwise.
#define P_HAS_ARGS(...) P_NOT(P_EMPTY_ARGS(__VA_ARGS__))

#if P_VA_OPT_SUPPORTED()

// Expands to 0 if one or more arguments are provided, 1 otherwise.
#define P_EMPTY_ARGS(...) _P_EMPTY_ARGS_##__VA_OPT__(0)
#define _P_EMPTY_ARGS_ 1
#define _P_EMPTY_ARGS_0 0

#else

// If __VA_OPT__ is not available, use a complicated fallback mechanism. This
// approach is from Jens Gustedt's blog:
//   https://gustedt.wordpress.com/2010/06/08/detect-empty-macro-arguments/
//
// Normally, with a standard-compliant C preprocessor, it's impossible to tell
// whether a variadic macro was called with no arguments or with one argument.
// A macro invoked with no arguments is actually passed one empty argument.
//
// This macro works by checking for the presence of a comma in four situations.
// These situations give the following information about __VA_ARGS__:
//
//   1. It is two or more variadic arguments.
//   2. It expands to one argument surrounded by parentheses.
//   3. It is a function-like macro that produces a comma when invoked.
//   4. It does not interfere with calling a macro when placed between it and
//      parentheses.
//
// If a comma is not present in 1, 2, 3, but is present in 4, then __VA_ARGS__
// is empty. For this case (0001), and only this case, a corresponding macro
// that expands to a comma is defined. The presence of this comma determines
// whether any arguments were passed in.
#define P_EMPTY_ARGS(...)                                             \
  _P_HAS_NO_ARGS(_P_HAS_COMMA(__VA_ARGS__),                          \
                  _P_HAS_COMMA(_P_MAKE_COMMA_IF_CALLED __VA_ARGS__), \
                  _P_HAS_COMMA(__VA_ARGS__()),                        \
                  _P_HAS_COMMA(_P_MAKE_COMMA_IF_CALLED __VA_ARGS__()))

// clang-format off
#define _P_HAS_COMMA(...)                                           \
  _P_MACRO_ARG_COUNT_IMPL(__VA_ARGS__,                              \
  /*  16 */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*  32 */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*  48 */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*  64 */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*     */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*     */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*     */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /* 128 */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*     */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*     */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*     */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /* 196 */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*     */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*     */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /*     */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, \
  /* 256 */          1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0)
// clang-format on

#define _P_HAS_NO_ARGS(a1, a2, a3, a4) \
  _P_HAS_COMMA(_P_PASTE_RESULTS(a1, a2, a3, a4))
#define _P_PASTE_RESULTS(a1, a2, a3, a4) _P_HAS_COMMA_CASE_##a1##a2##a3##a4
#define _P_HAS_COMMA_CASE_0001 ,
#define _P_MAKE_COMMA_IF_CALLED(...) ,

#endif  // P_VA_OPT_SUPPORTED()


// ========================================
// === from concat.h ===
// ========================================

// Copyright 2019 The Pigweed Authors
//
// Licensed under the Apache License, Version 2.0 (the "License"); you may not
// use this file except in compliance with the License. You may obtain a copy of
// the License at
//
//     https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
// License for the specific language governing permissions and limitations under
// the License.





// Expands macros and concatenates the results using preprocessor ##
// concatentation. Supports up to 32 arguments.
#define P_CONCAT(...) \
  _P_CONCAT_IMPL1(P_MACRO_ARG_COUNT(__VA_ARGS__), __VA_ARGS__)

// Expand the macro to allow P_MACRO_ARG_COUNT and any caller-provided macros
// to be evaluated before concatenating the tokens.
#define _P_CONCAT_IMPL1(count, ...) _P_CONCAT_IMPL2(count, __VA_ARGS__)
#define _P_CONCAT_IMPL2(count, ...) _P_CONCAT_##count(__VA_ARGS__)

// clang-format off
/* This macro implementation was generated with the following Python 3 code:
for i in range(32 + 1):
  args = [f'a{x}' for x in range(1, i + 1)]
  print(f'#define _P_CONCAT_{i}({", ".join(args)}) {"##".join(args)}  // NOLINT')
*/

#define _P_CONCAT_0()   // NOLINT
#define _P_CONCAT_1(a1) a1  // NOLINT
#define _P_CONCAT_2(a1, a2) a1##a2  // NOLINT
#define _P_CONCAT_3(a1, a2, a3) a1##a2##a3  // NOLINT
#define _P_CONCAT_4(a1, a2, a3, a4) a1##a2##a3##a4  // NOLINT
#define _P_CONCAT_5(a1, a2, a3, a4, a5) a1##a2##a3##a4##a5  // NOLINT
#define _P_CONCAT_6(a1, a2, a3, a4, a5, a6) a1##a2##a3##a4##a5##a6  // NOLINT
#define _P_CONCAT_7(a1, a2, a3, a4, a5, a6, a7) a1##a2##a3##a4##a5##a6##a7  // NOLINT
#define _P_CONCAT_8(a1, a2, a3, a4, a5, a6, a7, a8) a1##a2##a3##a4##a5##a6##a7##a8  // NOLINT
#define _P_CONCAT_9(a1, a2, a3, a4, a5, a6, a7, a8, a9) a1##a2##a3##a4##a5##a6##a7##a8##a9  // NOLINT
#define _P_CONCAT_10(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10  // NOLINT
#define _P_CONCAT_11(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11  // NOLINT
#define _P_CONCAT_12(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12  // NOLINT
#define _P_CONCAT_13(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13  // NOLINT
#define _P_CONCAT_14(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14  // NOLINT
#define _P_CONCAT_15(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15  // NOLINT
#define _P_CONCAT_16(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16  // NOLINT
#define _P_CONCAT_17(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17  // NOLINT
#define _P_CONCAT_18(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18  // NOLINT
#define _P_CONCAT_19(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19  // NOLINT
#define _P_CONCAT_20(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20  // NOLINT
#define _P_CONCAT_21(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21  // NOLINT
#define _P_CONCAT_22(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22  // NOLINT
#define _P_CONCAT_23(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23  // NOLINT
#define _P_CONCAT_24(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23##a24  // NOLINT
#define _P_CONCAT_25(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23##a24##a25  // NOLINT
#define _P_CONCAT_26(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23##a24##a25##a26  // NOLINT
#define _P_CONCAT_27(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23##a24##a25##a26##a27  // NOLINT
#define _P_CONCAT_28(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23##a24##a25##a26##a27##a28  // NOLINT
#define _P_CONCAT_29(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23##a24##a25##a26##a27##a28##a29  // NOLINT
#define _P_CONCAT_30(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23##a24##a25##a26##a27##a28##a29##a30  // NOLINT
#define _P_CONCAT_31(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23##a24##a25##a26##a27##a28##a29##a30##a31  // NOLINT
#define _P_CONCAT_32(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20, a21, a22, a23, a24, a25, a26, a27, a28, a29, a30, a31, a32) a1##a2##a3##a4##a5##a6##a7##a8##a9##a10##a11##a12##a13##a14##a15##a16##a17##a18##a19##a20##a21##a22##a23##a24##a25##a26##a27##a28##a29##a30##a31##a32  // NOLINT


