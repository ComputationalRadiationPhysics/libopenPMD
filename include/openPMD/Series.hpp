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

#include "openPMD/backend/Attributable.hpp"
#include "openPMD/backend/Container.hpp"
#include "openPMD/IO/AbstractIOHandler.hpp"
#include "openPMD/IO/AccessType.hpp"
#include "openPMD/IO/Format.hpp"
#include "openPMD/Iteration.hpp"
#include "openPMD/IterationEncoding.hpp"
#include "openPMD/auxiliary/Visibility.hpp"

#if openPMD_HAVE_MPI
#   include <mpi.h>
#endif

#include <string>


namespace openPMD
{
/** @brief  Root level of the openPMD hierarchy.
 *
 * Entry point and common link between all iterations of particle and mesh data.
 *
 * @see https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#hierarchy-of-the-data-file
 * @see https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#iterations-and-time-series
 */
class OPENPMD_PUBLIC Series : public Attributable
{
    friend class Iteration;

public:
#if openPMD_HAVE_MPI
    static Series create(std::string const& filepath,
                         MPI_Comm comm,
                         AccessType at = AccessType::CREATE);
#endif
    static Series create(std::string const& filepath,
                         AccessType at = AccessType::CREATE);

#if openPMD_HAVE_MPI
    static Series read(std::string const& filepath,
                       MPI_Comm comm,
                       AccessType at = AccessType::READ_ONLY);
#endif
    static Series read(std::string const& filepath,
                       AccessType at = AccessType::READ_ONLY);
    ~Series();

    /**
     * @return  String representing the current enforced version of the <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#hierarchy-of-the-data-file">openPMD standard</A>.
     */
    std::string openPMD() const;
    /** Set the version of the enforced <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#hierarchy-of-the-data-file">openPMD standard</A>.
     *
     * @param   openPMD   String <CODE>MAJOR.MINOR.REVISION</CODE> of the desired version of the openPMD standard.
     * @return  Reference to modified series.
     */
    Series& setOpenPMD(std::string const& openPMD);

    /**
     * @return  32-bit mask of applied extensions to the <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#hierarchy-of-the-data-file">openPMD standard</A>.
     */
    uint32_t openPMDextension() const;
    /** Set a 32-bit mask of applied extensions to the <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#hierarchy-of-the-data-file">openPMD standard</A>.
     *
     * @param   openPMDextension  Unsigned 32-bit integer used as a bit-mask of applied extensions.
     * @return  Reference to modified series.
     */
    Series& setOpenPMDextension(uint32_t openPMDextension);

    /**
     * @return  String representing the common prefix for all data sets and sub-groups of a specific iteration.
     */
    std::string basePath() const;
    /** Set the common prefix for all data sets and sub-groups of a specific iteration.
     *
     * @param   basePath    String of the common prefix for all data sets and sub-groups of a specific iteration.
     * @return  Reference to modified series.
     */
    Series& setBasePath(std::string const& basePath);

    /**
     * @throw   no_such_attribute_error If optional attribute is not present.
     * @return  String representing the path to mesh records, relative(!) to <CODE>basePath</CODE>.
     */
    std::string meshesPath() const;
    /** Set the path to <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#mesh-based-records">mesh records</A>, relative(!) to <CODE>basePath</CODE>.
     *
     * @param   meshesPath  String of the path to <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#mesh-based-records">mesh records</A>, relative(!) to <CODE>basePath</CODE>.
     * @return  Reference to modified series.
     */
    Series& setMeshesPath(std::string const& meshesPath);

    /**
     * @throw   no_such_attribute_error If optional attribute is not present.
     * @return  String representing the path to particle species, relative(!) to <CODE>basePath</CODE>.
     */
    std::string particlesPath() const;
    /** Set the path to groups for each <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#particle-records">particle species</A>, relative(!) to <CODE>basePath</CODE>.
     *
     * @param   particlesPath   String of the path to groups for each <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#particle-records">particle species</A>, relative(!) to <CODE>basePath</CODE>.
     * @return  Reference to modified series.
     */
    Series& setParticlesPath(std::string const& particlesPath);

    /**
     * @throw   no_such_attribute_error If optional attribute is not present.
     * @return  String indicating author and contact for the information in the file.
     */
    std::string author() const;
    /** Indicate the author and contact for the information in the file.
     *
     * @param   author  String indicating author and contact for the information in the file.
     * @return  Reference to modified series.
     */
    Series& setAuthor(std::string const& author);

    /**
     * @throw   no_such_attribute_error If optional attribute is not present.
     * @return  String indicating the software/code/simulation that created the file;
     */
    std::string software() const;
    /** Indicate the software/code/simulation that created the file.
     *
     * @param   software    String indicating the software/code/simulation that created the file.
     * @return  Reference to modified series.
     */
    Series& setSoftware(std::string const& software);

