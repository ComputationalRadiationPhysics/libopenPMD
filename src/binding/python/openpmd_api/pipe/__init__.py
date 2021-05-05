"""
This file is part of the openPMD-api.

This module provides functions that are wrapped into sys.exit(...()) calls by
the setuptools (setup.py) "entry_points" -> "console_scripts" generator.

Copyright 2021 openPMD contributors
Authors: Franz Poeschel
License: LGPLv3+
"""
from .. import openpmd_api_cxx as io
import argparse
import sys  # sys.stderr.write

# MPI is an optional dependency
try:
    from mpi4py import MPI
    HAVE_MPI = True
except ImportError:
    HAVE_MPI = False

debug = True


class FallbackMPICommunicator:
    def __init__(self):
        self.size = 1
        self.rank = 0


def parse_args():
    parser = argparse.ArgumentParser(description="""
openPMD Pipe.

This tool connects an openPMD-based data source with an openPMD-based data sink
and forwards all data from source to sink.
Possible uses include conversion of data from one backend to another one
or multiplexing the data path in streaming setups.
Parallelization with MPI is optionally possible and is done automatically
as soon as the mpi4py package is found and this tool is called in an MPI
context. In that case, each dataset will be equally sliced along the dimension
with the largest extent.""")

    parser.add_argument('--infile', type=str, help='In file')
    parser.add_argument('--outfile', type=str, help='Out file')
    parser.add_argument('--inconfig',
                        type=str,
                        default='{}',
                        help='JSON config for the in file')
    parser.add_argument('--outconfig',
                        type=str,
                        default='{}',
                        help='JSON config for the out file')

    return parser.parse_args()


class deferred_load:
    def __init__(self, source, dynamicView, offset, extent):
        self.source = source
        self.dynamicView = dynamicView
        self.offset = offset
        self.extent = extent


class particle_patch_load:
    """
    A deferred load/store operation for a particle patch.
    Our particle-patch API requires that users pass a concrete value for
    storing, even if the actual write operation occurs much later at
    series.flush().
    So, unlike other record components, we cannot call .store_chunk() with
    a buffer that has not yet been filled, but must wait until the point where
    we actual have the data at hand already.
    In short: calling .store() must be deferred, until the data has been fully
    read from the sink.
    This class stores the needed parameters to .store().
    """
    def __init__(self, data, dest):
        self.data = data
        self.dest = dest

    def run(self):
        for index, item in enumerate(self.data):
            self.dest.store(index, item)


