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
// COLOR ENCODING SYSTEM, OR APPLICATIONS THEREO,  HELD BY PARTIES OTHER
// THAN A.M.P.A.S., WHETHER DISCLOSED OR UNDISCLOSED.
///////////////////////////////////////////////////////////////////////////
#ifndef _DEFINE_h__
#define _DEFINE_h__

#include <string>
#include <algorithm>
#include <boost/filesystem.hpp>

#ifndef WIN32
#    include <sys/stat.h>
#endif

#define INV_255 ( 1.0 / (double)255.0 )
#define INV_65535 ( 1.0 / (double)65535.0 )

#ifdef WIN32
// suppress sprintf-related warning. sprintf() is permitted in sample code
#    include <string.h>
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
//#    define snprintf _snprintf
#    define _CRT_SECURE_NO_WARNINGS
#    define cmp_str stricmp
#else
#    define cmp_str strcasecmp
#endif

#if !defined( TRUE )
#    define TRUE 1
#endif

#if !defined( FALSE )
#    define FALSE 0
#endif

#define FORI( val ) for ( int i = 0; i < val; i++ )
#define FORJ( val ) for ( int j = 0; j < val; j++ )
#define FORIJ( val1, val2 )                                                    \
    for ( int i = 0; i < val1; i++ )                                           \
        for ( int j = 0; j < val2; j++ )
#define countSize( a ) ( static_cast<int>( sizeof( a ) / sizeof( ( a )[0] ) ) )

typedef float  float32_t;
typedef double float64_t;

using namespace std;

enum matMethods_t
{
    matMethod0,
    matMethod1,
    matMethod2,
    matMethod3
};
enum wbMethods_t
{
    wbMethod0,
    wbMethod1,
    wbMethod2,
    wbMethod3,
    wbMethod4
};

struct Option
{
    int ret;
    int use_bigfile;
    int use_timing;
    int use_illum;
    int use_mul;
    int use_mmap;
    int msize;
    int verbosity;
    int highlight;
    int get_illums;
    int get_cameras;
    int get_libraw_cameras;

    matMethods_t mat_method;
    wbMethods_t  wb_method;

    char          *illumType;
    float          scale;
    vector<string> envPaths;

#ifndef WIN32
    void *iobuffer;
#endif
};

struct dataPath
{
    string         os;
    vector<string> paths;
};

static float                  custom_Matrix[3][3] = { 0.0 };
static float                  custom_Buffer[9]    = { 0.0 };
static vector<vector<double>> custom_idtm;

const double pi = 3.1416;
// 216.0/24389.0
const double e = 0.008856451679;
// (24389.0/27.0)/116.0
const double k = 7.787037037037;

// Planck's constant ([J*s] Joule-seconds)
const double bh = 6.626176 * 1e-34;
// Boltzmann constant ([J/K] Joules per Kelvin)
const double bk = 1.380662 * 1e-23;
// Speed of light ([m/s] meters per second)
const double bc = 2.99792458 * 1e8;

const double dmin = numeric_limits<double>::min();
const double dmax = numeric_limits<double>::max();

// clang-format off

static const double XYZ_w[3] = {0.952646074569846, 1.0,    1.00882518435159};
static const double d50  [3] = {0.9642,            1.0000, 0.8250};
static const double d60  [3] = {0.952646074569846, 1.0000, 1.00882518435159};
static const double d65  [3] = {0.9547,            1.0000, 1.0883};

static const double neutral3[3][3] = {
    {1.0, 0.0, 0.0},
    {0.0, 1.0, 0.0},
    {0.0, 0.0, 1.0}
};