    /**
     * @throw   no_such_attribute_error If optional attribute is not present.
     * @return  String indicating the version of the software/code/simulation that created the file.
     */
    std::string softwareVersion() const;
    /** Indicate the version of the software/code/simulation that created the file.
     *
     * @param   softwareVersion String indicating the version of the software/code/simulation that created the file.
     * @return  Reference to modified series.
     */
    Series& setSoftwareVersion(std::string const& softwareVersion);

    /**
     * @throw   no_such_attribute_error If optional attribute is not present.
     * @return  String indicating date of creation.
     */
    std::string date() const;
    /** Indicate the date of creation.
     *
     * @param   date    String indicating the date of creation.
     * @return  Reference to modified series.
     */
    Series& setDate(std::string const& date);

    /**
     * @throw   no_such_attribute_error If optional attribute is not present.
     * @return  String indicating dependencies of software that were used to create the file.
     */
    std::string softwareDependencies() const;
    /** Indicate dependencies of software that were used to create the file.
     *
     * @param   softwareDependencies    String indicating dependencies of software that were used to create the file (semicolon-separated list if needed).
     * @return  Reference to modified series.
     */
    Series& setSoftwareDependencies(std::string const& softwareDependencies);

    /**
     * @throw   no_such_attribute_error If optional attribute is not present.
     * @return  String indicating the machine or relevant hardware that created the file.
     */
    std::string machine() const;
    /** Indicate the machine or relevant hardware that created the file.
     *
     * @param   machine String indicating the machine or relevant hardware that created the file (semicolon-separated list if needed)..
     * @return  Reference to modified series.
     */
    Series& setMachine(std::string const& machine);

    /**
     * @return  Current encoding style for multiple iterations in this series.
     */
    IterationEncoding iterationEncoding() const;
    /** Set the <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#iterations-and-time-series">encoding style</A> for multiple iterations in this series.
     *
     * @param   iterationEncoding   Desired <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#iterations-and-time-series">encoding style</A> for multiple iterations in this series.
     * @return  Reference to modified series.
     */
    Series& setIterationEncoding(IterationEncoding iterationEncoding);

    /**
     * @return  String describing a <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#iterations-and-time-series">pattern</A> describing how to access single iterations in the raw file.
     */
    std::string iterationFormat() const;
    /** Set a <A HREF="https://github.com/openPMD/openPMD-standard/blob/latest/STANDARD.md#iterations-and-time-series">pattern</A> describing how to access single iterations in the raw file.
     *
     * @param   iterationFormat String with the iteration regex <CODE>\%T</CODE> defining either
     *                          the series of files (fileBased)
     *                          or the series of groups within a single file (groupBased)
     *                          that allows to extract the iteration from it.\n
     *                          For fileBased formats the iteration must be included in the file name.\n
     *                          The format depends on the selected iterationEncoding method.
     * @return  Reference to modified series.
     */
    Series& setIterationFormat(std::string const& iterationFormat);

    /**
     * @return  String of a pattern for file names.
     */
    std::string name() const;
    /** Set the pattern for file names.
     *
     * @param   name    String of the pattern for file names. Must include iteration regex <CODE>\%T</CODE> for fileBased data.
     * @return  Reference to modified series.
     */
    Series& setName(std::string const& name);

    /** Execute all required remaining IO operations to write or read data.
     */
    void flush();

    Container< Iteration, uint64_t > iterations;

private:
#if openPMD_HAVE_MPI
    Series(std::string const& filepath,
           AccessType at,
           MPI_Comm comm);
#endif
    Series(std::string const& filepath,
           AccessType at);

    void flushFileBased();
    void flushGroupBased();
    void flushMeshesPath();
    void flushParticlesPath();
    void readFileBased();
    void readGroupBased();
    void readBase();
    void read();


    constexpr static char const * const OPENPMD = "1.1.0";
    constexpr static char const * const BASEPATH = "/data/%T/";

    IterationEncoding m_iterationEncoding;
    std::string m_name;
    Format m_format;
};  //Series

/** Determine the storage format of a Series from the used filename extension.
 *
 * @param   filename    tring containing the filename.
 * @return  Format that best fits the filename extension.
 */
OPENPMD_PUBLIC
Format determineFormat(std::string const& filename);

/** Remove the filename extension of a given storage format.
 *
 * @param   filename    String containing the filename, possibly with filename extension.
 * @param   f           File format to remove filename extension for.
 * @return  String containing the filename without filename extension.
 */
OPENPMD_PUBLIC
std::string cleanFilename(std::string const& filename, Format f);

/** Create a functor to determine if a file can be of a format given the filename on disk.
 *
 * @param   name        String containing desired filename without filename extension.
 * @param   f           File format to check backend applicability for.
 * @return  Functor returning true if file could be of type f. False otherwise.
 */
OPENPMD_PUBLIC
std::function< bool(std::string const&) > matcher(std::string const& name, Format f);
} // openPMD
