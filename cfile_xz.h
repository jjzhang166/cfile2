/*
 * cfile_xz.h
 * This file is part of The PaulWay Libraries
 *
 * Copyright (C) 2006 Paul Wayper <paulway@mabula.net>
 * Copyright (C) 2012 Peter Miller
 *
 * The PaulWay Libraries are free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * The PaulWay Libraries are distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef CFILE_XZ_H
#define CFILE_XZ_H

/**
  * The cfile_xz function is used to open XZ and LZMA files.
  *
  * @param pathname
  *     The name of the file to open.
  * @param mode
  * 	The mode to use for file operations (read or write).
  * @returns
  *     The new file handle
  */
cfile *xz_open(const char *name, const char *mode);

#endif
