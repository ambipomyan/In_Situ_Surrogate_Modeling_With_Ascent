#!/bin/bash
set -ev

export CMAKE_ARCH=`uname -m`

if [[ ${CMAKE_ARCH} == "arm64" ]]; then
  export CMAKE_ARCH="aarch64"
fi

cmake_install_dir=/cmake-3.23.2-linux-${CMAKE_ARCH}
if [ ! -d ${cmake_install_dir} ]; then
  # setup cmake in container
  curl -L https://github.com/Kitware/CMake/releases/download/v3.23.2/cmake-3.23.2-linux-${CMAKE_ARCH}.tar.gz -o cmake-3.23.2-linux-${CMAKE_ARCH}.tar.gz
  tar -xzf cmake-3.23.2-linux-${CMAKE_ARCH}.tar.gz
fi

export PATH=$PATH:/${cmake_install_dir}/bin/

# build rocm tpls with helper script
chmod +x ascent/scripts/build_ascent/build_ascent.sh
# enable tests is of b/c one of the vtk-m tests fails to link
env enable_tests=OFF build_ascent=false enable_hip=ON ascent/scripts/build_ascent/build_ascent.sh

############################
# TODO: get spack working
############################
# variants
# TODO:
# (+genten) genten currently disabled, wait for genten master to gain cokurt
# (+dray+raja+umpire) (can't get raja or umpire to build with rocm)
#export SPACK_SPEC="%clang+mpi+vtkh~dray~raja~umpire+mfem+occa+rocm~genten~python~openmp~fortran"
# constraints
#export SPACK_SPEC="${SPACK_SPEC} ^hdf5~mpi ^mfem~rocm ^hypre~rocm ^conduit~fortran"
# config
#export SPACK_CONFIG="scripts/uberenv_configs/spack_configs/configs/alpinedav/ubuntu_20.04_rocm_5.1.3_devel/"

#cd ascent && python scripts/uberenv/uberenv.py \
#     -k \
#     --spec="${SPACK_SPEC}" \
#     --spack-config-dir="${SPACK_CONFIG}" \
#     --prefix="/uberenv_libs"

# cleanup the spack build stuff to free up space
#/uberenv_libs/spack/bin/spack clean --all

# change perms
# chmod -R a+rX /uberenv_libs

# back to where we started
#cd ../