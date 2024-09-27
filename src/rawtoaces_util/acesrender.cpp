///////////////////////////////////////////////////////////////////////////
// Copyright (c) 2013 Academy of Motion Picture Arts and Sciences
// ("A.M.P.A.S."). Portions contributed by others as indicated.
// All rights reserved.
//
// A worldwide, royalty-free, non-exclusive right to copy, modify, create
// derivatives, and use, in source and binary forms, is hereby granted,
// subject to acceptance of this license. Performance of any of the
// aforementioned acts indicates acceptance to be bound by the following
// terms and conditions:
//
//  * Copies of source code, in whole or in part, must retain the
//    above copyright notice, this list of conditions and the
//    Disclaimer of Warranty.
//
//  * Use in binary form must retain the above copyright notice,
//    this list of conditions and the Disclaimer of Warranty in the
//    documentation and/or other materials provided with the distribution.
//
//  * Nothing in this license shall be deemed to grant any rights to
//    trademarks, copyrights, patents, trade secrets or any other
//    intellectual property of A.M.P.A.S. or any contributors, except
//    as expressly stated herein.
//
//  * Neither the name "A.M.P.A.S." nor the name of any other
//    contributors to this software may be used to endorse or promote
//    products derivative of or based on this software without express
//    prior written permission of A.M.P.A.S. or the contributors, as
//    appropriate.
//
// This license shall be construed pursuant to the laws of the State of
// California, and any disputes related thereto shall be subject to the
// jurisdiction of the courts therein.
//
// Disclaimer of Warranty: THIS SOFTWARE IS PROVIDED BY A.M.P.A.S. AND
// CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
// BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE, AND NON-INFRINGEMENT ARE DISCLAIMED. IN NO
// EVENT SHALL A.M.P.A.S., OR ANY CONTRIBUTORS OR DISTRIBUTORS, BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, RESITUTIONARY,
// OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
// THE POSSIBILITY OF SUCH DAMAGE.
//
// WITHOUT LIMITING THE GENERALITY OF THE FOREGOING, THE ACADEMY
// SPECIFICALLY DISCLAIMS ANY REPRESENTATIONS OR WARRANTIES WHATSOEVER
// RELATED TO PATENT OR OTHER INTELLECTUAL PROPERTY RIGHTS IN THE ACADEMY
// COLOR ENCODING SYSTEM, OR APPLICATIONS THEREOF, HELD BY PARTIES OTHER
// THAN A.M.P.A.S., WHETHER DISCLOSED OR UNDISCLOSED.
///////////////////////////////////////////////////////////////////////////

#include <rawtoaces/acesrender.h>
#include <rawtoaces/mathOps.h>

#include <Imath/half.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <aces/aces_Writer.h>

#ifndef WIN32
#    include <fcntl.h>
#    include <sys/mman.h>
#endif

using namespace std;
using namespace boost::property_tree;

#include <boost/filesystem.hpp>

//  =====================================================================
//  Prepare the matching between string flags and single character flag
//
//  inputs:
//      N/A
//
//  outputs:
//      N/A : keys should be prepared and loaded

void create_key( unordered_map<string, char> &keys )
{
    keys["--help"]          = 'I';
    keys["--version"]       = 'V';
    keys["--cameras"]       = 'T';
    keys["--wb-method"]     = 'R';
    keys["--mat-method"]    = 'p';
    keys["--headroom"]      = 'M';
    keys["--valid-illums"]  = 'z';
    keys["--valid-cameras"] = 'Q';
    keys["-c"]              = 'c';
    keys["-C"]              = 'C';
    keys["-P"]              = 'P';
    keys["-K"]              = 'K';
    keys["-k"]              = 'k';
    keys["-S"]              = 'S';
    keys["-n"]              = 'n';
    keys["-H"]              = 'H';
    keys["-t"]              = 't';
    keys["-j"]              = 'j';
    keys["-W"]              = 'W';
    keys["-b"]              = 'b';
    keys["-q"]              = 'q';
    keys["-h"]              = 'h';
    keys["-f"]              = 'f';
    keys["-m"]              = 'm';
    keys["-s"]              = 's';
    keys["-G"]              = 'G';
    keys["-B"]              = 'B';
    keys["-v"]              = 'v';
    keys["-F"]              = 'F';
    keys["-d"]              = 'd';
    keys["-E"]              = 'E';
    keys["-I"]              = 'I';
    keys["-V"]              = 'V';
};

//  =====================================================================
//  Print usage / help message
//
//  inputs:
//      const char * : name of the program (i.e., rawtoaces)
//
//  outputs:
//      N/A

void usage( const char *prog )
{
    printf( "%s - convert RAW digital camera files to ACES\n", prog );
    printf( "\n" );
    printf( "Usage:\n" );
    printf( "  %s file ...\n", prog );
    printf( "  %s [options] file\n", prog );
    printf( "  %s --help\n", prog );
    printf( "  %s --version\n", prog );
    printf( "\n" );
    printf(
        "IDT options:\n"
        "  --help                  Show this screen\n"
        "  --version               Show version\n"
        "  --wb-method [0-4]       White balance factor calculation method\n"
        "                            0=white balance using file metadata \n"
        "                            1=white balance using user specified illuminant [str] \n"
        "                            2=Average the whole image for white balance\n"
        "                            3=Average a grey box for white balance <x y w h>\n"
        "                            4=Use custom white balance  <r g b g>\n"
        "                            (default = 0)\n"
        "  --mat-method [0-2]      IDT matrix calculation method\n"
        "                            0=Calculate matrix from camera spec sens\n"
        "                            1=Use file metadata color matrix\n"
        "                            2=Use adobe coeffs included in libraw\n"
        "                            3=Use custom matrix <m1r m1g m1b m2r m2g m2b m3r m3g m3b>\n"
        "                            (default = 0)\n"
        "                            (default = /usr/local/include/rawtoaces/data/camera)\n"
        "  --headroom float        Set highlight headroom factor (default = 6.0)\n"
        "  --cameras               Show a list of supported cameras/models by LibRaw\n"
        "  --valid-illums          Show a list of illuminants\n"
        "  --valid-cameras         Show a list of cameras/models with available\n"
        "                          spectral sensitivity datasets\n"
        "\n"
        "Raw conversion options:\n"
        "  -c float                Set adjust maximum threshold (default = 0.75)\n"
        "  -C <r b>                Correct chromatic aberration\n"
        "  -P <file>               Fix the dead pixels listed in this file\n"
        "  -K <file>               Subtract dark frame (16-bit raw PGM)\n"
        "  -k <num>                Set the darkness level\n"
        "  -S <num>                Set the saturation level\n"
        "  -n <num>                Set threshold for wavelet denoising\n"
        "  -H [0-9]                Highlight mode (0=clip, 1=unclip, 2=blend, 3+=rebuild) (default = 0)\n"
        "  -t [0-7]                Flip image (0=none, 3=180, 5=90CCW, 6=90CW)\n"
        "  -j                      Don't stretch or rotate raw pixels\n"
        "  -W                      Don't automatically brighten the image\n"
        "  -b <num>                Adjust brightness (default = 1.0)\n"
        "  -q [0-3]                Set the interpolation quality\n"
        "  -h                      Half-size color image (twice as fast as \"-q 0\")\n"
        "  -f                      Interpolate RGGB as four colors\n"
        "  -m <num>                Apply a 3x3 median filter to R-G and B-G\n"
        "  -s [0..N-1]             Select one raw image from input file\n"
        "  -G                      Use green_matching() filter\n"
        "  -B <x y w h>            Use cropbox\n"
        "\n"
        "Benchmarking options:\n"
        "  -v                      Verbose: print progress messages (repeated -v will add verbosity)\n"
        "  -F                      Use FILE I/O instead of streambuf API\n"
        "  -d                      Detailed timing report\n"
#ifndef WIN32
        "  -E                      Use mmap()-ed buffer instead of plain FILE I/O\n"
#endif
    );
    exit( -1 );
};

