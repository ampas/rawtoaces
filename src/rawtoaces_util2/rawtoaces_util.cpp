// Copyright Contributors to the rawtoaces project.
// SPDX-License-Identifier: Apache-2.0
// https://github.com/AcademySoftwareFoundation/rawtoaces

#include <rawtoaces/rawtoaces_util.h>
#include <rawtoaces/define.h>
#include <rawtoaces/metadata.h>
#include <rawtoaces/rta.h>

#include <OpenImageIO/imageio.h>
#include <OpenImageIO/imagebuf.h>
#include <OpenImageIO/imagebufalgo.h>

#include <filesystem>

namespace rta
{

std::string findFile( const std::string &filename )
{
    auto paths = pathsFinder();

    for ( auto &path: paths.paths )
    {
        std::string full_path = path + "/" + filename;
        if ( std::filesystem::exists( full_path ) )
            return full_path;
    }
    return "";
}

std::vector<std::string> collectDataFiles( const std::string &type )
{
    std::vector<std::string> result;

    auto paths = pathsFinder();

    for ( auto &path: paths.paths )
    {
        auto it = std::filesystem::directory_iterator( path + "/" + type );

        for ( auto filename2: it )
        {
            auto p = filename2.path();
            if ( filename2.path().extension() == ".json" )
            {
                result.push_back( filename2.path().string() );
            }
        }
    }
    return result;
}

ImageConverter::ImageConverter()
{
    initArgParse();
}

const char *HelpString =
    "Rawtoaces converts raw image files from a digital camera to "
    "the Academy Colour Encoding System (ACES) compliant images.\n"
    "The process consists of two parts:\n"
    "- the colour values get converted from the camera native colour "
    "space to the ACES AP0 (see \"SMPTE ST 2065-1\"), and \n"
    "- the image file gets converted from the camera native raw "
    "file format to the ACES Image Container file format "
    "(see \"SMPTE ST 2065-4\").\n"
    "\n"
    "Rawtoaces supports the following white-balancing modes:\n"
    "- \"metadata\" uses the white-balancing coefficients from the raw "
    "image file, provided by the camera.\n"
    "- \"illuminant\" performs white balancing to the illuminant, "
    "provided in the \"--illuminant\" parameter. The list of the "
    "supported illuminants can be seen using the "
    "\"--list-illuminants\" parameter. This mode requires spectral "
    "sensitivity data for the camera model the image comes from. "
    "The list of cameras such data is available for, can be "
    "seen using the \"--list-cameras\" parameter.\n"
    "- \"box\" performs white-balancing to make the given region of "
    "the image appear neutral gray. The box position (origin and size) "
    "can be specified using the \"--wb-box\" parameter. In case no such "
    "parameter provided, the whole image is used for white-balancing.\n"
    "- \"custom\" uses the custom white balancing coefficients "
    "provided using the -\"custom-wb\" parameter.\n"
    "\n"
    "Rawtoaces supports the following methods of color matrix "
    "computation:\n"
    "- \"spectral\" uses the camera sensor's spectral sensitivity data "
    "to compute the optimal matrix. This mode requires spectral "
    "sensitivity data for the camera model the image comes from. "
    "The list of cameras such data is available for, can be "
    "seen using the \"--list-cameras\" parameter.\n"
    "- \"metadata\" uses the matrix (matrices) contained in the raw "
    "image file metadata. This mode works best with the images using "
    "the DNG format, as the DNG standard mandates the presense of "
    "such matrices.\n"
    "- \"Adobe\" uses the Adobe coefficients provided by LibRaw. \n"
    "- \"custom\" uses a user-provided color conversion matrix. "
    "A matrix can be specified using the \"--custom-mat\" parameter.\n"
    "\n"
    "The paths rawtoaces uses to search for the spectral sensitivity "
    "data can be specified in the AMPAS_DATA_PATH environment "
    "variable.\n";

const char *UsageString =
    "\n"
    "    rawtoaces --wb-method METHOD --mat-method METHOD [PARAMS] "
    "path/to/dir/or/file ...\n"
    "Examples: \n"
    "    rawtoaces --wb-method metadata --mat-method metadata raw_file.cr3\n"
    "    rawtoaces --wb-method illuminant --illuminant 3200K --mat-method "
    "spectral raw_file.cr3\n";

void ImageConverter::initArgParse()
{
    _argParse.intro( HelpString ).usage( UsageString );
    _argParse.print_defaults( true );
    _argParse.add_help( true );
    _argParse.add_version( "VERSION NUMBER" );

    _argParse.arg( "filename" ).action( OIIO::ArgParse::append() ).hidden();

    _argParse.arg( "--wb-method" )
        .help(
            "White balance method. Supported options: metadata, illuminant, "
            "box, custom." )
        .metavar( "STR" )
        .defaultval( "metadata" )
        .action( OIIO::ArgParse::store() );

    _argParse.arg( "--mat-method" )
        .help(
            "IDT matrix calculation method. Supported options: spectral, "
            "metadata, Adobe, custom." )
        .metavar( "STR" )
        .defaultval( "spectral" )
        .action( OIIO::ArgParse::store() );

    _argParse.arg( "--illuminant" )
        .help( "Illuminant for white balancing. (default = D55)" )
        .metavar( "STR" )
        .action( OIIO::ArgParse::store() );

    _argParse.arg( "--wb-box" )
        .help(
            "Box to use for white balancing. (default = (0,0,0,0) - full "
            "image)" )
        .nargs( 4 )
        .metavar( "X Y W H" )
        .action( OIIO::ArgParse::store<int>() );

    _argParse.arg( "--custom-wb" )
        .help( "Custom white balance multipliers." )
        .nargs( 4 )
        .metavar( "R G B G" )
        .action( OIIO::ArgParse::store<float>() );

    _argParse.arg( "--custom-mat" )
        .help( "Custom camera RGB to XYZ matrix." )
        .nargs( 9 )
        .metavar( "Rr Rg Rb Gr Gg Gb Br Bg Bb" )
        .action( OIIO::ArgParse::store<float>() );

    _argParse.arg( "--headroom" )
        .help( "Highlight headroom factor." )
        .metavar( "VAL" )
        .defaultval( 6.0f )
        .action( OIIO::ArgParse::store<float>() );

    _argParse.separator( "Raw conversion options:" );

    _argParse.arg( "--no-auto-bright" )
        .help( "Disable automatic exposure adjustment." )
        .action( OIIO::ArgParse::store_true() );

    _argParse.arg( "--adjust-maximum-threshold" )
        .help(
            "Automatically lower the linearity threshold provided in the "
            "metadata by this scaling factor." )
        .metavar( "VAL" )
        .defaultval( 0.75f )
        .action( OIIO::ArgParse::store<float>() );

    _argParse.arg( "--black-level" )
        .help( "If >= 0, override the black level." )
        .metavar( "VAL" )
        .defaultval( -1 )
        .action( OIIO::ArgParse::store<int>() );

    _argParse.arg( "--saturation-level" )
        .help(
            "If not 0, override the level which appears to be saturated "
            "after normalisation." )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    _argParse.arg( "--chromatic-aberration" )
        .help(
            "Red and blue scale factors for chromatic aberration correction. "
            "The value of 1 means no correction." )
        .metavar( "R B" )
        .nargs( 2 )
        .defaultval( 1.0f )
        .action( OIIO::ArgParse::store<float>() );

    _argParse.arg( "--half-size" )
        .help( "If present, decode image at half size resolution." )
        .action( OIIO::ArgParse::store_true() );

    _argParse.arg( "--highlight-mode" )
        .help( "0 = clip, 1 = unclip, 2 = blend, 3..9 = rebuild." )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    _argParse.arg( "--cropbox" )
        .help(
            "Apply custom crop. If not present, the default crop is applied, "
            "which should match the crop of the in-camera JPEG." )
        .nargs( 4 )
        .metavar( "X Y W H" )
        .action( OIIO::ArgParse::store<int>() );

    _argParse.arg( "--flip" )
        .help(
            "If not 0, override the orientation specified in the metadata. "
            "1..8 correspond to EXIF orientation codes "
            "(3 = 180 deg, 6 = 90 deg CCW, 8 = 90 deg CW.)" )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    _argParse.arg( "--denoise_threshold" )
        .help( "Wavelet denoising threshold" )
        .metavar( "VAL" )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );

    _argParse.separator( "Benchmarking and debugging:" );

    _argParse.arg( "--list-cameras" )
        .help( "Shows the list of cameras supported in spectral mode." )
        .action( OIIO::ArgParse::store_true() );

    _argParse.arg( "--list-illuminants" )
        .help( "Shows the list of illuminants supported in spectral mode." )
        .action( OIIO::ArgParse::store_true() );

    _argParse.arg( "--verbose" )
        .help( "Verbosity level. 0 = off, 1 = info, 2 = debug." )
        .defaultval( 0 )
        .action( OIIO::ArgParse::store<int>() );
}

OIIO::ArgParse &ImageConverter::argParse()
{
    return _argParse;
}

OIIO::ImageBuf &ImageConverter::imageBuffer()
{
    return _imageBuffer;
}

template <typename T, typename F1, typename F2>
bool check_param(
    const std::string    &mode_name,
    const std::string    &mode_value,
    const std::string    &param_name,
    const std::vector<T> &param_value,
    size_t                correct_size,
    const std::string    &default_value_message,
    bool                  is_correct_mode,
    F1                    on_success,
    F2                    on_failure )
{
    if ( is_correct_mode )
    {
        if ( param_value.size() == correct_size )
        {
            on_success();
            return true;
        }
        else
        {
            if ( ( param_value.size() == 0 ) ||
                 ( ( param_value.size() == 1 ) && ( param_value[0] == 0 ) ) )
            {
                std::cerr << "Warning: " << mode_name << " was set to \""
                          << mode_value << "\", but no \"--" << param_name
                          << "\" parameter provided. " << default_value_message
                          << std::endl;

                on_failure();
                return false;
            }

            if ( param_value.size() != correct_size )
            {
                std::cerr << "Warning: The parameter \"" << param_name
                          << "\" must have " << correct_size << " values. "
                          << default_value_message << std::endl;

                on_failure();
                return false;
            }
        }
    }
    else
    {
        if ( ( param_value.size() > 1 ) ||
             ( ( param_value.size() == 1 ) && ( param_value[0] != 0 ) ) )
        {
            std::cerr << "Warning: the \"--" << param_name
                      << "\" parameter provided, but the " << mode_name
                      << " is different from \"" << mode_value << "\". "
                      << default_value_message << std::endl;

            on_failure();
            return false;
        }
        else
        {
            return true;
        }
    }
}

