/*
 *  Copyright (C) 2005-2020 Team Kodi
 *
 *  This file is based on source code from Kodi - https://kodi.tv
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSES/README.md for more information.
 */

#ifndef HDR_H_
#define HDR_H_

#include <string>
#include <utility>
#include <vector>

namespace hdr {

enum class Status { Unsupported = 0, Off = 1, On = 2 };

Status GetWindowsHDRStatus();

} // namespace hdr

#endif // HDR_H_