//  =====================================================================
//	Defaul Constructor

AcesRender::AcesRender()
{
    _idt          = new Idt();
    _image        = new libraw_processed_image_t();
    _rawProcessor = new LibRawAces();

    _idtm.resize( 3 );
    _wbv.resize( 3 );
    _catm.resize( 3 );

    FORI( 3 )
    {
        _idtm[i].resize( 3 );
        _catm[i].resize( 3 );

        _wbv[i]               = 1.0;
        FORJ( 3 ) _idtm[i][j] = neutral3[i][j];
        FORJ( 3 ) _catm[i][j] = neutral3[i][j];
    }
}

//  =====================================================================
//	Defaul Destructor

AcesRender::~AcesRender()
{
    if ( _pathToRaw )
    {
        delete _pathToRaw;
        _pathToRaw = nullptr;
    }

    if ( _idt )
    {
        delete _idt;
        _idt = nullptr;
    }

    if ( _image )
    {
        delete _image;
        _image = nullptr;
    }

    if ( _rawProcessor )
    {
        delete _rawProcessor;
        _rawProcessor = nullptr;
    }

    vector<vector<double>>().swap( _idtm );
    vector<vector<double>>().swap( _catm );
    vector<double>().swap( _wbv );
    vector<string>().swap( _illuminants );
    vector<string>().swap( _cameras );
}

//	=====================================================================
//	Get the only instance of the "AcesRender" class
//
//	inputs:
//      N/A
//
//	outputs:
//      static AcesRender & : the referece to the only instance by calling
//      the private getPrivateInstance() function

AcesRender &AcesRender::getInstance()
{

    return getPrivateInstance();
}

//	=====================================================================
//	Initialize the single instance of the "AcesRender" class privately
//
//	inputs:
//      N/A
//
//	outputs:
//      static AcesRender &  : the referece to the only instance
//      of "AcesRender" class

AcesRender &AcesRender::getPrivateInstance()
{

    mutex mtx;
    mtx.lock();
    static AcesRender acesrender;
    mtx.unlock();

    return acesrender;
}

//	=====================================================================
//	Operator = overloading in "AcesRender" class
//
//	inputs:
//      const AcesRender & : acesrender
//
//	outputs:
//      const AcesRender & : current instance filled with data from
//      acesrender

const AcesRender &AcesRender::operator=( const AcesRender &acesrender )
{
    if ( this != &acesrender )
    {
        clearVM( _idtm );
        clearVM( _catm );
        clearVM( _wbv );
        clearVM( _illuminants );
        clearVM( _cameras );

        if ( _idt != nullptr )
            delete _idt;
        _idt = (Idt *)malloc( sizeof( Idt ) );
        memcpy( _idt, acesrender._idt, sizeof( Idt ) );

        if ( _rawProcessor != nullptr )
            delete _rawProcessor;
        _rawProcessor = (LibRawAces *)malloc( sizeof( LibRawAces ) );
        memcpy(
            (void *)_rawProcessor,
            (void *)acesrender._rawProcessor,
            sizeof( LibRawAces ) );

        if ( _image != nullptr )
            delete _image;
        _image = (libraw_processed_image_t *)malloc(
            sizeof( libraw_processed_image_t ) );
        memcpy( _image, acesrender._image, sizeof( libraw_processed_image_t ) );

        _idtm        = acesrender._idtm;
        _catm        = acesrender._catm;
        _wbv         = acesrender._wbv;
        _illuminants = acesrender._illuminants;
        _cameras     = acesrender._cameras;
        _opts        = acesrender._opts;
    }

    return *this;
}

//	=====================================================================
//	Initialize the process by first setting up default values for "_opts"
//  and some flags for "_rawProcessor"
//
//	inputs:
//      struct dataPath : data path of the processed environment variable
//
//	outputs:
//      N/A : _opts will be set up by the default values;
//            A few parameters of "_rawProcessor" (imgdata.params)
//            will be given a set of initial values

void AcesRender::initialize( const dataPath &dp )
{
    _opts.use_bigfile        = 0;
    _opts.use_timing         = 0;
    _opts.use_illum          = 0;
    _opts.use_mul            = 0;
    _opts.verbosity          = 0;
    _opts.mat_method         = matMethod0;
    _opts.wb_method          = wbMethod0;
    _opts.highlight          = 0;
    _opts.scale              = 6.0;
    _opts.highlight          = 0;
    _opts.get_illums         = 0;
    _opts.get_cameras        = 0;
    _opts.get_libraw_cameras = 0;

#ifndef WIN32
    _opts.iobuffer = 0;
#endif

    struct stat st;
    FORI( dp.paths.size() )
    {
        if ( !stat( ( dp.paths )[i].c_str(), &st ) )
            _opts.envPaths.push_back( ( dp.paths )[i] );
    }

#ifndef WIN32
    _opts.msize    = 0;
    _opts.use_mmap = 0;
#endif

#ifdef OUT
#    undef OUT
#endif

#define OUT _rawProcessor->imgdata.params

    //  General set-up for _rawProcessor->imgdata.params
    OUT.output_color = 5;
    OUT.output_bps   = 16;
    OUT.highlight    = 0;
    //    OUT.use_camera_matrix = 0;
    OUT.gamm[0]        = 1.0;
    OUT.gamm[1]        = 1.0;
    OUT.no_auto_bright = 1;
    OUT.use_camera_wb  = 0;
    OUT.use_auto_wb    = 0;
}

//	=====================================================================
//	Configure settings by taking in user specified options
//
//	inputs:
//      int argc        : number of user input
//      char * argv[]   : an array of user input
//
//	outputs:
//      N/A : _opts will be ready by digesting the user input;
//            _rawProcessor (imgdata.params) will take initial
//            set of values from user inputs