bool ImageConverter::parse( int argc, const char *argv[] )
{
    if ( _argParse.parse_args( argc, argv ) )
    {
        // report error?
        return false;
    }

    if ( _argParse["list-cameras"].get<int>() )
    {
        std::cout
            << "Spectral sensitivity data are available for the following cameras:"
            << std::endl;

        Idt                      idt;
        auto                     paths = collectDataFiles( "camera" );
        std::vector<std::string> cameras;
        for ( auto path: paths )
        {
            Spst spst;
            if ( spst.loadSpst( path, nullptr, nullptr ) )
            {
                cameras.push_back(
                    std::string( spst.getBrand() ) + " " + spst.getModel() );
            }
        }

        std::sort( cameras.begin(), cameras.end() );

        for ( auto s: cameras )
            std::cout << "- " << s << std::endl;
        std::cout << std::endl;
        exit( 0 );
    }

    if ( _argParse["list-illuminants"].get<int>() )
    {
        std::cout << "The following illuminants are supported:" << std::endl;
        std::cout << "- The standard illuminant series D (e.g., D60, D6025)"
                  << std::endl;
        std::cout << "- Black-body radiation (e.g., 3200K)" << std::endl;

        Idt                      idt;
        auto                     paths = collectDataFiles( "illuminant" );
        std::vector<std::string> illuminants;
        for ( auto path: paths )
        {
            Illum illum;
            if ( illum.readSPD( path, "na" ) )
            {
                illuminants.push_back( illum.getIllumType() );
            }
        }

        std::sort( illuminants.begin(), illuminants.end() );

        for ( auto s: illuminants )
            std::cout << "- " << s << std::endl;
        std::cout << std::endl;
        exit( 0 );
    }

    verbosity = _argParse["verbose"].get<int>();

    std::string wb_method = _argParse["wb-method"].get();

    if ( wb_method == "metadata" )
    {
        wbMethod = WBMethod::Metadata;
    }
    else if ( wb_method == "illuminant" )
    {
        wbMethod = WBMethod::Illuminant;
    }
    else if ( wb_method == "box" )
    {
        wbMethod = WBMethod::Box;
    }
    else if ( wb_method == "custom" )
    {
        wbMethod = WBMethod::Custom;
    }
    else
    {
        std::cerr << std::endl
                  << "Unsupported white balancing method: \"" << wb_method
                  << "\"." << std::endl;

        return false;
    }

    std::string mat_method = _argParse["mat-method"].get();

    if ( mat_method == "spectral" )
    {
        matrixMethod = MatrixMethod::Spectral;
    }
    else if ( mat_method == "metadata" )
    {
        matrixMethod = MatrixMethod::Metadata;
    }
    else if ( mat_method == "Adobe" )
    {
        matrixMethod = MatrixMethod::Adobe;
    }
    else if ( mat_method == "custom" )
    {
        matrixMethod = MatrixMethod::Custom;
    }
    else
    {
        std::cerr << std::endl
                  << "Unsupported matrix method: \"" << mat_method << "\"."
                  << std::endl;

        return false;
    }

    headroom = _argParse["headroom"].get<float>();

    std::string illum = _argParse["illuminant"].get();

    if ( wbMethod == WBMethod::Illuminant )
    {
        if ( illum.size() == 0 )
        {
            std::cerr << "Warning: the white balancing method was set to "
                      << "\"illuminant\", but no \"--illuminant\" parameter "
                      << "provided. " << illuminant << " will be used."
                      << std::endl;
        }
    }
    else
    {
        if ( illum.size() != 0 )
        {
            std::cerr << "Warning: the \"--illuminant\" parameter provided "
                      << "but the white balancing mode different from "
                      << "\"illuminant\" "
                      << "requested. The custom illuminant will be ignored."
                      << std::endl;
        }
    }

    auto box = _argParse["wb-box"].as_vec<int>();
    check_param(
        "white balancing mode",
        "box",
        "wb-box",
        box,
        4,
        "The box will be ignored.",
        wbMethod == WBMethod::Box,
        [&]() {
            for ( int i = 0; i < 4; i++ )
                wbBox[i] = box[i];
        },
        [&]() {
            for ( int i = 0; i < 4; i++ )
                wbBox[i] = 0;
        } );

    auto custom_wb = _argParse["custom-wb"].as_vec<float>();
    check_param(
        "white balancing mode",
        "custom",
        "custom-wb",
        custom_wb,
        4,
        "The scalers will be ignored. The default values of (1, 1, 1, 1) will be used",
        wbMethod == WBMethod::Custom,
        [&]() {
            for ( int i = 0; i < 4; i++ )
                customWB[i] = custom_wb[i];
        },
        [&]() {
            for ( int i = 0; i < 4; i++ )
                customWB[i] = 1.0;
        } );

    auto custom_mat = _argParse["custom-mat"].as_vec<float>();
    check_param(
        "matrix mode",
        "custom",
        "custom-mat",
        custom_mat,
        9,
        "Identity matrix will be used",
        matrixMethod == MatrixMethod::Custom,
        [&]() {
            for ( int i = 0; i < 3; i++ )
                for ( int j = 0; j < 3; j++ )
                    customMatrix[i][j] = custom_mat[i * 3 + j];
        },
        [&]() {
            for ( int i = 0; i < 3; i++ )
                for ( int j = 0; j < 3; j++ )
                    customMatrix[i][j] = i == j ? 1.0 : 0.0;
        } );

    auto crop = _argParse["cropbox"].as_vec<int>();
    if ( crop.size() == 4 )
    {
        for ( size_t i = 0; i < 4; i++ )
            cropbox[i] = crop[i];
    }

    no_auto_bright           = _argParse["no-auto-bright"].get<int>();
    adjust_maximum_threshold = _argParse["adjust-maximum-threshold"].get<int>();
    black_level              = _argParse["black-level"].get<int>();
    saturation_level         = _argParse["saturation-level"].get<int>();
    half_size                = _argParse["half-size"].get<int>();
    highlight_mode           = _argParse["highlight-mode"].get<int>();
    flip                     = _argParse["flip"].get<int>();

    return true;
}

