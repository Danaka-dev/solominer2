#pragma once

/*
 * CryptH - Simple symmetric encryption/decryption
 *
 * Copyright 2012-2024 M. Ganapati <m.ganapati@proton.me>
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CRYPTH_H
#define CRYPTH_H

//////////////////////////////////////////////////////////////////////////////
#define CRYPTH_SALT     "salt"
#define CRYPTH_PEPPER   "pepper"

//////////////////////////////////////////////////////////////////////////////
#include <string>

/// @depends sha258 & base58 algorithms

//////////////////////////////////////////////////////////////////////////////
bool CryptH( const char *password ,const char *text ,std::string &s );
bool DecryptH( const char *password ,const char *text ,std::string &s );

//////////////////////////////////////////////////////////////////////////////
#endif //CRYPTH_H