int AcesRender::configureSettings( int argc, char *argv[] )
{
#ifdef OUT
#    undef OUT
#endif

#define OUT _rawProcessor->imgdata.params

    char *cp, *sp;
    int   arg;
    argv[argc] = (char *)"";

    for ( arg = 1; arg < argc; )
    {
        string key( argv[arg] );

        if ( key[0] != '-' )
        {
            break;
        }

        arg++;

        static unordered_map<string, char> keys;
        create_key( keys );

        char opt = keys[key];

        if ( !opt )
        {
            fprintf(
                stderr, "\nNon-recognizable flag - \"%s\"\n", key.c_str() );
            exit( -1 );
        }

        if ( ( cp = strchr( sp = (char *)"HcnbksStqmBC", opt ) ) != 0 )
        {
            for ( int i = 0; i < "111111111142"[cp - sp] - '0'; i++ )
            {
                if ( !isdigit( argv[arg + i][0] ) )
                {
                    fprintf(
                        stderr,
                        "\nError: Non-numeric argument to "
                        "\"%s\"\n",
                        key.c_str() );
                    exit( -1 );
                }
            }
        }

        switch ( opt )
        {
            case 'I': usage( argv[0] ); break;
            case 'V': printf( "%s\n", VERSION ); break;
            case 'v': _opts.verbosity++; break;
            case 'G': OUT.green_matching = 1; break;
            case 'c':
                OUT.adjust_maximum_thr = (float)atof( argv[arg++] );
                break;
            case 'n': OUT.threshold = (float)atof( argv[arg++] ); break;
            case 'b': OUT.bright = (float)atof( argv[arg++] ); break;
            case 'P': OUT.bad_pixels = argv[arg++]; break;
            case 'K': OUT.dark_frame = argv[arg++]; break;
            case 'C': {
                OUT.aber[0] = 1.0 / atof( argv[arg++] );
                OUT.aber[2] = 1.0 / atof( argv[arg++] );
                break;
            }
            case 'k': OUT.user_black = atoi( argv[arg++] ); break;
            case 'S': OUT.user_sat = atoi( argv[arg++] ); break;
            case 't': OUT.user_flip = atoi( argv[arg++] ); break;
            case 'q': OUT.user_qual = atoi( argv[arg++] ); break;
            case 'm': OUT.med_passes = atoi( argv[arg++] ); break;
            case 'h':
                OUT.half_size = 1;
                // no break:  "-h" implies "-f"
            case 'f': OUT.four_color_rgb = 1; break;
            case 'B': FORI( 4 ) OUT.cropbox[i] = atoi( argv[arg++] ); break;
            case 'j': OUT.use_fuji_rotate = 0; break;
            case 'W': OUT.no_auto_bright = 1; break;
            case 'F': _opts.use_bigfile = 1; break;
            case 'd': _opts.use_timing = 1; break;
            case 'Q':
                _opts.get_cameras = 1;
                {
                    // gather a list of cameras supported
                    gatherSupportedCameras();
                    vector<string> clist = getSupportedCameras();
                    printf(
                        "\nThe following cameras' sensitivity data is available:\n\n" );
                    FORI( clist.size() ) printf( "%s \n", clist[i].c_str() );

                    break;
                }
            case 'z':
                _opts.get_illums = 1;
                {
                    // gather a list of illuminants supported
                    gatherSupportedIllums();
                    vector<string> ilist = getSupportedIllums();
                    printf( "\nThe following illuminants are available:\n\n" );
                    FORI( ilist.size() ) printf( "%s \n", ilist[i].c_str() );

                    break;
                }
            case 'T':
                _opts.get_libraw_cameras = 1;
                {
                    // print a list of cameras supported by LibRaw
                    printLibRawCameras();
                    break;
                }
            case 'M': _opts.scale = atof( argv[arg++] ); break;
            case 'H': {
                OUT.highlight   = atoi( argv[arg++] );
                _opts.highlight = OUT.highlight;
                break;
            }
            case 'p': {
                _opts.mat_method = matMethods_t( atoi( argv[arg++] ) );
                if ( _opts.mat_method > 3 || _opts.mat_method < 0 )
                {
                    fprintf(
                        stderr,
                        "\nError: Invalid argument to "
                        "\"%s\" \n",
                        key.c_str() );
                    exit( -1 );
                }

                if ( _opts.mat_method == matMethod3 )
                {

                    bool flag = false;
                    FORI( 9 )
                    {
                        if ( isalpha( argv[arg][0] ) )
                        {
                            fprintf(
                                stderr,
                                "\nError: Non-numeric argument to "
                                "\"%s %i\" \n",
                                key.c_str(),
                                _opts.mat_method );
                            exit( -1 );
                        }
                        custom_Buffer[i] =
                            static_cast<float>( atof( argv[arg++] ) );
                        if ( i == 8 )
                        {
                            flag = true;
                        }
                    }

                    // For testing purposes FORI(9){ cout<<custom_Buffer[i]<<endl;}
                    if ( flag )
                    {
                        FORI( 3 )
                        {
                            FORJ( 3 )
                            {
                                custom_Matrix[i][j] = custom_Buffer[i * 3 + j];
                                //cout<< custom_Matrix[i][j]<<endl;
                            }
                        }
                    }
                }
                break;
            }
            case 'R': {
                std::string flag = std::string( argv[arg] );
                FORI( flag.size() )
                {
                    if ( !isdigit( flag[i] ) )
                    {
                        fprintf(
                            stderr,
                            "\nNon-recognizable argument to "
                            "\"--wb-method\".\n" );
                        exit( -1 );
                    }
                }

                _opts.wb_method = wbMethods_t( atoi( argv[arg++] ) );

                // 1
                if ( _opts.wb_method == wbMethod1 )
                {
                    _opts.use_illum = 1;
                    _opts.illumType = static_cast<char *>( argv[arg++] );
                    lowerCase( _opts.illumType );

                    if ( !isValidCT( string( _opts.illumType ) ) )
                    {
                        fprintf(
                            stderr,
                            "\nError: white balance method 1 requires a valid "
                            "illuminant (e.g., D60, 3200K) to be specified\n" );
                        exit( -1 );
                    }
                }
                // 3
                if ( _opts.wb_method == wbMethod3 )
                {
                    FORI( 4 )
                    {
                        if ( !isdigit( argv[arg][0] ) )
                        {
                            fprintf(
                                stderr,
                                "\nError: Non-numeric argument to "
                                "\"%s %i\" \n",
                                key.c_str(),
                                _opts.wb_method );
                            exit( -1 );
                        }
                        OUT.greybox[i] =
                            static_cast<float>( atof( argv[arg++] ) );
                    }
                }
                // 4
                else if ( _opts.wb_method == wbMethod4 )
                {
                    _opts.use_mul = 1;
                    FORI( 4 )
                    {
                        if ( !isdigit( argv[arg][0] ) )
                        {
                            fprintf(
                                stderr,
                                "\nError: Non-numeric argument to "
                                "\"%s %i\" \n",
                                key.c_str(),
                                _opts.wb_method );
                            exit( -1 );
                        }
                        OUT.user_mul[i] =
                            static_cast<float>( atof( argv[arg++] ) );
                    }
                }
                else if ( _opts.wb_method > 4 || _opts.wb_method < 0 )
                {
                    fprintf(
                        stderr,
                        "\nError: Invalid argument to \"%s\" \n",
                        key.c_str() );
                    exit( -1 );
                }
                break;
            }
#ifndef WIN32
            case 'E': _opts.use_mmap = 1; break;
#endif
            default:
                fprintf(
                    stderr, "\nError: Unknown option \"%s\".\n", key.c_str() );
                exit( -1 );
        }
    }

    return arg;
}

//	=====================================================================
//	Set processed image buffer from libraw
//
//	inputs:
//      libraw_processed_image_t     : processed RAW through libraw
//
//	outputs:
//      N/A        : _image will point to the address of image

void AcesRender::setPixels( libraw_processed_image_t *image )
{
    assert( image );
    if ( _image != nullptr )
        delete _image;
    _image = image;

    ////    Strange because memcpying on libraw_processed_image_t
    //    will generate an error
    //    if (_image != nullptr ) delete _image;
    //    _image = (libraw_processed_image_t *) malloc(sizeof(libraw_processed_image_t));
    //    memcpy(_image, image, sizeof(libraw_processed_image_t));
}

//	=====================================================================
//	Gather supported Illuminants by reading from JSON files
//
//	inputs:
//      N/A
//
//	outputs:
//      N/A        : _illuminants be filled

