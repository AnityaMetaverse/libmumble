// Copyright 2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_SRC_TLS_HPP
#define MUMBLE_SRC_TLS_HPP

#include "TCP.hpp"

#include "mumble/Cert.hpp"

#include <openssl/ossl_typ.h>

namespace mumble {
class Key;

class SocketTLS : public SocketTCP {
public:
	enum Code : int8_t { Memory = -3, Failure, Unknown, Success, Retry, Shutdown, WaitIn, WaitOut };

	SocketTLS(SocketTLS &&socket);
	SocketTLS(SocketTCP &&socket, const bool server);
	~SocketTLS();

	explicit operator bool() const;

	bool setCert(const Cert::Chain &cert, const Key &key);

	Cert::Chain peerCert() const;

	uint32_t pending() const;

	Code accept();
	Code connect();
	Code disconnect();

	Code read(BufRef &buf);
	Code write(BufRefConst &buf);

protected:
	bool m_server;

private:
	static int verifyCallback(int, X509_STORE_CTX *);
	static constexpr Code interpretLibCode(const int code, const bool processed = true, const bool remaining = false);

	SSL *m_ssl;
	SSL_CTX *m_sslCtx;
};
} // namespace mumble

#endif