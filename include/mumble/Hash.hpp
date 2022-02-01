// Copyright 2022 The Mumble Developers. All rights reserved.
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file at the root of the
// Mumble source tree or at <https://www.mumble.info/LICENSE>.

#ifndef MUMBLE_HASH_HPP
#define MUMBLE_HASH_HPP

#include "Types.hpp"

namespace mumble {
class EXPORT Hash {
public:
	class P;

	Hash(Hash &&crypt);
	Hash();
	virtual ~Hash();

	virtual explicit operator bool() const;

	virtual Hash &operator=(Hash &&crypt);

	virtual size_t operator()(const BufRef out, const BufRefConst in);

	virtual void *handle() const;

	virtual std::string_view type() const;
	virtual bool setType(const std::string_view name);

	virtual uint32_t blockSize() const;

	virtual bool reset();

private:
	Hash(const Hash &)    = delete;
	virtual Hash &operator=(const Hash &) = delete;

	std::unique_ptr< P > m_p;
};
} // namespace mumble

#endif