void AcesRender::gatherSupportedIllums()
{

    if ( _illuminants.size() != 0 )
        _illuminants.clear();

    _illuminants.push_back( "Day-light (e.g., D60, D6025)" );
    _illuminants.push_back( "Blackbody (e.g., 3200K)" );

    std::unordered_map<string, int> record;

    FORI( _opts.envPaths.size() )
    {
        vector<string> iFiles = openDir(
            static_cast<string>( ( _opts.envPaths )[i] ) + "/illuminant" );
        for ( vector<string>::iterator file = iFiles.begin();
              file != iFiles.end();
              ++file )
        {
            string path( *file );
            try
            {
                ptree pt;
                read_json( path, pt );
                string tmp = pt.get<string>( "header.illuminant" );

                if ( record.find( tmp ) != record.end() )
                    continue;
                else
                {
                    _illuminants.push_back( tmp );
                    record[tmp] = 1;
                }
            }
            catch ( std::exception const &e )
            {
                std::cerr << e.what() << std::endl;
            }
        }
    }
}

//	=====================================================================
//	Gather supported cameras by reading from JSON files
//
//	inputs:
//      N/A
//
//	outputs:
//      N/A        : _cameras be filled

void AcesRender::gatherSupportedCameras()
{

    if ( _cameras.size() != 0 )
        _cameras.clear();

    std::unordered_map<string, int> record;

    FORI( _opts.envPaths.size() )
    {
        vector<string> iFiles =
            openDir( static_cast<string>( ( _opts.envPaths )[i] ) + "/camera" );
        for ( vector<string>::iterator file = iFiles.begin();
              file != iFiles.end();
              ++file )
        {
            string path( *file );
            try
            {
                ptree pt;
                read_json( path, pt );
                string tmp = pt.get<string>( "header.manufacturer" );
                tmp += ( " / " + pt.get<string>( "header.model" ) );

                if ( record.find( tmp ) != record.end() )
                    continue;
                else
                {
                    _cameras.push_back( tmp );
                    record[tmp] = 1;
                }
            }
            catch ( std::exception const &e )
            {
                std::cerr << e.what() << std::endl;
            }
        }
    }
}

//	=====================================================================
//  Open the RAW file from the path to the file
//
//	inputs:
//      const char *       : path to the raw file
//
//	outputs:
//		int                : "1" means raw file successfully opened;
//                           "0" means error when opening the file

int AcesRender::openRawPath( const char *pathToRaw )
{
    assert( pathToRaw != nullptr );

#ifndef WIN32
    //    void *iobuffer=0;
    struct stat st;

    if ( _opts.use_mmap )
    {
        int file = open( pathToRaw, O_RDONLY );

        if ( file < 0 )
        {
            fprintf(
                stderr,
                "\nError: Cannot open %s: %s\n\n",
                pathToRaw,
                strerror( errno ) );
            _opts.ret = 0;

            return 0;
        }

        if ( fstat( file, &st ) )
        {
            fprintf(
                stderr,
                "\nError: Cannot stat %s: %s\n\n",
                pathToRaw,
                strerror( errno ) );
            close( file );
            _opts.ret = 0;

            return 0;
        }

        int pgsz       = getpagesize();
        _opts.msize    = ( ( st.st_size + pgsz - 1 ) / pgsz ) * pgsz;
        _opts.iobuffer = mmap(
            NULL, size_t( _opts.msize ), PROT_READ, MAP_PRIVATE, file, 0 );
        if ( !_opts.iobuffer )
        {
            fprintf(
                stderr,
                "\nError: Cannot mmap %s: %s\n\n",
                pathToRaw,
                strerror( errno ) );
            close( file );
            _opts.ret = 0;

            return 0;
        }

        close( file );
        if ( ( _opts.ret =
                   _rawProcessor->open_buffer( _opts.iobuffer, st.st_size ) !=
                   LIBRAW_SUCCESS ) )
        {
            fprintf(
                stderr,
                "\nError: Cannot open_buffer %s: %s\n\n",
                pathToRaw,
                libraw_strerror( _opts.ret ) );
        }
    }
    else
#endif
    {
#if ( LIBRAW_VERSION < LIBRAW_MAKE_VERSION( 0, 20, 0 ) )
        if ( _opts.use_bigfile )
            _opts.ret = _rawProcessor->open_file( pathToRaw, 1 );
        else
            _opts.ret = _rawProcessor->open_file( pathToRaw );
#else
        _opts.ret = _rawProcessor->open_file( pathToRaw );
#endif
    }

    unpack( pathToRaw );

    return _opts.ret;
}

//	=====================================================================
//  Unpack the RAW file based on the path to the file (after openRawPath)
//
//	inputs:
//      const char *       : path to the raw file
//
//	outputs:
//		int                : "1" means raw file successfully unpacked;
//                           "0" means error when unpacking the file

int AcesRender::unpack( const char *pathToRaw )
{
    assert( _opts.ret == LIBRAW_SUCCESS && pathToRaw != nullptr );

    if ( ( _opts.ret = _rawProcessor->unpack() ) != LIBRAW_SUCCESS )
    {
        fprintf(
            stderr,
            "\nError: Cannot unpack %s: %s\n\n",
            pathToRaw,
            libraw_strerror( _opts.ret ) );
    }

    return _opts.ret;
}

//	=====================================================================
//	Read camera spectral sensitivity data from path
//
//	inputs:
//      const char *     : camera spectral sensitivity path
//                         (either in the environment variable of
//                          "AMPAS_CAMERA_SENSITIVITIES_PATH"
//                          such as  "/usr/local/include/rawtoaces/data/camera"
//                          or specified by users)
//      libraw_iparams_t : main parameters read from RAW
//
//	outputs:
//		int              : "1" means loading/injecting camera spectral
//                         sensitivity data successfully;
//                         "0" means error in reading/injecting data

int AcesRender::fetchCameraSenPath( const libraw_iparams_t &P )
{
    int readC = 0;

    FORI( _opts.envPaths.size() )
    {
        vector<string> cFiles =
            openDir( static_cast<string>( ( _opts.envPaths )[i] ) + "/camera" );
        for ( vector<string>::iterator file = cFiles.begin();
              file != cFiles.end();
              ++file )
        {
            string fn( *file );

            if ( fn.find( ".json" ) == std::string::npos )
                continue;
            readC = _idt->loadCameraSpst( fn, P.make, P.model );
            if ( readC )
                return 1;
        }
    }

    return readC;
}

//	=====================================================================
//	Fetch light source data to calcuate white balance coefficients
//
//	inputs:
//      const char *  : type of light source ("unknown" if not specified)
//                      (in the environment variable of "AMPAS_ILLUMINANT_PATH"
//                       such as "/usr/local/include/rawtoaces/data/Illuminant")
//
//	outputs:
//		int : "1" means loading/injecting light source datasets successfully,
//            "0" means error / no illumiant data has been loaded

int AcesRender::fetchIlluminant( const char *illumType )
{
    vector<string> paths;

    FORI( _opts.envPaths.size() )
    {
        vector<string> iFiles =
            openDir( ( _opts.envPaths )[i] + "/illuminant" );
        for ( vector<string>::iterator file = iFiles.begin();
              file != iFiles.end();
              ++file )
        {
            string fn( *file );
            if ( fn.find( ".json" ) == std::string::npos )
                continue;
            paths.push_back( fn );
        }
    }

    return _idt->loadIlluminant( paths, static_cast<string>( illumType ) );
}

vector<string> findFiles( string filePath, vector<string> searchPaths )
{
    vector<string> foundFiles;

    for ( auto &i: searchPaths )
    {
        string path = i + "/" + filePath;

        if ( boost::filesystem::exists( path ) )
            foundFiles.push_back( path );
    }

    return foundFiles;
}