static const struct
{
    int    wl;
    double RGB[3];
} s_series[54] = {
    { 300,  {   0.04,  0.02, 0.0 } },
    { 310,	{   6.0,   4.5,  2.0 } },
    { 320,	{  29.6,  22.4,  4.0 } },
    { 330,	{  55.3,  42.0,  8.5 } },
    { 340,	{  57.3,  40.6,  7.8 } },
    { 350,	{  61.8,  41.6,  6.7 } },
    { 360,	{  61.5,  38.0,  5.3 } },
    { 370,	{  68.8,  42.4,  6.1 } },
    { 380,	{  63.4,  38.5,  3.0 } },
    { 390,	{  65.8,  35.0,  1.2 } },
    { 400,	{  94.8,  43.4, -1.1 } },
    { 410,	{ 104.8,  46.3, -0.5 } },
    { 420,	{ 105.9,  43.9,	-0.7 } },
    { 430,	{  96.8,  37.1,	-1.2 } },
    { 440,	{ 113.9,  36.7, -2.6 } },
    { 450,	{ 125.6,  35.9,	-2.9 } },
    { 460,	{ 125.5,  32.6,	-2.8 } },
    { 470,	{ 121.3,  27.9,	-2.6 } },
    { 480,	{ 121.3,  24.3,	-2.6 } },
    { 490,	{ 113.5,  20.1,	-1.8 } },
    { 500,	{ 113.1,  16.2,	-1.5 } },
    { 510,	{ 110.8,  13.2,	-1.3 } },
    { 520,	{ 106.5,   8.6, -1.2 } },
    { 530,	{ 108.8,   6.1, -1.0 } },
    { 540,	{ 105.3,   4.2,	-0.5 } },
    { 550,	{ 104.4,   1.9,	-0.3 } },
    { 560,	{ 100.0,   0.0,	 0.0 } },
    { 570,	{  96.0,  -1.6,	 0.2 } },
    { 580,	{  95.1,  -3.5,	 0.5 } },
    { 590,	{  89.1,  -3.5,	 2.1 } },
    { 600,	{  90.5,  -5.8,	 3.2 } },
    { 610,	{  90.3,  -7.2,	 4.1 } },
    { 620,	{  88.4,  -8.6,	 4.7 } },
    { 630,	{  84.0,  -9.5,	 5.1 } },
    { 640,	{  85.1, -10.9,	 6.7 } },
    { 650,	{  81.9, -10.7,	 7.3 } },
    { 660,	{  82.6, -12.0,	 8.6 } },
    { 670,	{  84.9, -14.0,	 9.8 } },
    { 680,	{  81.3, -13.6,	10.2 } },
    { 690,	{  71.9, -12.0,	 8.3 } },
    { 700,	{  74.3, -13.3,	 9.6 } },
    { 710,	{  76.4, -12.9,	 8.5 } },
    { 720,	{  63.3, -10.6,	 7.0 } },
    { 730,	{  71.7, -11.6,	 7.6 } },
    { 740,	{  77.0, -12.2,	 8.0 } },
    { 750,	{  65.2, -10.2,	 6.7 } },
    { 760,	{  47.7,  -7.8,	 5.2 } },
    { 770,	{  68.6, -11.2,	 7.4 } },
    { 780,	{  65.0, -10.4,	 6.8 } },
    { 790,	{  66.0, -10.6,	 7.0 } },
    { 800,	{  61.0,  -9.7,	 6.4 } },
    { 810,	{  53.3,  -8.3,	 5.5 } },
    { 820,	{  58.9,  -9.3,	 6.1 } },
    { 830,	{  61.9,  -9.8,	 6.5 } },
};

static const double XYZ_D65_acesrgb_3[3][3] = {
    {  1.0634731317028,    0.00639793641966071, -0.0157891874506841 },
    { -0.492082784686793,  1.36823709310019,     0.0913444629573544 },
    { -0.0028137154424595, 0.00463991165243123,  0.91649468506889   }
};

static const double XYZ_D65_acesrgb_4[4][4] = {
    { 1.0634731317028,     0.00639793641966071, -0.0157891874506841, 0.0 },
    { -0.492082784686793,  1.36823709310019,     0.0913444629573544, 0.0 },
    { -0.0028137154424595, 0.00463991165243123,  0.91649468506889,   0.0 },
    { 0.0,                 0.0,                  0.0,                1.0 }
};

static const double XYZ_acesrgb_3[3][3] = {
    {  1.0498110175, 0.0000000000, -0.0000974845 },
    { -0.4959030231, 1.3733130458,  0.0982400361 },
    {  0.0000000000, 0.0000000000,  0.9912520182 }
};

