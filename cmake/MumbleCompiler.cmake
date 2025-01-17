# Copyright 2022 The Mumble Developers. All rights reserved.
# Use of this source code is governed by a BSD-style license
# that can be found in the LICENSE file at the root of the
# Mumble source tree or at <https://www.mumble.info/LICENSE>.

function(target_pedantic_warnings TARGET)
	# With clang-cl CMAKE_CXX_COMPILER_ID evaluates to "Clang" and CMAKE_CXX_SIMULATE_ID to "MSVC".
	# There is no generator expression for CMAKE_CXX_SIMULATE_ID as of CMake 3.23.
	if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_SIMULATE_ID STREQUAL "MSVC")
		set(COMPILER_MSVC TRUE)
	endif()

	target_compile_options(${TARGET}
		PRIVATE
			$<IF:$<BOOL:${COMPILER_MSVC}>,/W4 /WX-,-Wall -Wextra -Wpedantic -Werror>
	)
endfunction()