//	=====================================================================
//  Calculate IDT matrix from camera spectral sensitivity data and the
//  selected or specified light source data. THe best White balance
//  coefficients will be generated in the process of obtaining IDT.
//
//	inputs:
//      libraw_iparams_t              : main parameters read from RAW
//      libraw_colordata_t            : color information from RAW
//
//	outputs:
//		int                           : "1" means IDT matrix generated;
//                                      "0" means error in calculation.

int AcesRender::prepareIDT( const libraw_iparams_t &P, float *M )
{
    // _rawProcessor->imgdata.idata
    int read = fetchCameraSenPath( P );

    if ( !read )
    {
        fprintf(
            stderr,
            "\nError: No matching cameras found. "
            "Please use other options for "
            "\"--mat-method\" and/or \"--wb-method\".\n" );
        exit( -1 );
    }

    vector<string> foundFiles =
        findFiles( "training/training_spectral.json", _opts.envPaths );
    if ( foundFiles.size() )
    {
        // loading training data (190 patches)
        _idt->loadTrainingData( foundFiles[0] );
    }

    foundFiles = findFiles( "cmf/cmf_1931.json", _opts.envPaths );
    if ( foundFiles.size() )
    {
        _idt->loadCMF( foundFiles[0] );
    }

    _idt->setVerbosity( _opts.verbosity );
    if ( _opts.illumType )
        _idt->chooseIllumType( _opts.illumType, _opts.highlight );
    else
    {
        vector<double> mulV( M, M + 3 );
        _idt->chooseIllumSrc( mulV, _opts.highlight );
    }

    if ( _opts.verbosity > 1 )
        printf( "Regressing IDT matrix coefficients ...\n" );

    if ( _idt->calIDT() )
    {
        _idtm = _idt->getIDT();
        _wbv  = _idt->getWB();

        return 1;
    }

    return 0;
}

//	=====================================================================
//  Calculate just white balance coefficients from camera spectral
//  sensitivity data and the best or specified light source data.
//  IDT matrix will not be calculated here, as curve-fitting may
//  take more time
//
//	inputs:
//      libraw_iparams_t   : main parameters read from RAW
//      libraw_colordata_t : color information from RAW
//
//	outputs:
//		int                : "1" means white balance coefficients generated;
//                           "0" means error during calculation

int AcesRender::prepareWB( const libraw_iparams_t &P )
{
    int read = fetchCameraSenPath( P );

    if ( !read )
    {
        fprintf(
            stderr,
            "\nError: No matching cameras found. "
            "Please use other options for "
            "\"--wb-method\".\n" );
        exit( -1 );
    }

    assert( _opts.illumType );
    read = fetchIlluminant( _opts.illumType );

    if ( !read )
    {
        fprintf(
            stderr,
            "\nError: No matching light source. "
            "Please find available options by "
            "\"rawtoaces --valid-illum\".\n" );
        exit( -1 );
    }
    else
    {
        vector<string> foundFiles =
            findFiles( "training/training_spectral.json", _opts.envPaths );
        if ( foundFiles.size() )
        {
            // loading training data (190 patches)
            _idt->loadTrainingData( foundFiles[0] );
        }

        foundFiles = findFiles( "cmf/cmf_1931.json", _opts.envPaths );
        if ( foundFiles.size() )
        {
            _idt->loadCMF( foundFiles[0] );
        }

        // choose the best light source based on
        // as-shot white balance coefficients
        _idt->chooseIllumType( _opts.illumType, _opts.highlight );

        if ( _opts.verbosity > 1 )
        {
            printf(
                "Calculating White Balance Coefficients "
                "from Spectral Sensitivity ...\n" );
            printf(
                "Applying Calculated White Balance "
                "Coefficients ...\n" );
        }

        _wbv = _idt->getWB();

        return 1;
    }

    return 0;
}

//  =====================================================================
//  Conduct dcraw process on the RAW
//
//  inputs:
//      const char *       : path to the raw file
//
//  outputs:
//      int                : "1" means raw file successfully pre-processed;
//                           "0" means error when pre-processing the file

int AcesRender::dcraw()
{
    assert( _opts.ret == LIBRAW_SUCCESS );

    if ( LIBRAW_SUCCESS != ( _opts.ret = _rawProcessor->dcraw_process() ) )
    {
        fprintf(
            stderr,
            "Error: Cannot do postpocessing: %s\n\n",
            libraw_strerror( _opts.ret ) );
        _opts.ret = errno;

        if ( LIBRAW_FATAL_ERROR( _opts.ret ) )
            exit( 1 );
    }

    return _opts.ret;
}

//  =====================================================================
//  Preprocess the RAW file based on the path to the file
//
//  inputs:
//      const char *       : path to the raw file
//
//  outputs:
//      int                : "1" means raw file successfully pre-processed;
//                           "0" means error when pre-processing the file

int AcesRender::preprocessRaw( const char *path )
{
    assert( path != nullptr );

    size_t len = strlen( path );
    _pathToRaw = (char *)malloc( len + 1 );
    memset( _pathToRaw, 0x0, len );
    memcpy( _pathToRaw, path, len );
    _pathToRaw[len] = '\0';

    // if ( _opts.verbosity > 2 )
    //     _rawProcessor->set_progress_handler ( my_progress_callback,
    //                                         ( void * )"Sample data passed" );
    // Start rawtoaces
    if ( _opts.verbosity )
    {
        printf( "\nStarting rawtoaces ...\n" );
        printf( "Processing %s ...\n", path );
    }

#ifdef LIBRAW_USE_OPENMP
    if ( _opts.verbosity )
        printf( "Using %d threads\n", omp_get_max_threads() );
#endif

    if ( openRawPath( path ) )
        unpack( path );

    return _opts.ret;
}

//  =====================================================================
//  Postprocess the RAW file
//
//  inputs:
//      N/A
//
//  outputs:
//      int                : "1" means raw file successfully pre-processed;
//                           "0" means error when pre-processing the file

