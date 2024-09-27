#!/usr/bin/env bash

set -ex

vcpkg install \
    libraw:x64-windows \
    ceres:x64-windows \
    imath:x64-windows \
    openimageio:x64-windows \
    boost-system:x64-windows \
    boost-foreach:x64-windows \
    boost-filesystem:x64-windows \
    boost-test:x64-windows \
    boost-property-tree:x64-windows
