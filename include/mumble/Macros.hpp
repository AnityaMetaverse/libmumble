// Copyright 2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_MACROS_HPP
#define MUMBLE_MACROS_HPP

#define MUMBLE_ENUM_OPERATORS(T)                                                                     \
	static inline T operator~(const T lhs) {                                                         \
		return static_cast< T >(~static_cast< std::underlying_type< T >::type >(lhs));               \
	}                                                                                                \
	static inline T operator|(const T lhs, const T rhs) {                                            \
		return static_cast< T >(static_cast< std::underlying_type< T >::type >(lhs)                  \
								| static_cast< std::underlying_type< T >::type >(rhs));              \
	}                                                                                                \
	static inline T operator&(const T lhs, const T rhs) {                                            \
		return static_cast< T >(static_cast< std::underlying_type< T >::type >(lhs)                  \
								& static_cast< std::underlying_type< T >::type >(rhs));              \
	}                                                                                                \
	static inline T operator^(const T lhs, const T rhs) {                                            \
		return static_cast< T >(static_cast< std::underlying_type< T >::type >(lhs)                  \
								^ static_cast< std::underlying_type< T >::type >(rhs));              \
	}                                                                                                \
	static inline T &operator|=(T &lhs, const T rhs) {                                               \
		return reinterpret_cast< T & >(reinterpret_cast< std::underlying_type< T >::type & >(lhs) |= \
									   static_cast< std::underlying_type< T >::type >(rhs));         \
	}                                                                                                \
	static inline T &operator&=(T &lhs, const T rhs) {                                               \
		return reinterpret_cast< T & >(reinterpret_cast< std::underlying_type< T >::type & >(lhs) &= \
									   static_cast< std::underlying_type< T >::type >(rhs));         \
	}                                                                                                \
	static inline T &operator^=(T &lhs, const T rhs) {                                               \
		return reinterpret_cast< T & >(reinterpret_cast< std::underlying_type< T >::type & >(lhs) ^= \
									   static_cast< std::underlying_type< T >::type >(rhs));         \
	}

#ifdef COMPILER_MSVC
#	define MUMBLE_PACK(decl) __pragma(pack(push, 1)) decl __pragma(pack(pop))
#else
#	define MUMBLE_PACK(decl) decl __attribute__((__packed__))
#endif

#endif