int AcesRender::postprocessRaw()
{
    assert( _opts.ret == LIBRAW_SUCCESS );

#ifdef OUT
#    undef OUT
#endif

#define OUT _rawProcessor->imgdata.params
#define P _rawProcessor->imgdata.idata
#define C _rawProcessor->imgdata.color

    if ( _opts.verbosity > 1 )
        printf(
            "The camera has been identified as a %s %s ...\n",
            P.make,
            P.model );

    switch ( _opts.wb_method )
    {
            // 0
        case wbMethod0: {
            _opts.use_mul = 1;
            vector<double> mulV( C.cam_mul, C.cam_mul + 3 );

            FORI( 3 ) OUT.user_mul[i] = static_cast<float>( mulV[i] );

            if ( _opts.verbosity > 1 )
            {
                printf( "White Balance method is 0 - " );
                printf(
                    "Using white balance factors from "
                    "file metadata ...\n" );
            }
            break;
        }
        // 1
        case wbMethod1: {
            if ( prepareWB( _rawProcessor->imgdata.idata ) )
            {
                _opts.use_mul             = 1;
                vector<double> wbv        = getWB();
                FORI( 3 ) OUT.user_mul[i] = static_cast<float>( wbv[i] );
            }
            else
            {
                fprintf(
                    stderr,
                    "\nError: Cannot obtain a set of White "
                    "Balance Coefficient Factors \n" );
                _opts.ret = errno;
                return 0;
            }

            if ( _opts.verbosity > 1 )
            {
                printf( "White Balance calculation method is 1 - " );
                printf(
                    "Using white balance factors calculated from "
                    "spec sens and user specified illuminant ...\n" );
            }

            break;
        }
        // 2
        case wbMethod2: {
            OUT.use_auto_wb = 1;

            if ( _opts.verbosity > 1 )
            {
                printf( "White Balance calculation method is 2 - " );
                printf(
                    "Using white balance factors calculate by "
                    "averaging the entire image ...\n" );
            }

            break;
        }
        // 3
        case wbMethod3: {
            if ( _opts.verbosity > 1 )
            {
                printf( "White Balance calculation method is 3 - " );
                printf(
                    "Using white balance factors calculated by "
                    "averaging a grey box ...\n" );
            }

            break;
        }
        // 4
        case wbMethod4: {
            _opts.use_mul = 1;
            double sc     = dmax;

            FORI( P.colors )
            {
                if ( OUT.user_mul[i] <= sc )
                    sc = OUT.user_mul[i];
            }

            if ( sc != 1.0 )
            {
                fprintf(
                    stderr,
                    "\nWarning: The smallest channel  "
                    "multiplier should be 1.0.\n\n" );
            }

            if ( _opts.verbosity > 1 )
            {
                printf( "White Balance calculation method is 4 - " );
                printf( "Using user-supplied white balance factors ...\n" );
            }

            break;
        }
        default: {
            fprintf(
                stderr,
                "White Balance method is must be 0, 1, 2, 3, "
                "or 4 \n" );
            exit( -1 );
            break;
        }
    }

    //  Set parameters for --mat-method
    switch ( _opts.mat_method )
    {
        // 0
        case matMethod0: {
            OUT.output_color      = 0;
            OUT.use_camera_matrix = 0;

            if ( _opts.verbosity > 1 )
            {
                printf( "IDT matrix calculation method is 0 - " );
                printf( "Calculating IDT matrix using camera spec sens ...\n" );
            }

            break;
        }
        // 1
        case matMethod1:
            OUT.use_camera_matrix = 3;

            if ( P.dng_version )
            {
                OUT.use_camera_matrix = 1;

                if ( _opts.verbosity > 1 )
                    printf( "DNG file uses embeded color profile. \n " );
            }

            if ( _opts.verbosity > 1 )
            {
                printf( "IDT matrix calculation method is 1 - " );
                printf( "Calculating IDT matrix using file metadata ...\n" );
            }

            break;
        // 2
        case matMethod2:
            OUT.use_camera_matrix = 1;

            if ( _opts.verbosity > 1 )
            {
                printf( "IDT matrix calculation method is 2 - " );
                printf(
                    "Calculating IDT matrix using adobe coeffs included in libraw ...\n" );
            }

            break;
            // 3
        case matMethod3:
            OUT.output_color = 0;
            if ( _opts.verbosity > 1 )
            {
                printf( "IDT matrix calculation method is 3 - " );
                printf( "IDT matrix is custom defined ...\n" );
            }

            break;
        default:
            fprintf(
                stderr,
                "IDT matrix calculation method is must be 0, 1, 2, 3\n" );
            exit( -1 );
            break;
    }

    // Set four_color_rgb to 0 when half_size is set to 1
    if ( OUT.half_size == 1 )
        OUT.four_color_rgb = 0;

    if ( P.dng_version && OUT.output_color != 0 )
    {
        OUT.use_camera_matrix = 1;
        OUT.use_camera_wb     = 1;
    }

    _opts.ret = dcraw();
    if ( _opts.mat_method == matMethod0 )
        if ( !prepareIDT( P, C.pre_mul ) )
            _opts.ret = errno;

    libraw_processed_image_t *image =
        _rawProcessor->dcraw_make_mem_image( &( _opts.ret ) );
    setPixels( image );

    return _opts.ret;
}

//	=====================================================================
//	Render ACES Buffer
//
//	inputs:
//      N/A
//
//	outputs:
//      N/A        : either call renderIDT() or renderDNG()
//                   or renderNonDNG()

float *AcesRender::renderACES()
{
#ifdef P
#    undef P
#endif

#define P _rawProcessor->imgdata.idata
    if ( !_rawProcessor->imgdata.params.output_color )
    {
        cout << "rendering IDT" << endl;
        return renderIDT();
    }
    else
    {
        if ( P.dng_version )
        {
            cout << "rendering DNG" << endl;
            return renderDNG();
        }
        else
        {
            cout << "rendering NonDNG" << endl;
            return renderNonDNG();
        }
    }
}
//	=====================================================================
//	Write rendered ACES Buffer into an OpenEXR Image File
//
//	inputs:
//      N/A
//
//	outputs:
//      N/A        : An ACES file will be generated

void AcesRender::outputACES( const char *path )
{
#ifdef C
#    undef C
#endif

#define C _rawProcessor->imgdata.color

    assert( _pathToRaw != nullptr );

    float *aces = renderACES();
    if ( _opts.verbosity > 1 )
    {
        if ( _opts.mat_method && !P.dng_version )
        {
            vector<vector<double>> camXYZ( 3, vector<double>( 3, 1.0 ) );
            FORIJ( 3, 3 ) camXYZ[i][j]    = C.cam_xyz[i][j];
            vector<vector<double>> camcat = mulVector( camXYZ, _catm );

            printf( "The Approximate IDT matrix is ...\n" );
            FORI( 3 )
            printf(
                "   %f, %f, %f\n", camcat[i][0], camcat[i][1], camcat[i][2] );
        }
        // printing white balance coefficients
        printf( "The final white balance coefficients are ...\n" );
        printf( "   %f   %f   %f\n", C.pre_mul[0], C.pre_mul[1], C.pre_mul[2] );
        printf( "Writing ACES file to %s ...\n", path );
    }

    if ( _opts.highlight > 0 )
    {
        float ratio =
            ( *( std::max_element( C.pre_mul, C.pre_mul + 3 ) ) /
              *( std::min_element( C.pre_mul, C.pre_mul + 3 ) ) );
        acesWrite( path, aces, ratio );
    }
    else
        acesWrite( path, aces );

#ifndef WIN32
    if ( _opts.use_mmap && _opts.iobuffer )
    {
        munmap( _opts.iobuffer, size_t( _opts.msize ) );
        _opts.iobuffer = 0;
    }
#endif

    _rawProcessor->recycle();

    if ( _opts.verbosity )
        printf( "Finished\n\n" );
}

//	=====================================================================
//  Apply white balance values to each pixel
//  ( We actually do not need it here because white-balancing
//  happens before demosaicing )
//
//	inputs:
//      float *          : pixels (R/G/B)
//      uint8_t          : number of channels
//      uint32_t         : the size of pixels
//
//	outputs:
//		N/A              : pixel values modified by multiplying
//                         white balance coefficients

void AcesRender::applyWB( float *pixels, int bits, uint32_t total )
{
    double min_wb = *min_element( _wbv.begin(), _wbv.end() );
    double target = 1.0;

    if ( bits == 8 )
        target /= INV_255;
    else if ( bits == 16 )
        target /= INV_65535;

    if ( !pixels )
    {
        fprintf( stderr, "\nThe pixels cannot be found. \n" );
        exit( 1 );
    }
    else
    {
        for ( uint32_t i = 0; i < total; i += 3 )
        {
            pixels[i]     = clip( _wbv[0] * pixels[i] / min_wb, target );
            pixels[i + 1] = clip( _wbv[1] * pixels[i + 1] / min_wb, target );
            pixels[i + 2] = clip( _wbv[2] * pixels[i + 2] / min_wb, target );
        }
    }
}

//	=====================================================================
//  Apply IDT matrix to each pixel
//
//	inputs:
//      float *   : pixels (R/G/B)
//      uint8_t   : number of channels
//      uint32_t  : the size of pixels
//      vector < vector <double> >: 3 x 3 IDT matrix
//
//	outputs:
//		N/A       : pixel values modified by mutiplying IDT matrix