bool ImageConverter::configure( const std::string &input_filename )
{
    _configFilename = input_filename;

    _inputHint = OIIO::ImageSpec();

    _inputHint["raw:ColorSpace"]    = "XYZ";
    _inputHint["raw:use_camera_wb"] = 0;
    _inputHint["raw:use_auto_wb"]   = 0;

    _inputHint["raw:auto_bright"]        = no_auto_bright ? 0 : 1;
    _inputHint["raw:adjust_maximum_thr"] = adjust_maximum_threshold;
    _inputHint["raw:user_black"]         = black_level;
    _inputHint["raw:user_sat"]           = saturation_level;
    _inputHint["raw:half_size"]          = (int)half_size;
    _inputHint["raw:flip"]               = flip;
    _inputHint["raw:HighlightMode"]      = highlight_mode;

    if ( cropbox[2] != 0 && cropbox[3] != 0 )
    {
        _inputHint.attribute(
            "raw:cropbox", OIIO::TypeDesc( OIIO::TypeDesc::INT, 4 ), cropbox );
    }

    auto imageInput = OIIO::ImageInput::create( "raw", false, &_inputHint );
    bool result = imageInput->open( input_filename, _inputFull, _inputHint );
    if ( !result )
    {
        return false;
    }

    _is_DNG = _inputFull.extra_attribs.find( "raw:dng:version" )->get_int() > 0;

    switch ( wbMethod )
    {
        case WBMethod::Metadata: {
            float user_mul[4];

            for ( int i = 0; i < 4; i++ )
            {
                user_mul[i] = _inputFull.find_attribute( "raw:cam_mul" )
                                  ->get_float_indexed( i ) /
                              256.0;
            }

            _inputHint.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                user_mul );
            break;
        }

        case WBMethod::Illuminant: {
            std::string lower_illuminant( illuminant );
            std::transform(
                lower_illuminant.begin(),
                lower_illuminant.end(),
                lower_illuminant.begin(),
                []( unsigned char c ) { return std::tolower( c ); } );

            if ( !isValidCT( lower_illuminant ) )
            {
                std::cerr << "Unrecognised illuminant \'" << illuminant << "\'"
                          << std::endl;
                return false;
            }

            break;
        }
        case WBMethod::Box:

            if ( wbBox[2] == 0 || wbBox[3] == 0 )
            {
                // Empty box, use whole image.
                _inputHint["raw:use_auto_wb"] = 1;
            }
            else
            {
                int32_t box[4];
                for ( int i = 0; i < 4; i++ )
                {
                    box[i] = wbBox[i];
                }
                _inputHint.attribute(
                    "raw:greybox",
                    OIIO::TypeDesc( OIIO::TypeDesc::INT, 4 ),
                    box );
            }
            break;

        case WBMethod::Custom:
            _inputHint.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                customWB );
            break;

        default: break;
    }

    switch ( matrixMethod )
    {
        case MatrixMethod::Spectral:
            _inputHint["raw:ColorSpace"]        = "raw";
            _inputHint["raw:use_camera_matrix"] = 0;
            break;
        case MatrixMethod::Metadata:
            _inputHint["raw:use_camera_matrix"] = _is_DNG ? 1 : 3;
            break;
        case MatrixMethod::Adobe:
            _inputHint["raw:use_camera_matrix"] = 1;
            break;
        case MatrixMethod::Custom:
            _inputHint["raw:use_camera_matrix"] = 0;
            _inputHint["raw:ColorSpace"]        = "raw";

            _IDT_matrix.resize( 3 );
            for ( int i = 0; i < 3; i++ )
            {
                _IDT_matrix[i].resize( 3 );
                for ( int j = 0; j < 3; j++ )
                {
                    _IDT_matrix[i][j] = customMatrix[i][j];
                }
            }

            break;
        default: break;
    }

    bool spectral_white_balance = wbMethod == WBMethod::Illuminant;
    bool spectral_matrix        = matrixMethod == MatrixMethod::Spectral;

    if ( spectral_white_balance || spectral_matrix )
    {
        float pre_mul[4];

        for ( int i = 0; i < 4; i++ )
        {
            pre_mul[i] = _inputFull.find_attribute( "raw:pre_mul" )
                             ->get_float_indexed( i );
        }

        _cameraMake  = _inputFull["Make"];
        _cameraModel = _inputFull["Model"];

        prepareIDT_spectral( spectral_white_balance, spectral_matrix );

        if ( spectral_white_balance )
        {
            float user_mul[4];

            for ( int i = 0; i < _WB_mults.size(); i++ )
            {
                user_mul[i] = _WB_mults[i];
            }
            if ( _WB_mults.size() == 3 )
                user_mul[3] = _WB_mults[1];

            _inputHint.attribute(
                "raw:user_mul",
                OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 4 ),
                user_mul );
        }
    }

    if ( matrixMethod == MatrixMethod::Metadata ||
         matrixMethod == MatrixMethod::Adobe )
    {
        if ( _is_DNG )
        {
            _inputHint["raw:use_camera_matrix"] = 1;
            _inputHint["raw:use_camera_wb"]     = 1;

            prepareIDT_DNG();
        }
        else
        {
            prepareIDT_nonDNG();
        }
    }

    return true;
}

