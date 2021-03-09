/* Copyright 2020-2021 Franz Poeschel
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

#include "openPMD/benchmark/mpi/BlockSlicer.hpp"
#include "openPMD/Dataset.hpp" // Offset, Extent

#include <vector>


namespace openPMD
{
/**
 * Represents the meta info around a chunk in a dataset.
 *
 * A chunk consists of its offset and its extent
 */
struct ChunkInfo
{
    Offset offset; //!< origin of the chunk
    Extent extent; //!< size of the chunk

    /*
     * If rank is smaller than zero, will be converted to zero.
     */
    explicit ChunkInfo() = default;
    ChunkInfo( Offset, Extent );

    bool
    operator==( ChunkInfo const & other ) const;
};

/**
 * Represents the meta info around a chunk that has been written by some
 * data producing application.
 * Produced by BaseRecordComponent::availableChunk.
 *
 * Carries along the usual chunk meta info also the ID for the data source from
 * which the chunk is received.
 * Examples for this include the writing MPI rank in streaming setups or the
 * subfile containing the chunk.
 * If not specified explicitly, the sourceID will be assumed to be 0.
 * This information will vary between different backends and should be used
 * for optimization purposes only.
 */
struct WrittenChunkInfo : ChunkInfo
{
    unsigned int sourceID = 0; //!< ID of the data source containing the chunk

    explicit WrittenChunkInfo() = default;
    /*
     * If rank is smaller than zero, will be converted to zero.
     */
    WrittenChunkInfo( Offset, Extent, int sourceID );
    WrittenChunkInfo( Offset, Extent );

    bool
    operator==( WrittenChunkInfo const & other ) const;
};

using ChunkTable = std::vector< WrittenChunkInfo >;

namespace chunk_assignment
{
    constexpr char const * HOSTFILE_VARNAME = "MPI_WRITTEN_HOSTFILE";

    using RankMeta = std::map< unsigned int, std::string >;

    struct PartialAssignment
    {
        ChunkTable notAssigned;
        ChunkTable assigned;

        explicit PartialAssignment() = default;
        PartialAssignment( ChunkTable notAssigned );
        PartialAssignment( ChunkTable notAssigned, ChunkTable assigned );
    };

    /**
     * @brief Interface for a chunk distribution strategy.
     *
     * Used for implementing algorithms that read a ChunkTable as produced
     * by BaseRecordComponent::availableChunks() and produce as result a
     * ChunkTable that guides data sinks on how to load data into reading
     * processes.
     */
    struct Strategy
    {
        /**
         * @brief Assign chunks to be loaded to reading processes.
         *
         * @param partialAssignment Two chunktables, one of unassigned chunks
         *        and one of chunks that might have already been assigned
         *        previously.
         *        Merge the unassigned chunks into the partially assigned table.
         * @param in Meta information on writing processes, e.g. hostnames.
         * @param out Meta information on reading processes, e.g. hostnames.
         * @return ChunkTable A table that assigns chunks to reading processes.
         */
        virtual ChunkTable
        assign(
            PartialAssignment partialAssignment,
            RankMeta const & in,
            RankMeta const & out ) = 0;

        virtual ~Strategy() = default;
    };

    /**
     * @brief A chunk distribution strategy that guarantees no complete
     *        distribution.
     *
     * Combine with a full Strategy using the FromPartialStrategy struct to
     * obtain a Strategy that works in two phases:
     * 1. Apply the partial strategy.
     * 2. Apply the full strategy to assign unassigned leftovers.
     *
     */
    struct PartialStrategy
    {
        /**
         * @brief Assign chunks to be loaded to reading processes.
         *
         * @param partialAssignment Two chunktables, one of unassigned chunks
         *        and one of chunks that might have already been assigned
         *        previously.
         *        Merge the unassigned chunks into the partially assigned table.
         * @param in Meta information on writing processes, e.g. hostnames.
         * @param out Meta information on reading processes, e.g. hostnames.
         * @return PartialAssignment Two chunktables, one of leftover chunks
         *         that were not assigned and one that assigns chunks to
         *         reading processes.
         */
        virtual PartialAssignment
        assign(
            PartialAssignment partialAssignment,
            RankMeta const & in,
            RankMeta const & out ) = 0;

        virtual ~PartialStrategy() = default;
    };

