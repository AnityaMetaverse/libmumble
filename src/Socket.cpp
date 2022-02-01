// Copyright 2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "Socket.hpp"

#include "IP.hpp"

#include "mumble/Endian.hpp"
#include "mumble/Mumble.hpp"

#ifdef OS_WINDOWS
#	include <afunix.h>
#	include <WinSock2.h>

#	define poll WSAPoll
#else
#	include <errno.h>
#	include <fcntl.h>
#	include <poll.h>
#	include <unistd.h>

#	include <arpa/inet.h>

#	include <sys/socket.h>
#	include <sys/types.h>
#endif

using namespace mumble;

using Handle = Socket::Handle;
using State  = Socket::State;

Socket::Socket(Socket &&socket) : m_handle(std::move(socket.m_handle)), m_manualEvent(std::move(socket.m_manualEvent)) {
}

Socket::Socket(const int fd) : m_handle(fd), m_manualEvent(Handle::pair()) {
}

Socket::Socket(const Type type) : m_handle(type), m_manualEvent(Handle::pair()) {
}

Socket::~Socket() = default;

Socket::operator bool() const {
	return static_cast< bool >(m_handle);
}

int Socket::getEndpoint(Endpoint &endpoint) {
	sockaddr_in6 addr;
#ifdef OS_WINDOWS
	int size = sizeof(addr);
#else
	socklen_t size  = sizeof(addr);
#endif
	if (getsockname(m_handle.fd(), reinterpret_cast< sockaddr * >(&addr), &size) != 0) {
		return osError();
	}

	endpoint.ip   = IP(addr);
	endpoint.port = Endian::toHost(addr.sin6_port);

	return 0;
}

int Socket::setEndpoint(const Endpoint &endpoint, const bool ipv6Only) {
	sockaddr_in6 addr = {};
	endpoint.ip.toSockAddr(addr);
	addr.sin6_port = Endian::toNetwork(endpoint.port);

	int value = ipv6Only;
	if (setsockopt(m_handle.fd(), IPPROTO_IPV6, IPV6_V6ONLY, &value, sizeof(value)) != 0) {
		return osError();
	}

	value = 1;
#ifdef SO_EXCLUSIVEADDRUSE
	if (setsockopt(m_handle->fd(), SOL_SOCKET, SO_EXCLUSIVEADDRUSE, &value, sizeof(value)) != 0) {
		return osError();
	}
#endif
	if (bind(m_handle.fd(), reinterpret_cast< sockaddr * >(&addr), sizeof(addr)) != 0) {
		return osError();
	}

	return 0;
}

int Socket::setBlocking(const bool enable) {
#ifdef OS_WINDOWS
	const u_long value = !enable;
	if (ioctlsocket(m_handle, FIONBIO, &value) != 0) {
		return osError();
	}
#else
	const int flags = fcntl(m_handle.fd(), F_GETFL, 0);
	if (flags == -1) {
		return osError();
	}

	const int new_flags = enable ? flags & ~O_NONBLOCK : flags | O_NONBLOCK;
	if (flags == new_flags) {
		return 0;
	}

	if (fcntl(m_handle.fd(), F_SETFL, new_flags) != 0) {
		return osError();
	}
#endif
	return 0;
}

bool Socket::trigger() {
	constexpr uint8_t byte = 0;
	if (send(m_manualEvent[1].fd(), &byte, sizeof(byte), 0) < sizeof(byte)) {
		return false;
	}

	return true;
}

State Socket::wait(const bool in, const bool out, const uint32_t timeout) {
	pollfd pollfds[2] = {};

	pollfds[0].fd     = m_manualEvent[0].fd();
	pollfds[0].events = POLLIN;

	if (in || out) {
		pollfds[1].fd = m_handle.fd();

		if (in) {
			pollfds[1].events |= POLLIN;
		}

		if (out) {
			pollfds[1].events |= POLLOUT;
		}
	}

	const int ret = poll(pollfds, in || out ? 2 : 1, timeout == infinite32 ? -1 : timeout);
	if (ret < 1) {
		if (ret == 0) {
			return Timeout;
		}

		return Error;
	}

	State state = Timeout;

	if (pollfds[0].revents & POLLIN) {
		uint8_t byte;
		recv(pollfds[0].fd, &byte, sizeof(byte), 0);

		state |= Triggered;
	}

	if (pollfds[1].revents & POLLHUP) {
		state |= Disconnected;
	}

	if (pollfds[1].revents & POLLERR) {
		state |= Error;
	}

	if (pollfds[1].revents & POLLIN) {
		state |= InReady;
	}

	if (pollfds[1].revents & POLLOUT) {
		state |= OutReady;
	}

	return state;
}

int Socket::osError() {
#ifdef OS_WINDOWS
	return WSAGetLastError();
#else
	return errno;
#endif
}

Handle::Handle(Handle &&handle) : m_fd(std::exchange(handle.m_fd, invalid)) {
}

Handle::Handle(const int32_t fd) : m_fd(fd) {
}

Handle::Handle(const Type type) {
	switch (type) {
		case Type::TCP:
			m_fd = socket(PF_INET6, SOCK_STREAM, IPPROTO_TCP);
			break;
		case Type::UDP:
			m_fd = socket(PF_INET6, SOCK_DGRAM, IPPROTO_UDP);
			break;
		default:
			m_fd = invalid;
	}
}

Handle::~Handle() {
	if (m_fd != invalid) {
#ifdef OS_WINDOWS
		closesocket(m_fd);
#else
		close(m_fd);
#endif
	}
}

Handle::operator bool() const {
	return m_fd != invalid;
}

int32_t Handle::fd() const {
	return m_fd;
}

Handle::Pair Handle::pair() {
#ifdef OS_WINDOWS
	// TODO: Write socketpair() emulation!
	return {};
#else
	int fds[2];
	if (socketpair(PF_LOCAL, SOCK_DGRAM, 0, fds) != 0) {
		return {};
	}

	return { fds[0], fds[1] };
#endif
}