static const double XYZ_acesrgb_4[4][4] = {
    {  1.0498110175, 0.0000000000, -0.0000974845, 0.0 },
    { -0.4959030231, 1.3733130458,  0.0982400361, 0.0 },
    {  0.0000000000, 0.0000000000,  0.9912520182, 0.0 },
    {  0.0,          0.0,           0.0,          1.0 }
};

static const float XYZ_acesrgb_float_4[4][4] = {
    {  1.0498110175, 0.0000000000, -0.0000974845, 0.0 },
    { -0.4959030231, 1.3733130458,  0.0982400361, 0.0 },
    {  0.0000000000, 0.0000000000,  0.9912520182, 0.0 },
    {  0.0,          0.0,           0.0,          1.0 }
};

static const float XYZ_acesrgb_transposed_4[4][4] = {
    {  1.0498110175, -0.4959030231, 0.0000000000, 0.0 },
    {  0.0000000000,  1.3733130458, 0.0000000000, 0.0 },
    { -0.0000974845,  0.0982400361, 0.9912520182, 0.0 },
    {  0.0000000000,  0.0000000000, 0.0000000000, 1.0 }
};

static const double acesrgb_XYZ_3[3][3] = {
    { 0.952552395938186, 0.0,                9.36786316604686e-05 },
    { 0.343966449765075, 0.728166096613485, -0.0721325463785608   },
    { 0.0,               0.0,                1.00882518435159     }
};

static const double chromaticitiesACES[4][2] = {
    { 0.73470,  0.26530 },
    { 0.00000,  1.00000 },
    { 0.00010, -0.07700 },
    { 0.32168,  0.33767 }
};

// Roberson UV Table
static const double Robertson_uvtTable[][3] = {
    { 0.18006,  0.26352,   -0.24341 },
    { 0.18066,  0.26589,   -0.25479 },
    { 0.18133,  0.26846,   -0.26876 },
    { 0.18208,  0.27119,   -0.28539 },
    { 0.18293,  0.27407,   -0.3047  },
    { 0.18388,  0.27709,   -0.32675 },
    { 0.18494,  0.28021,   -0.35156 },
    { 0.18611,  0.28342,   -0.37915 },
    { 0.18740,  0.28668,   -0.40955 },
    { 0.18880,  0.28997,   -0.44278 },
    { 0.19032,  0.29326,   -0.47888 },
    { 0.19462,  0.30141,   -0.58204 },
    { 0.19962,  0.30921,   -0.70471 },
    { 0.20525,  0.31647,   -0.84901 },
    { 0.21142,  0.32312,   -1.0182  },
    { 0.21807,  0.32909,   -1.2168  },
    { 0.22511,  0.33439,   -1.4512  },
    { 0.23247,  0.33904,   -1.7298  },
    { 0.24010,  0.34308,   -2.0637  },
    { 0.24792,  0.34655,   -2.4681  },
    { 0.25591,  0.34951,   -2.9641  },
    { 0.26400,  0.35200,   -3.5814  },
    { 0.27218,  0.35407,   -4.3633  },
    { 0.28039,  0.35577,   -5.3762  },
    { 0.28863,  0.35714,   -6.7262  },
    { 0.29685,  0.35823,   -8.5955  },
    { 0.30505,  0.35907,  -11.324   },
    { 0.31320,  0.35968,  -15.628   },
    { 0.32129,  0.36011,  -23.325   },
    { 0.32931,  0.36038,  -40.77    },
    { 0.33724,  0.36051, -116.45    }
};

// Roberson Mired Matrix
static const double RobertsonMired[] = {
      1.0e-10, 10.0,  20.0,  30.0,  40.0,  50.0,  60.0,  70.0,
     80.0,     90.0, 100.0, 125.0, 150.0, 175.0, 200.0, 225.0,
    250.0,    275.0, 300.0, 325.0, 350.0, 375.0, 400.0, 425.0,
    450.0,    475.0, 500.0, 525.0, 550.0, 575.0, 600.0
};

