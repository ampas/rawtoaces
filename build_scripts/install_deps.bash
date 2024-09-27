#!/usr/bin/env bash

set -ex

time sudo apt-get update

#time sudo apt-get -q -f install -y libunwind-dev

time sudo apt-get -q -f install -y \
    libunwind-dev libilmbase-dev libopenexr-dev \
    libopenimageio-dev \
    libboost-dev libboost-thread-dev libboost-filesystem-dev \
    libboost-test-dev \
    libraw-dev libceres-dev
