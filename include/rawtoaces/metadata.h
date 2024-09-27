// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#ifndef RTA_METADATA_H_
#define RTA_METADATA_H_

namespace rta
{

struct Metadata
{
    // Colorimetry
    std::vector<double> cameraCalibration1;
    std::vector<double> cameraCalibration2;
    std::vector<double> xyz2rgbMatrix1;
    std::vector<double> xyz2rgbMatrix2;
    double              calibrationIlluminant1;
    double              calibrationIlluminant2;

    std::vector<double> analogBalance;
    std::vector<double> neutralRGB;

    double baselineExposure;
};

} // namespace rta

#endif // RTA_METADATA_H_
