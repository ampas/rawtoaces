#!/usr/bin/env bash

set -ex

time sudo apt-get update

time sudo apt-get -q -f install -y \
    libunwind-dev \
    libimath-dev libopenexr-dev \
    libboost-dev libboost-filesystem-dev \
    libboost-test-dev \
    libraw-dev libceres-dev \
    libopencv-dev \
    openimageio-tools \
    libopenimageio-dev
