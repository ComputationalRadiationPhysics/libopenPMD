/* Copyright 2017-2021 Fabian Koller
 *
 * This file is part of openPMD-api.
 *
 * openPMD-api is free software: you can redistribute it and/or modify
 * it under the terms of of either the GNU General Public License or
 * the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * openPMD-api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License and the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and the GNU Lesser General Public License along with openPMD-api.
 * If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <ostream>


namespace openPMD
{
/** Encoding scheme of an Iterations Series'.
 *
 * @see https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#iterations-and-time-series
 */
enum class IterationEncoding
{
    fileBased, groupBased, variableBased
};

std::ostream&
operator<<(std::ostream&, openPMD::IterationEncoding const&);

} // openPMD