//  Color Adaptation Matrix - Bradford
static const double bradford[3][3] = {
    { 0.8951,  0.2664, -0.1614},
    {-0.7502,  1.7135,  0.0367},
    { 0.0389, -0.0685,  1.0296}
};

//  Color Adaptation Matrices - Cat02 (default)
static const double cat02[3][3] = {
    {  0.7328, 0.4296, -0.1624 },
    { -0.7036, 1.6975,  0.0061 },
    {  0.0030, 0.0136,  0.9834 }
};

// clang-format on

// Function to Open Directories
inline vector<string> openDir( string path = "." )
{
    vector<string> paths;

    for ( auto &i: boost::filesystem::directory_iterator( path ) )
    {
        if ( i.status().type() != boost::filesystem::file_type::directory_file )
            paths.push_back( i.path().string() );
    }

    return paths;
};

// Function to clear the memories occupied by vectors
template <typename T> inline void clearVM( vector<T> vct )
{
    vector<T>().swap( vct );
};

// Function to covert upper-case to lower-case
inline void lowerCase( char *tex )
{
    string tmp( tex );

    FORI( tmp.size() )
    tex[i] = tolower( tex[i] );
};

// Function to check if a value is numeric
inline bool isNumeric( const char *val )
{
    string base = "0123456789E-.";
    string input( val );

    return (
        input.find_first_not_of( base.substr( 0, base.size() ) ) ==
        string::npos );
};

// Function to check if a input is a alphabetic letter
inline bool isCTLetterDigit( const char c )
{
    return (
        ( c >= 'a' && c <= 'z' ) || ( c >= 'A' && c <= 'Z' ) || ( c == '-' ) ||
        isdigit( c ) );
};

// Function to check if a string is a valid input
// to represent color temperature(s) (e.g., D60, 3200K)
inline bool isValidCT( string str )
{
    int    i      = 0;
    size_t length = str.length();

    if ( length == 0 )
        return false;

    // other light sources
    if ( str[0] != 'd' && str[length - 1] != 'k' )
    {
        while ( str[i] )
        {
            if ( !isCTLetterDigit( str[i] ) )
                return false;
            i++;
        }
    }
    // daylight ("D" + Numeric values)
    else if ( str[0] == 'd' && str[length - 1] != 'k' )
    {
        i = 1;
        while ( str[i] )
        {
            if ( !isdigit( str[i] ) )
                return false;
            i++;
        }
    }
    // daylight (Numeric values + "K")
    else if ( str[0] != 'd' && str[length - 1] == 'k' )
    {
        while ( str[i] && i < str.length() - 1 )
        {
            if ( !isdigit( str[i] ) )
                return false;
            i++;
        }
    }
    // It is rarely for "D" and "K" appear
    // together as the first letter and
    // the last letter of the string
    else if ( str[0] == 'd' && str[length - 1] == 'k' )
        return false;

    return true;
};

// Function to get environment variable for camera data
inline dataPath &pathsFinder()
{
    static dataPath cdp;
    static bool     firstTime = 1;

    if ( firstTime )
    {
        string      path;
        const char *env;

        vector<string> &PATHs = cdp.paths;
        env                   = getenv( "AMPAS_DATA_PATH" );

        if ( env )
            path = env;

        if ( path == "" )
        {
#if defined( WIN32 ) || defined( WIN64 )
            path   = ".";
            cdp.os = "WIN";
#else
            // clang-format off
            path   = "/usr/local/include/rawtoaces/data";
            cdp.os = "UNIX";
            // clang-format on
#endif
        }

        size_t pos = 0;

        while ( pos < path.size() )
        {
#if defined( WIN32 ) || defined( WIN64 )
            size_t end = path.find( ';', pos );
#else
            size_t end = path.find( ':', pos );
#endif

            if ( end == string::npos )
                end = path.size();

            string pathItem = path.substr( pos, end - pos );

            if ( find( PATHs.begin(), PATHs.end(), pathItem ) == PATHs.end() )
                PATHs.push_back( pathItem );

            pos = end + 1;
        }
    }
    return cdp;
};

#endif
