// Copyright 2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_SRC_UDP_HPP
#define MUMBLE_SRC_UDP_HPP

#include "Socket.hpp"

namespace mumble {
class SocketUDP : public Socket {
public:
	SocketUDP();

	Code read(Endpoint &endpoint, BufRef &buf);
	Code write(const Endpoint &endpoint, const BufRefConst buf);
};
} // namespace mumble

#endif