bool ImageConverter::load( const std::string &input_filename )
{
    if ( _configFilename != input_filename )
    {
        auto imageInput = OIIO::ImageInput::create( "raw", false, &_inputHint );
        imageInput->open( input_filename, _inputFull, _inputHint );
    }

    _imageBuffer =
        OIIO::ImageBuf( input_filename, 0, 0, nullptr, &_inputHint, nullptr );
    bool result = _imageBuffer.init_spec( input_filename, 0, 0 );
    if ( result )
    {
        auto &spec     = _imageBuffer.spec();
        auto  channels = spec.nchannels;
        result =
            _imageBuffer.read( 0, 0, 0, channels, true, OIIO::TypeDesc::FLOAT );
    }
    return result;
}

void ImageConverter::applyMatrix(
    const std::vector<std::vector<double>> &matrix )
{
    float M[4][4];

    size_t n = matrix.size();

    if ( n )
    {
        size_t m = matrix[0].size();

        for ( size_t i = 0; i < n; i++ )
        {
            for ( size_t j = 0; j < m; j++ )
            {
                M[j][i] = matrix[i][j];
            }

            for ( size_t j = m; j < 4; j++ )
                M[j][i] = 0;
        }

        for ( size_t i = n; i < 4; i++ )
        {
            for ( size_t j = 0; j < m; j++ )
                M[j][i] = 0;
            for ( size_t j = m; j < 4; j++ )
                M[j][i] = 1;
        }
    }

    _imageBuffer = OIIO::ImageBufAlgo::colormatrixtransform( _imageBuffer, M );
}

