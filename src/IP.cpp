// Copyright 2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#include "mumble/IP.hpp"

#include <algorithm>
#include <cstring>

#ifdef OS_WINDOWS
#	include <WS2tcpip.h>
#else
#	include <arpa/inet.h>
#endif

static constexpr uint8_t v6StrSize = 46;
static constexpr uint8_t v4StrSize = 16;

using namespace mumble;

using P        = IP::P;
using Ref      = IP::Ref;
using RefConst = IP::RefConst;

EXPORT IP::IP() : m_bytes({}) {
}

EXPORT IP::IP(const IP &ip) : m_bytes(ip.m_bytes) {
}

EXPORT IP::IP(const RefConst ref) {
	switch (ref.size()) {
		case v6Size:
			std::copy(ref.begin(), ref.end(), m_bytes.begin());
			break;
		case v4Size:
			std::fill_n(&m_bytes[0], 10, 0x00);
			std::fill_n(&m_bytes[10], 2, 0xff);
			std::copy(ref.begin(), ref.end(), &m_bytes[12]);
	}
}

EXPORT IP::IP(const std::string_view string) {
	if (inet_pton(AF_INET6, string.data(), m_bytes.data()) == 1) {
		return;
	}

	std::fill_n(&m_bytes[0], 10, 0x00);
	std::fill_n(&m_bytes[10], 2, 0xff);

	std::sscanf(string.data(), "%hhu.%hhu.%hhu.%hhu", &m_bytes[12], &m_bytes[13], &m_bytes[14], &m_bytes[15]);
}

EXPORT IP::IP(const sockaddr_in6 &sockaddr) {
	std::memcpy(m_bytes.data(), &sockaddr.sin6_addr, m_bytes.size());
}

EXPORT IP::~IP() = default;

EXPORT IP &IP::operator=(const IP &ip) {
	m_bytes = ip.m_bytes;
	return *this;
}

EXPORT bool IP::operator==(const IP &ip) const {
	return m_bytes == ip.m_bytes;
}

EXPORT RefConst IP::v6() const {
	return m_bytes;
}

EXPORT RefConst IP::v4() const {
	return { m_bytes.cbegin() + 12, m_bytes.cend() };
}

EXPORT Ref IP::v6() {
	return m_bytes;
}

EXPORT Ref IP::v4() {
	return { m_bytes.begin() + 12, m_bytes.end() };
}

EXPORT bool IP::isV6() const {
	return !isV4();
}

EXPORT bool IP::isV4() const {
	if (!std::all_of(&m_bytes[0], &m_bytes[9], [](const uint8_t byte) { return byte == 0x00; })) {
		return false;
	}

	if (!std::all_of(&m_bytes[10], &m_bytes[11], [](const uint8_t byte) { return byte == 0xff; })) {
		return false;
	}

	return true;
}

EXPORT bool IP::isWildcard() const {
	const auto ref = isV6() ? v6() : v4();
	return std::all_of(ref.begin(), ref.end(), [](const uint8_t byte) { return byte == 0x00; });
}

EXPORT std::string IP::text() const {
	std::string ret;

	if (isV6()) {
		ret.resize(v6StrSize);

		if (!inet_ntop(AF_INET6, m_bytes.data(), ret.data(), v6StrSize)) {
			return {};
		}
	} else {
		ret.resize(v4StrSize);

		std::snprintf(ret.data(), v4StrSize, "%hhu.%hhu.%hhu.%hhu", m_bytes[12], m_bytes[13], m_bytes[14], m_bytes[15]);
	}

	return ret;
}

EXPORT void IP::toSockAddr(sockaddr_in6 &sockaddr) const {
	sockaddr.sin6_family = AF_INET6;
	std::memcpy(&sockaddr.sin6_addr, m_bytes.data(), sizeof(sockaddr.sin6_addr));
}