    ChunkTable
    assignChunks(
        ChunkTable,
        RankMeta const & rankMetaIn,
        RankMeta const & rankMetaOut,
        Strategy & strategy );

    PartialAssignment assignChunks(
        ChunkTable,
        RankMeta const & rankMetaIn,
        RankMeta const & rankMetaOut,
        PartialStrategy & strategy );

    /**
     * @brief Combine a PartialStrategy and a Strategy to obtain a Strategy
     *        working in two phases.
     *
     * 1. Apply the PartialStrategy to obtain a PartialAssignment.
     *    This may be a heuristic that will not work under all circumstances,
     *    e.g. trying to distribute chunks within the same compute node.
     * 2. Apply the Strategy to assign leftovers.
     *    This guarantees correctness in case the heuristics in the first phase
     *    were not applicable e.g. due to a suboptimal setup.
     *
     */
    struct FromPartialStrategy : Strategy
    {
        FromPartialStrategy(
            std::unique_ptr< PartialStrategy > firstPass,
            std::unique_ptr< Strategy > secondPass );

        virtual ChunkTable
        assign( PartialAssignment, RankMeta const & in, RankMeta const & out );

    private:
        std::unique_ptr< PartialStrategy > m_firstPass;
        std::unique_ptr< Strategy > m_secondPass;
    };

    /**
     * @brief Simple strategy that assigns produced chunks to reading processes
     *        in a round-Robin manner.
     *
     */
    struct RoundRobin : Strategy
    {
        ChunkTable
        assign( PartialAssignment, RankMeta const & in, RankMeta const & out );
    };

    /**
     * @brief Strategy that assigns chunks to be read by processes within
     *        the same host that produced the chunk.
     *
     * The distribution strategy within one such chunk can be flexibly
     * chosen.
     *
     */
    struct ByHostname : PartialStrategy
    {
        ByHostname( std::unique_ptr< Strategy > withinNode );

        PartialAssignment
        assign( PartialAssignment, RankMeta const & in, RankMeta const & out )
            override;

    private:
        std::unique_ptr< Strategy > m_withinNode;
    };

    /**
     * @brief Slice the n-dimensional dataset into hyperslabs and distribute
     *        chunks according to them.
     *
     * This strategy only produces chunks in the returned ChunkTable for the
     * calling parallel process.
     * Incoming chunks are intersected with the hyperslab and assigned to the
     * current parallel process in case this intersection is non-empty.
     *
     */
    struct ByCuboidSlice : Strategy
    {
        ByCuboidSlice(
            std::unique_ptr< BlockSlicer > blockSlicer,
            Extent totalExtent,
            unsigned int mpi_rank,
            unsigned int mpi_size );

        ChunkTable
        assign( PartialAssignment, RankMeta const & in, RankMeta const & out )
            override;

    private:
        std::unique_ptr< BlockSlicer > blockSlicer;
        Extent totalExtent;
        unsigned int mpi_rank, mpi_size;
    };

    /**
     * @brief Strategy that tries to assign chunks in a balanced manner without
     *        arbitrarily cutting chunks.
     *
     * Idea:
     * Calculate the ideal amount of data to be loaded per parallel process
     * and cut chunks s.t. no chunk is larger than that ideal size.
     * The resulting problem is an instance of the Bin-Packing problem which
     * can be solved by a factor-2 approximation, meaning that a reading process
     * will be assigned at worst twice the ideal amount of data.
     *
     */
    struct BinPacking : Strategy
    {
        size_t splitAlongDimension = 0;

        /**
         * @param splitAlongDimension If a chunk needs to be split, split it
         *        along this dimension.
         */
        BinPacking( size_t splitAlongDimension = 0 );

        ChunkTable
        assign( PartialAssignment, RankMeta const & in, RankMeta const & out )
            override;
    };

    /**
     * C++11 doesn't have it and it's useful for some of these.
     */
    template< typename T, typename... Args >
    std::unique_ptr< T >
    make_unique( Args &&... args )
    {
        return std::unique_ptr< T >( new T( std::forward< Args >( args )... ) );
    }
} // namespace chunk_assignment

namespace host_info
{
    enum class Method
    {
        HOSTNAME
    };

    std::string byMethod( Method );

    std::string
    hostname();
} // namespace host_info
} // namespace openPMD
