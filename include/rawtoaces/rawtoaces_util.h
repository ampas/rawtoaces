// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#ifndef _RAWTOACES_UTIL_H_
#define _RAWTOACES_UTIL_H_

#include <OpenImageIO/argparse.h>
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>

namespace rta
{

/// An image converter converts an image read from a camera raw image file
/// into an ACESContainer compatible image.
class ImageConverter
{
public:
    ImageConverter();

    /// The  white balancing method to use for conversion can be specified
    ///
    enum class WBMethod
    {
        /// Use the metadata provided in the image file. This mode is mostly
        /// usable with DNG files, as the information needed for conversion
        /// is mandatory in the DNG format.
        Metadata,
        /// White balance to a specified illuminant. See the `illuminant`
        /// property for more information on the supported illuminants. This
        /// mode can only be used if spectral sensitivities are available for
        /// the camera.
        Illuminant,
        /// Calculate white balance by averaging over a specified region of
        /// the image. See `wbBox`. In this mode if an empty box if provided,
        /// white balancing is done by averaging over the whole image.
        Box,
        /// Use custom white balancing multipliers. This mode is useful if
        /// the white balancing coefficients are calculated by an external
        /// tool.
        Custom
    } wbMethod = WBMethod::Metadata;

    enum class MatrixMethod
    {
        /// Use the camera spectral sensitivity curves to solve for the colour
        /// conversion matrix. In this mode the illuminant is either provided
        /// directly in `illuminant` if `wbMethod` ==
        /// `WBMethod::Illuminant`, or the best illuminant is derived from the
        /// white balancing multipliers.
        Spectral,
        /// Use the metadata provided in the image file. This mode is mostly
        /// usable with DNG files, as the information needed for conversion
        /// is mandatory in the DNG format.
        Metadata,
        /// Use the Adobe colour matrix for the camera supplied in LibRaw.
        Adobe,
        /// Specify a custom matrix in `colourMatrix`. This mode is useful if
        /// the matrix is calculated by an external tool.
        Custom
    } matrixMethod = MatrixMethod::Spectral;

    /// An illuminant to use for white balancing and/or colour matrix
    /// calculation. Only used when `wbMethod` ==
    /// `WBMethod::Illuminant` or `matrixMethod` == `MatrixMethod::Spectral`.
    /// An illuminant can be provided as a black body correlated colour
    /// temperature, like `3200K`; or a D-series illuminant, like `D56`;
    /// or any other illuminant, in such case it must be present in the data
    /// folder.
    std::string illuminant;

    float headroom           = 6.0;
    int   wbBox[4]           = { 0 };
    float customWB[4]        = { 1.0, 1.0, 1.0, 1.0 };
    float customMatrix[3][3] = { { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };

    bool  no_auto_bright           = false;
    float adjust_maximum_threshold = 0.75;
    int   black_level              = -1;
    int   saturation_level         = -1;
    bool  half_size                = false;
    int   highlight_mode           = 0;
    int   flip                     = 0;
    int   cropbox[4]               = { 0, 0, 0, 0 };

    int verbosity;

    /// Returns a class which will be used for parsing the command line
    /// parameters. Can be used to add additional parameters before calling
    /// `parse()`. The additional parameters will be parsed by ignored by this
    /// class.
    /// @result A non-const reference to an
    /// initialised OIIO::ArgParse class.
    OIIO::ArgParse &argParse();

    /// Returns an image buffer, containing the image in the current state
    /// after the previous step of processing. The image can be modified before
    /// executing the next step if needed.
    /// @result A non-const reference to the image buffer.
    OIIO::ImageBuf &imageBuffer();

    /// Parse the command line parameters. This method can be used to
    /// configure the converter instead of modifying each conversion parameter
    /// individually. Additional command line parameters can be added by
    /// modifying the structure returned by `argParse()`.
    /// @param argc
    ///    number of parameters
    /// @param argv
    ///    list of parameters
    /// @result
    ///    `true` if parsed successfully
    bool parse( int argc, const char *argv[] );

    /// Configures the converter using the requested white balance and colour
    /// matrix method, and the metadata of the file provided in `input_file`.
    /// @param input_filename
    ///    A file name of the raw image file to read the metadata from.
    /// @result
    ///    `true` if configured successfully.
    bool configure( const std::string &input_filename );

    /// Loads an image to convert. Note that the image file name in
    /// `input_filename` can be differnt from the one used in `configure()`.
    /// This is useful for configuring the converter using one image, and
    /// applying the conversion to a different one, or multiple images.
    /// @param input_filename
    ///    A file name of the raw image file to read the pixels from.
    /// @result
    ///    `true` if loaded successfully.
    bool load( const std::string &input_filename );

    /// Converts the image from raw camera colour space to ACES.
    /// @result
    ///    `true` if converted successfully.
    bool process();

    /// Saves the image into ACES Container.
    /// @result
    ///    `true` if saved successfully.
    bool save( const std::string &output_filename );

private:
    void initArgParse();
    void prepareIDT_DNG();
    void prepareIDT_nonDNG();
    void prepareIDT_spectral( bool calc_white_balance, bool calc_matrix );
    void applyMatrix( const std::vector<std::vector<double>> &matrix );

    bool        _is_DNG;
    std::string _configFilename;
    std::string _cameraMake;
    std::string _cameraModel;

    OIIO::ArgParse  _argParse;
    OIIO::ImageSpec _inputHint;
    OIIO::ImageSpec _inputFull;
    OIIO::ImageBuf  _imageBuffer;

    std::vector<double>              _WB_mults;
    std::vector<std::vector<double>> _IDT_matrix;
    std::vector<std::vector<double>> _CAT_matrix;
};

} // namespace rta

#endif // _RAWTOACES_UTIL_H_
