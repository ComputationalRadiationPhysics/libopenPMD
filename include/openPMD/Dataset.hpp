/* Copyright 2017-2020 Fabian Koller
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

#include "openPMD/Datatype.hpp"

#include <memory>
#include <type_traits>
#include <vector>
#include <string>


namespace openPMD
{
using Extent = std::vector< std::uint64_t >;
using Offset = std::vector< std::uint64_t >;

class Dataset
{
    friend class RecordComponent;

public:
    Dataset(Datatype, Extent);

    Dataset& extend(Extent newExtent);
    Dataset& setChunkSize(Extent const&);
    Dataset& setCompression(std::string const&, uint8_t const);
    Dataset& setCustomTransform(std::string const&);

    Extent extent;
    Datatype dtype;
    uint8_t rank;
    Extent chunkSize;
    std::string compression;
    std::string transform;
};

/**
 * A chunk consists of its offset, its extent
 * and the rank from which it was written.
 * If not specified explicitly, the rank will be assumed to be 0.
 */
struct Chunk
{
    Offset offset;
    Extent extent;
    unsigned int mpi_rank = 0;

    /*
     * If rank is smaller than zero, will be converted to zero.
     */
    explicit Chunk() = default;
    Chunk( Offset, Extent, int mpi_rank );
    Chunk( Offset, Extent );

    bool
    operator==( Chunk const & other ) const;
};
using ChunkTable = std::vector< Chunk >;
} // namespace openPMD
