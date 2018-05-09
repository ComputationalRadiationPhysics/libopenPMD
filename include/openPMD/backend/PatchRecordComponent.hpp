/* Copyright 2017-2018 Fabian Koller
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

#include "openPMD/backend/BaseRecordComponent.hpp"
#include "openPMD/auxiliary/Visibility.hpp"

#include <unordered_map>
#include <string>


namespace openPMD
{
class OPENPMD_PUBLIC PatchRecordComponent : public BaseRecordComponent
{
    template<
        typename T,
        typename T_key,
        typename T_container
    >
    friend
    class Container;

    friend class ParticlePatches;
    friend class PatchRecord;

public:
    PatchRecordComponent& setUnitSI(double);

    PatchRecordComponent& resetDataset(Dataset);

    uint8_t getDimensionality() const;
    Extent getExtent() const;

    template< typename T >
    void load(size_t idx, std::unique_ptr< T >&);
    template< typename T >
    void store(size_t idx, T);

private:
    PatchRecordComponent();

    void flush(std::string const&);
    void read();

    std::queue< IOTask > m_chunks;
};  //PatchRecordComponent


template< typename T >
inline void
PatchRecordComponent::load(size_t idx, std::unique_ptr< T >& data)
{
    Datatype dtype = determineDatatype< T >();
    if( dtype != getDatatype() )
        throw std::runtime_error("Type conversion during particle patch loading not yet implemented");

    Extent dse = getExtent();
    if( dse[0] < idx )
        throw std::runtime_error("Index does not reside inside patch (patch: " + std::to_string(dse[0])
                                 + " - index: " + std::to_string(idx) + ")");

    if( !data )
        data = std::unique_ptr< T >(new T);
    T* raw_ptr = data.get();

    Parameter< Operation::READ_DATASET > dRead;
    dRead.offset = {idx};
    dRead.extent = {1};
    dRead.dtype = getDatatype();
    dRead.data = raw_ptr;
    IOHandler->enqueue(IOTask(this, dRead));
    IOHandler->flush();
}

template< typename T >
inline void
PatchRecordComponent::store(size_t idx, T data)
{
    Datatype dtype = determineDatatype< T >();
    if( dtype != getDatatype() )
        throw std::runtime_error("Datatypes of chunk and dataset do not match.");

    Extent dse = getExtent();
    if( dse[0] < idx )
        throw std::runtime_error("Index does not reside inside patch (patch: " + std::to_string(dse[0])
                                 + " - index: " + std::to_string(idx) + ")");

    Parameter< Operation::WRITE_DATASET > dWrite;
    dWrite.offset = {idx};
    dWrite.extent = {1};
    dWrite.dtype = dtype;
    dWrite.data = std::make_shared< T >(data);
    m_chunks.push(IOTask(this, dWrite));
}
} // openPMD
