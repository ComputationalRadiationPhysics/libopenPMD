#pragma once

#include "openPMD/config.hpp"

#if openPMD_HAVE_MPI

#    include <mpi.h>
#    include <string>
#    include <vector>

namespace openPMD
{
namespace auxiliary
{
    std::vector< std::string >
    collectStringsTo(
        MPI_Comm communicator,
        int destRank,
        std::string const & thisRankString );

    std::vector< std::string >
    distributeStringsToAllRanks(
        MPI_Comm communicator,
        std::string const & thisRankString );
} // namespace auxiliary
} // namespace openPMD
#endif