class pipe:
    """
    Represents the configuration of one "pipe" pass.
    """
    def __init__(self, infile, outfile, inconfig, outconfig, comm):
        self.infile = infile
        self.outfile = outfile
        self.inconfig = inconfig
        self.outconfig = outconfig
        self.loads = []
        self.comm = comm
        if HAVE_MPI:
            hostinfo = io.HostInfo.HOSTNAME
            self.outranks = hostinfo.get_collective(self.comm)
        else:
            self.outranks = {i: str(i) for i in range(self.comm.size)}

    def run(self):
        if self.comm.size == 1:
            print("Opening data source")
            sys.stdout.flush()
            inseries = io.Series(self.infile, io.Access.read_only,
                                 self.inconfig)
            print("Opening data sink")
            sys.stdout.flush()
            outseries = io.Series(self.outfile, io.Access.create,
                                  self.outconfig)
            print("Opened input and output")
            sys.stdout.flush()
        else:
            print("Opening data source on rank {}.".format(self.comm.rank))
            sys.stdout.flush()
            inseries = io.Series(self.infile, io.Access.read_only, self.comm,
                                 self.inconfig)
            print("Opening data sink on rank {}.".format(self.comm.rank))
            sys.stdout.flush()
            outseries = io.Series(self.outfile, io.Access.create, self.comm,
                                  self.outconfig)
            print("Opened input and output on rank {}.".format(self.comm.rank))
            sys.stdout.flush()
        if inseries.contains_attribute("rankMetaInfo"):
            self.inranks = inseries.mpi_ranks_meta_info
        else:
            self.inranks = {}
        self.__copy(inseries, outseries)

    def __copy(self, src, dest, current_path="/data/"):
        """
        Worker method.
        Copies data from src to dest. May represent any point in the openPMD
        hierarchy, but src and dest must both represent the same layer.
        """
        if (type(src) != type(dest)
                and not isinstance(src, io.IndexedIteration)
                and not isinstance(dest, io.Iteration)):
            raise RuntimeError(
                "Internal error: Trying to copy mismatching types")
        attribute_dtypes = src.attribute_dtypes
        for key in src.attributes:
            attr = src.get_attribute(key)
            attr_type = attribute_dtypes[key]
            dest.set_attribute(key, attr, attr_type)
        container_types = [
            io.Mesh_Container, io.Particle_Container, io.ParticleSpecies,
            io.Record, io.Mesh, io.Particle_Patches, io.Patch_Record
        ]
        if isinstance(src, io.Series):
            # main loop: read iterations of src, write to dest
            write_iterations = dest.write_iterations()
            for in_iteration in src.read_iterations():
                if self.comm.rank == 0:
                    print("Iteration {0} contains {1} meshes:".format(
                        in_iteration.iteration_index,
                        len(in_iteration.meshes)))
                    for m in in_iteration.meshes:
                        print("\t {0}".format(m))
                    print("")
                    print(
                        "Iteration {0} contains {1} particle species:".format(
                            in_iteration.iteration_index,
                            len(in_iteration.particles)))
                    for ps in in_iteration.particles:
                        print("\t {0}".format(ps))
                        print("With records:")
                        for r in in_iteration.particles[ps]:
                            print("\t {0}".format(r))
                out_iteration = write_iterations[in_iteration.iteration_index]
                sys.stdout.flush()
                self.__particle_patches = []
                self.__copy(
                    in_iteration, out_iteration,
                    current_path + str(in_iteration.iteration_index) + "/")
                for deferred in self.loads:
                    deferred.source.load_chunk(
                        deferred.dynamicView.current_buffer(), deferred.offset,
                        deferred.extent)
                in_iteration.close()
                for patch_load in self.__particle_patches:
                    patch_load.run()
                out_iteration.close()
                self.__particle_patches.clear()
                self.loads.clear()
                sys.stdout.flush()
        elif isinstance(src, io.Record_Component):
            shape = src.shape
            dtype = src.dtype
            dest.reset_dataset(io.Dataset(dtype, shape))
            if src.empty:
                # empty record component automatically created by
                # dest.reset_dataset()
                pass
            elif src.constant:
                dest.make_constant(src.get_attribute("value"))
            else:
                chunk_table = src.available_chunks()
                strategy = io.BinPacking()
                my_chunks = strategy.assign_chunks(chunk_table, self.inranks,
                                                   self.outranks)
                for chunk in [
                        chunk for chunk in my_chunks
                        if chunk.source_id == self.comm.rank
                ]:
                    if debug:
                        end = chunk.offset.copy()
                        for i in range(len(end)):
                            end[i] += chunk.extent[i]
                        print("{}\t{}/{}:\t{} -- {}".format(
                            current_path, self.comm.rank, self.comm.size,
                            chunk.offset, end))
                    span = dest.store_chunk(chunk.offset, chunk.extent)
                    self.loads.append(
                        deferred_load(src, span, chunk.offset, chunk.extent))
        elif isinstance(src, io.Patch_Record_Component):
            dest.reset_dataset(io.Dataset(src.dtype, src.shape))
            if self.comm.rank == 0:
                self.__particle_patches.append(
                    particle_patch_load(src.load(), dest))
        elif isinstance(src, io.Iteration):
            self.__copy(src.meshes, dest.meshes, current_path + "meshes/")
            self.__copy(src.particles, dest.particles,
                        current_path + "particles/")
        elif any([
                isinstance(src, container_type)
                for container_type in container_types
        ]):
            for key in src:
                self.__copy(src[key], dest[key], current_path + key + "/")
            if isinstance(src, io.ParticleSpecies):
                self.__copy(src.particle_patches, dest.particle_patches)
        else:
            raise RuntimeError("Unknown openPMD class: " + str(src))


def main():
    args = parse_args()
    if not args.infile or not args.outfile:
        print("Please specify parameters --infile and --outfile.")
        sys.exit(1)
    if (HAVE_MPI):
        run_pipe = pipe(args.infile, args.outfile, args.inconfig,
                        args.outconfig, MPI.COMM_WORLD)
    else:
        run_pipe = pipe(args.infile, args.outfile, args.inconfig,
                        args.outconfig, FallbackMPICommunicator())

    run_pipe.run()


if __name__ == "__main__":
    main()
    sys.exit()
