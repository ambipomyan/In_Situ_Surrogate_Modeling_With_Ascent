# Copyright (c) Lawrence Livermore National Security, LLC and other Ascent
# Project developers. See top-level LICENSE AND COPYRIGHT files for dates and
# other details. No copyright assignment is required to contribute to Ascent.

FROM nvidia/cuda:11.8.0-devel-ubuntu22.04

# add sudo to base cuda devel env
# so we can install additional packages as
# non-root, but admin default user on azure pipelines
RUN apt-get update && apt-get -y install sudo
# tzdata install wants to ask questions, so handled as sep case
RUN DEBIAN_FRONTEND="noninteractive" apt-get -y install tzdata
# install std packages we need for cuda dev env and test
RUN apt-get update && apt-get -y install \
               git \
               git-lfs \
               gfortran \
               python3 \
               python3-dev \
               python-is-python3 \
               perl \
               git \
               git-lfs \
               curl \
               libcurl4-openssl-dev \
               wget \
               tar \
               unzip \
               file \
               zlib1g-dev \
               curl \
               mpich \
               libmpich-dev \
               libblas-dev \
               liblapack-dev \
               liblapacke-dev \
               unzip \
               vim