bool ImageConverter::process()
{
    if ( _IDT_matrix.size() )
    {
        applyMatrix( _IDT_matrix );
    }

    if ( _CAT_matrix.size() )
    {
        applyMatrix( _CAT_matrix );

        _imageBuffer = OIIO::ImageBufAlgo::colormatrixtransform(
            _imageBuffer, XYZ_acesrgb_transposed_4 );
    }

    _imageBuffer = OIIO::ImageBufAlgo::mul( _imageBuffer, headroom );
    return true;
}

bool ImageConverter::save( const std::string &output_filename )
{
    const float chromaticities[] = { 0.7347, 0.2653, 0,       1,
                                     0.0001, -0.077, 0.32168, 0.33767 };

    OIIO::ImageSpec imageSpec = _inputFull;
    imageSpec.set_format( OIIO::TypeDesc::HALF );
    imageSpec["acesImageContainerFlag"] = 1;
    imageSpec["compression"]            = "none";
    imageSpec.attribute(
        "chromaticities",
        OIIO::TypeDesc( OIIO::TypeDesc::FLOAT, 8 ),
        chromaticities );

    auto imageOutput = OIIO::ImageOutput::create( "exr" );
    bool result      = imageOutput->open( output_filename, imageSpec );
    result           = _imageBuffer.write( imageOutput.get() );
    return result;
}