void AcesRender::applyIDT( float *pixels, int channel, uint32_t total )
{
    assert( pixels );
    cout << "applying IDT" << endl;

    if ( _opts.mat_method != matMethod3 )
    {

        if ( channel == 4 )
        {
            _idtm.resize( 4 );
            FORI( 4 ) _idtm[i].resize( 4 );

            _idtm[0][3] = 0.0;
            _idtm[1][3] = 0.0;
            _idtm[2][3] = 0.0;
            _idtm[3][0] = 0.0;
            _idtm[3][1] = 0.0;
            _idtm[3][2] = 0.0;
            _idtm[3][3] = 1.0;
        }

        if ( channel != 3 && channel != 4 )
        {
            fprintf(
                stderr,
                "\nError: Currently support 3 channels "
                "and 4 channels. \n" );
            exit( 1 );
        }

        pixels = mulVectorArray( pixels, total, channel, _idtm );

        FORI( 3 )
        {

            cout << _idtm[i][0] << " " << _idtm[i][1] << " " << _idtm[i][2]
                 << endl;
        }
    }
    else if ( _opts.mat_method == matMethod3 )
    {
        cout << "Using custom defined matrix for IDT" << endl;
        custom_idtm.resize( 3 );

        FORI( 3 )
        {
            custom_idtm[i].resize( 3 );

            FORJ( 3 )
            custom_idtm[i][j] = static_cast<double>( custom_Matrix[i][j] );
        }

        if ( channel == 4 )
        {
            custom_idtm.resize( 4 );
            FORI( 4 ) custom_idtm[i].resize( 4 );

            custom_idtm[0][3] = 0.0;
            custom_idtm[1][3] = 0.0;
            custom_idtm[2][3] = 0.0;
            custom_idtm[3][0] = 0.0;
            custom_idtm[3][1] = 0.0;
            custom_idtm[3][2] = 0.0;
            custom_idtm[3][3] = 1.0;
        }

        if ( channel != 3 && channel != 4 )
        {
            fprintf(
                stderr,
                "\nError: Currently support 3 channels "
                "and 4 channels. \n" );
            exit( 1 );
        }

        pixels = mulVectorArray( pixels, total, channel, custom_idtm );

        FORI( 3 )
        {

            cout << custom_idtm[i][0] << " " << custom_idtm[i][1] << " "
                 << custom_idtm[i][2] << endl;
        }
    }
}

//	=====================================================================
//  Apply CAT matrix (e.g., D65 to D60) to each pixel
//  It will be used if using adobe coeffs from "libraw"
//
//	inputs:
//      float *   : pixels (R/G/B)
//      uint8_t   : number of channels
//      uint32_t  : the size of pixels
//
//	outputs:
//		N/A       : pixel values modified by mutiplying CAT matrix

void AcesRender::applyCAT( float *pixels, int channel, uint32_t total )
{
    assert( pixels );

    if ( channel != 3 && channel != 4 )
    {
        fprintf(
            stderr,
            "\nError: Currenly support 3 channels "
            "and 4 channels. \n" );
        exit( 1 );
    }

    // will use calCAT() inside rawtoaces
    vector<double> dIV( d65, d65 + 3 );
    vector<double> dOV( d60, d60 + 3 );
    _catm = getCAT( dIV, dOV );

    pixels = mulVectorArray( pixels, total, channel, _catm );
}

//	=====================================================================
//  Convert DNG RAW to aces file
//
//	inputs:
//      vector < float > : camera to display matrix
//
//	outputs:
//		float * : an array of converted aces values

float *AcesRender::renderDNG()
{
#ifdef P
#    undef P
#endif

#define P _rawProcessor->imgdata.idata

#ifdef R
#    undef R
#endif

#define R _rawProcessor->imgdata.rawdata

    assert( _image && P.dng_version );

    Metadata metadata;
    metadata.neutralRGB.resize( 3 );
    metadata.xyz2rgbMatrix1.resize( 9 );
    metadata.xyz2rgbMatrix2.resize( 9 );
    metadata.cameraCalibration1.resize( 9 );
    metadata.cameraCalibration2.resize( 9 );

#if LIBRAW_VERSION >= LIBRAW_MAKE_VERSION( 0, 20, 0 )
    metadata.baselineExposure =
        static_cast<double>( R.color.dng_levels.baseline_exposure );
#else
    metadata.baselineExposure =
        static_cast<double>( R.color.baseline_exposure );
#endif
    metadata.calibrationIlluminant1 =
        static_cast<double>( R.color.dng_color[0].illuminant );
    metadata.calibrationIlluminant2 =
        static_cast<double>( R.color.dng_color[1].illuminant );

    FORI( 3 )
    {
        metadata.neutralRGB[i] =
            1.0 / static_cast<double>( R.color.cam_mul[i] );
    }

    FORIJ( 3, 3 )
    {
        metadata.xyz2rgbMatrix1[i * 3 + j] =
            static_cast<double>( ( R.color.dng_color[0].colormatrix )[i][j] );
        metadata.xyz2rgbMatrix2[i * 3 + j] =
            static_cast<double>( ( R.color.dng_color[1].colormatrix )[i][j] );
        metadata.cameraCalibration1[i * 3 + j] =
            static_cast<double>( ( R.color.dng_color[0].calibration )[i][j] );
        metadata.cameraCalibration2[i * 3 + j] =
            static_cast<double>( ( R.color.dng_color[1].calibration )[i][j] );
    }

    DNGIdt *dng = new DNGIdt( metadata );
    _catm       = dng->getDNGCATMatrix();
    _idtm       = dng->getDNGIDTMatrix();

    if ( _opts.verbosity > 1 )
    {
        printf( "The Approximate IDT matrix is ...\n" );
        FORI( 3 )
        printf( "   %f, %f, %f\n", _idtm[i][0], _idtm[i][1], _idtm[i][2] );
    }

    ushort  *pixels = (ushort *)_image->data;
    uint32_t total  = _image->width * _image->height * _image->colors;
    float   *aces   = new ( std::nothrow ) float[total];
    FORI( total )
    aces[i] = static_cast<float>( pixels[i] );

    if ( _opts.verbosity > 1 )
        printf( "Applying IDT Matrix ...\n" );

    applyIDT( aces, _image->colors, total );
    delete dng;

    return aces;
}

//	=====================================================================
//  Convert Non-DNG RAW to aces file (no IDT involved)
//
//	inputs:  N/A
//
//	outputs:
//		float * : an array of converted aces code values

float *AcesRender::renderNonDNG()
{
    assert( _image );

    ushort  *pixels = (ushort *)_image->data; //Getting the image data
    uint32_t total  = _image->width * _image->height *
                     _image->colors; //Total number of data pixels
    float *aces = new ( std::nothrow ) float[total];

    FORI( total ) aces[i] = static_cast<float>( pixels[i] );

    if ( _opts.mat_method > 0 )
    {
        applyCAT(
            aces,
            _image->colors,
            total ); // Apply Chromatic Adaptation Transform
    }

    vector<vector<double>> XYZ_acesrgb(
        _image->colors, vector<double>( _image->colors ) );
    if ( _image->colors == 3 )
    {
        FORIJ( 3, 3 )
        XYZ_acesrgb[i][j] = XYZ_acesrgb_3
            [i][j]; //Populating a new vector with a pre-defined array.
        aces = mulVectorArray( aces, total, 3, XYZ_acesrgb );
    }
    else if ( _image->colors == 4 )
    {
        FORIJ( 4, 4 ) XYZ_acesrgb[i][j] = XYZ_acesrgb_4[i][j];
        aces = mulVectorArray( aces, total, 4, XYZ_acesrgb );
    }
    else
    {
        fprintf(
            stderr,
            "\nError: Currenly support 3 channels "
            "and 4 channels. \n" );
        exit( 1 );
    }

    return aces;
}

