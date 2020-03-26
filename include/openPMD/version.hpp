/* Copyright 2017-2020 Fabian Koller, Axel Huebl
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

#include "openPMD/auxiliary/Export.hpp"

#include <map>
#include <string>

/** version of the openPMD-api library (compile-time)
 * @{
 */
#define OPENPMDAPI_VERSION_MAJOR 0
#define OPENPMDAPI_VERSION_MINOR 12
#define OPENPMDAPI_VERSION_PATCH 0
#define OPENPMDAPI_VERSION_LABEL "dev"
/** @} */

/** maximum supported version of the openPMD standard (read & write, compile-time)
 * @{
 */
#define OPENPMD_STANDARD_MAJOR 1
#define OPENPMD_STANDARD_MINOR 1
#define OPENPMD_STANDARD_PATCH 0
/** @} */

/** minimum supported version of the openPMD standard (read, compile-time)
 * @{
 */
#define OPENPMD_STANDARD_MIN_MAJOR 1
#define OPENPMD_STANDARD_MIN_MINOR 0
#define OPENPMD_STANDARD_MIN_PATCH 0
/** @} */

// runtime functions (TODO: noinline qualifiers? what about LTO/IPO?)
namespace openPMD
{
    /** Return the version of the openPMD-api library (run-time)
     *
     * @return std::string API version (dot separated)
     */
    OPENPMDAPI_EXPORT
    std::string
    getVersion( );

    /** Return the maximum supported version of the openPMD standard (read & write, run-time)
     *
     * @return std::string openPMD standard version (dot separated)
     */
    OPENPMDAPI_EXPORT
    std::string
    getStandard( );

    /** Return the minimum supported version of the openPMD standard (read, run-time)
     *
     * @return std::string minimum openPMD standard version (dot separated)
     */
    OPENPMDAPI_EXPORT
    std::string
    getStandardMinimum( );

    /** Return the feature variants of the openPMD-api library (run-time)
     *
     * @return std::map< std::string, bool > with variants such as backends
     */
    OPENPMDAPI_EXPORT
    std::map< std::string, bool >
    getVariants( );
} // namespace openPMD