void ImageConverter::prepareIDT_spectral(
    bool calc_white_balance, bool calc_matrix )
{
    std::string lower_illuminant( illuminant );

    if ( lower_illuminant.length() == 0 )
        lower_illuminant = "na";
    else
    {
        std::transform(
            lower_illuminant.begin(),
            lower_illuminant.end(),
            lower_illuminant.begin(),
            []( unsigned char c ) { return std::tolower( c ); } );
    }

    Idt idt;

    auto illum_paths = collectDataFiles( "illuminant" );
    int  res         = idt.loadIlluminant( illum_paths, lower_illuminant );

    bool found_camera = false;
    auto camera_paths = collectDataFiles( "camera" );

    for ( auto &path: camera_paths )
    {
        if ( idt.loadCameraSpst(
                 path, _cameraMake.c_str(), _cameraModel.c_str() ) )
        {
            found_camera = true;
            break;
        }
    }

    auto training = findFile( "training/training_spectral.json" );
    if ( training.length() )
    {
        idt.loadTrainingData( training );
    }

    auto cmf = findFile( "cmf/cmf_1931.json" );
    if ( cmf.length() )
    {
        idt.loadCMF( cmf );
    }

    if ( lower_illuminant != "na" )
    {
        idt.chooseIllumType( lower_illuminant.c_str(), headroom );
    }
    else
    {
        std::vector<double> cam_mul( 3 );
        for ( int i = 0; i < 3; i++ )
            cam_mul[i] = _inputFull.find_attribute( "raw:cam_mul" )
                             ->get_float_indexed( i );
        idt.chooseIllumSrc( cam_mul, headroom );
    }

    if ( idt.calIDT() )
    {
        if ( calc_white_balance )
        {
            _WB_mults = idt.getWB();
        }

        if ( calc_matrix )
        {
            _IDT_matrix = idt.getIDT();
            _CAT_matrix.resize( 0 );
        }
    }
}

void ImageConverter::prepareIDT_DNG()
{
    Metadata metadata;
    metadata.neutralRGB.resize( 3 );
    metadata.xyz2rgbMatrix1.resize( 9 );
    metadata.xyz2rgbMatrix2.resize( 9 );
    metadata.cameraCalibration1.resize( 9 );
    metadata.cameraCalibration2.resize( 9 );

    metadata.baselineExposure =
        _inputFull.get_float_attribute( "raw:dng:baseline_exposure" );
    metadata.calibrationIlluminant1 =
        _inputFull.get_int_attribute( "raw:dng:calibration_illuminant1" );
    metadata.calibrationIlluminant2 =
        _inputFull.get_int_attribute( "raw:dng:calibration_illuminant2" );

    for ( int i = 0; i < 3; i++ )
    {
        metadata.neutralRGB[i] =
            1.0 /
            _inputFull.find_attribute( "raw:cam_mul" )->get_float_indexed( i );

        for ( int j = 0; j < 3; j++ )
        {
            metadata.xyz2rgbMatrix1[i * 3 + j] =
                _inputFull.find_attribute( "raw:dng:color_matrix1" )
                    ->get_float_indexed( i * 3 + j );
            metadata.xyz2rgbMatrix2[i * 3 + j] =
                _inputFull.find_attribute( "raw:dng:color_matrix2" )
                    ->get_float_indexed( i * 3 + j );
            metadata.cameraCalibration1[i * 3 + j] =
                _inputFull.find_attribute( "raw:dng:camera_calibration1" )
                    ->get_float_indexed( i * 4 + j );
            metadata.cameraCalibration2[i * 3 + j] =
                _inputFull.find_attribute( "raw:dng:camera_calibration2" )
                    ->get_float_indexed( i * 4 + j );
        }
    }

    DNGIdt *dng = new DNGIdt( metadata );

    _IDT_matrix = dng->getDNGIDTMatrix();

    // Do not apply CAT for DNG
    _CAT_matrix.resize( 0 );
    //    _CAT_matrix = dng->getDNGCATMatrix();
}

void ImageConverter::prepareIDT_nonDNG()
{
    _IDT_matrix.resize( 0 );

    vector<double> dIV( d65, d65 + 3 );
    vector<double> dOV( d60, d60 + 3 );
    _CAT_matrix = getCAT( dIV, dOV );
}

} // namespace rta