//	=====================================================================
//  Convert Non-DNG RAW to aces file through IDT
//
//	inputs:  N/A
//
//	outputs:
//		float * : an array of aces values for each pixel

float *AcesRender::renderIDT()
{
    assert( _image );
    ushort  *pixels = (ushort *)_image->data;
    uint32_t total  = _image->width * _image->height * _image->colors;
    float   *aces   = new ( std::nothrow ) float[total];

    FORI( total )
    {
        aces[i] = static_cast<float>( pixels[i] );
    }

    if ( _opts.verbosity > 1 )
        printf( "Applying IDT Matrix ...\n" );

    applyIDT( aces, _image->colors, total );

    return aces;
};

//	=====================================================================
//  Write processed image file to an aces-compliant openexr file
//
//	inputs:
//      const char *               : the name of output file
//      float *                    : an array of converted aces values
//
//	outputs:
//		N/A                        : an aces file should be generated in
//                                   the same folder

void AcesRender::acesWrite( const char *name, float *aces, float ratio ) const
{
    assert( aces );

    uint16_t width    = _image->width;
    uint16_t height   = _image->height;
    uint8_t  channels = _image->colors;
    uint8_t  bits     = _image->bits;

    halfBytes *halfIn = new ( std::nothrow )
        halfBytes[channels * width * height];

    FORI( channels * width * height )
    {
        if ( bits == 8 )
            aces[i] = (double)aces[i] * INV_255 * ( _opts.scale ) * ratio;
        else if ( bits == 16 )
            aces[i] = (double)aces[i] * INV_65535 * ( _opts.scale ) * ratio;

        Imath::half tmpV( aces[i] );
        halfIn[i] = tmpV.bits();
    }

    vector<std::string> filenames;
    filenames.push_back( name );

    aces_Writer x;

    MetaWriteClip writeParams;

    writeParams.duration        = 1;
    writeParams.outputFilenames = filenames;

    writeParams.outputRows = height;
    writeParams.outputCols = width;

    writeParams.hi                   = x.getDefaultHeaderInfo();
    libraw_iparams_t *iparams        = &_rawProcessor->imgdata.idata;
    writeParams.hi.originalImageFlag = 1;
    writeParams.hi.software          = "rawtoaces v0.1";
    writeParams.hi.cameraMake        = string( iparams->make );
    writeParams.hi.cameraModel       = string( iparams->model );
    writeParams.hi.cameraLabel =
        writeParams.hi.cameraMake + " " + writeParams.hi.cameraModel;

    libraw_lensinfo_t *lens         = &_rawProcessor->imgdata.lens;
    writeParams.hi.lensMake         = string( lens->LensMake );
    writeParams.hi.lensModel        = string( lens->Lens );
    writeParams.hi.lensSerialNumber = string( lens->LensSerial );

    libraw_imgother_t *other   = &_rawProcessor->imgdata.other;
    writeParams.hi.isoSpeed    = other->iso_speed;
    writeParams.hi.expTime     = other->shutter;
    writeParams.hi.aperture    = other->aperture;
    writeParams.hi.focalLength = other->focal_len;
    writeParams.hi.comments    = string( other->desc );
    writeParams.hi.artist      = string( other->artist );
    writeParams.hi.channels.clear();

    switch ( channels )
    {
        case 3:
            writeParams.hi.channels.resize( 3 );
            writeParams.hi.channels[0].name = "B";
            writeParams.hi.channels[1].name = "G";
            writeParams.hi.channels[2].name = "R";
            break;
        case 4:
            writeParams.hi.channels.resize( 4 );
            writeParams.hi.channels[0].name = "A";
            writeParams.hi.channels[1].name = "B";
            writeParams.hi.channels[2].name = "G";
            writeParams.hi.channels[3].name = "R";
            break;
        case 6:
            throw std::invalid_argument(
                "Stereo RGB support not yet implemented" );
        case 8:
            throw std::invalid_argument(
                "Stereo RGB support not yet implemented" );
        default:
            throw std::invalid_argument(
                "Only RGB, RGBA or"
                "stereo RGB[A] file supported" );
            break;
    }

    DynamicMetadata dynamicMeta;
    dynamicMeta.imageIndex   = 0;
    dynamicMeta.imageCounter = 0;

    x.configure( writeParams );
    x.newImageObject( dynamicMeta );

    FORI( height )
    {
        halfBytes *rgbData = halfIn + width * channels * i;
        x.storeHalfRow( rgbData, i );
    }

#if 0
    std::cout << "saving aces file" << std::endl;
    std::cout << "size " << width << "x" << height << "x"
              << channels << std::endl;
    std::cout << "size " << writeParams.outputCols << "x"
              << writeParams.outputRows << std::endl;
    std::cout << "duration " << writeParams.duration << std::endl;
    std::cout << writeParams.hi;
    std::cout << "\ndynamic meta" << std::endl;
    std::cout << "imageIndex " << dynamicMeta.imageIndex << std::endl;
    std::cout << "imageCounter " << dynamicMeta.imageCounter << std::endl;
    std::cout << "timeCode " << dynamicMeta.timeCode << std::endl;
    std::cout << "keyCode " << dynamicMeta.keyCode << std::endl;
    std::cout << "capDate " << dynamicMeta.capDate << std::endl;
    std::cout << "uuid " << dynamicMeta.uuid << std::endl;
#endif

    delete[] halfIn;

    x.saveImageObject();
}

//	=====================================================================
//	Get a list of Supported Illuminants
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < vector < string > > : _illuminant values
//	=====================================================================

const vector<string> AcesRender::getSupportedIllums() const
{
    return _illuminants;
}

//	=====================================================================
//	Get a list of Supported Cameras
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < string > : _cameras values/names

const vector<string> AcesRender::getSupportedCameras() const
{
    return _cameras;
}

//	=====================================================================
//	Print a list of Cameras Supported by LibRaw
//
//	inputs:
//      N/A
//
//	outputs:
//      N/A

void AcesRender::printLibRawCameras()
{
    const char **cl = _rawProcessor->cameraList();
    while ( *( cl + 1 ) != NULL )
        printf( "%s\n", *cl++ );
}

//	=====================================================================
//	Get IDT matrix
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < vector < double > > : _idtm (3x3) values

const vector<vector<double>> AcesRender::getIDTMatrix() const
{
    return _idtm;
}

//	=====================================================================
//	Get CAT matrix
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < vector < double > > : _catm (3x3) values

const vector<vector<double>> AcesRender::getCATMatrix() const
{
    return _catm;
}

//	=====================================================================
//	Get White Balance Coefficients
//
//	inputs:
//      N/A
//
//	outputs:
//      vector < double >  : _wbv (1x3) values

const vector<double> AcesRender::getWB() const
{
    return _wbv;
}

//	=====================================================================
//	Get Image Buffer
//
//	inputs:
//      N/A
//
//	outputs:
//      libraw_processed_image_t : _image

const libraw_processed_image_t *AcesRender::getImageBuffer() const
{
    assert( _image );

    return _image;
}

//	=====================================================================
//	Fetch user option list
//
//	inputs:
//      NA
//
//	outputs:
//      Options   :  _opts will be returned

const struct Option AcesRender::getSettings() const
{
    return _opts;
}
