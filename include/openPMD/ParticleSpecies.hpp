/* Copyright 2017-2019 Fabian Koller
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

#include "openPMD/backend/Attributable.hpp"
#include "openPMD/backend/Container.hpp"
#include "openPMD/ParticlePatches.hpp"
#include "openPMD/Record.hpp"

#include <string>


namespace openPMD
{

class ParticleSpecies : public Container< Record >
{
    friend class Container< ParticleSpecies >;
    friend class Container< Record >;
    friend class Iteration;

public:
    ParticlePatches particlePatches;

private:
    ParticleSpecies(std::shared_ptr< Writable > const& w);

    void read();
    void flush(std::string const &) override;
};

namespace traits
{
    template<>
    struct GenerationPolicy< ParticleSpecies, true >
    {
        template< typename T >
        void operator()(T & ret)
        {
            /* enforce these two RecordComponents as required by the standard */
            //auto udl = std::array< double, 7 >({1., 0., 0., 0., 0., 0., 0.});
            //auto& po = 
            ret.init("position"); // .setUnitDimension({{UnitDimension::L, 1}});
            //po.initAttribute("unitDimension", udl);
            //auto& pp = 
            ret.init("positionOffset"); //.setUnitDimension({{UnitDimension::L, 1}});
            //pp.initAttribute("unitDimension", udl);
            ret.particlePatches.linkHierarchy(ret.m_writable);

            auto& np = ret.particlePatches.init("numParticles");
            auto& npc = np.init(RecordComponent::SCALAR);
            npc.resetDataset(Dataset(determineDatatype<uint64_t>(), {1}));
            npc.parent = np.parent;
            auto& npo = ret.particlePatches.init("numParticlesOffset");
            auto& npoc = npo.init(RecordComponent::SCALAR);
            npoc.resetDataset(Dataset(determineDatatype<uint64_t>(), {1}));
            npoc.parent = npo.parent;
        }
    };

    template<>
    struct GenerationPolicy< ParticleSpecies, false >
    {
        template< typename T >
        void operator()(T & ret)
        {
            /* enforce these two RecordComponents as required by the standard */
            ret["position"].setUnitDimension({{UnitDimension::L, 1}});
            ret["positionOffset"].setUnitDimension({{UnitDimension::L, 1}});
            ret.particlePatches.linkHierarchy(ret.m_writable);

            auto& np = ret.particlePatches["numParticles"];
            auto& npc = np[RecordComponent::SCALAR];
            npc.resetDataset(Dataset(determineDatatype<uint64_t>(), {1}));
            npc.parent = np.parent;
            auto& npo = ret.particlePatches["numParticlesOffset"];
            auto& npoc = npo[RecordComponent::SCALAR];
            npoc.resetDataset(Dataset(determineDatatype<uint64_t>(), {1}));
            npoc.parent = npo.parent;
        }
    };
} // traits
} // openPMD
