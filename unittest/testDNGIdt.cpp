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

#define BOOST_TEST_MAIN
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <boost/test/floating_point_comparison.hpp>

#include <rawtoaces/rta.h>

using namespace std;
using namespace rta;

BOOST_AUTO_TEST_CASE( TestIDT_CcttoMired )
{

    DNGIdt *di    = new DNGIdt();
    double  cct   = 6500.0;
    double  mired = di->ccttoMired( cct );
    delete di;

    BOOST_CHECK_CLOSE( mired, 153.8461538462, 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_RobertsonLength )
{

    DNGIdt        *di    = new DNGIdt();
    double         uv[]  = { 0.2042589852, 0.3196233991 };
    double         uvt[] = { 0.1800600000, 0.2635200000, -0.2434100000 };
    vector<double> uvVector( uv, uv + 2 );
    vector<double> uvtVector( uvt, uvt + 3 );

    double rLength = di->robertsonLength( uvVector, uvtVector );
    delete di;

    BOOST_CHECK_CLOSE( rLength, 0.060234937, 1e-5 );
};

DNGIdt *openFile( std::string path, LibRaw &rawProcessor )
{
    boost::filesystem::path pathToRaw = boost::filesystem::absolute(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng" );
    int ret = rawProcessor.open_file( ( pathToRaw.string() ).c_str() );
    ret     = rawProcessor.unpack();

#define R rawProcessor.imgdata.rawdata

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

    return new DNGIdt( metadata );
}

BOOST_AUTO_TEST_CASE( TestIDT_LightSourceToColorTemp )
{
    DNGIdt        *di  = new DNGIdt();
    unsigned short tag = 17;
    double         ct  = di->lightSourceToColorTemp( tag );
    delete di;

    BOOST_CHECK_CLOSE( ct, 2856.0, 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_XYZToColorTemperature )
{
    LibRaw  rawProcessor;
    DNGIdt *di = openFile(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng",
        rawProcessor );
    double         XYZ[3] = { 0.9731171910, 1.0174927152, 0.9498565880 };
    vector<double> XYZVector( XYZ, XYZ + 3 );
    double         cct = di->XYZToColorTemperature( XYZVector );

    rawProcessor.recycle();
    delete di;

    BOOST_CHECK_CLOSE( cct, 5564.6648479019, 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_XYZtoCameraWeightedMatrix )
{
    LibRaw  rawProcessor;
    DNGIdt *di = openFile(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng",
        rawProcessor );

    double         mirs[3] = { 158.8461538462, 350.1400560224, 153.8461538462 };
    double         matrix[9] = { 1.0165710542,  -0.2791973987, -0.0801820653,
                                 -0.4881171650, 1.3469051835,  0.1100471308,
                                 -0.0607157824, 0.3270949763,  0.5439419519 };
    vector<double> result =
        di->XYZtoCameraWeightedMatrix( mirs[0], mirs[1], mirs[2] );
    rawProcessor.recycle();
    delete di;

    FORI( countSize( matrix ) )
    BOOST_CHECK_CLOSE( result[i], matrix[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_FindXYZtoCameraMtx )
{
    LibRaw  rawProcessor;
    DNGIdt *di = openFile(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng",
        rawProcessor );

    double         neutralRGB[3] = { 0.6289999865, 1.0000000000, 0.7904000305 };
    double         matrix[9] = { 1.0616656923,  -0.3124143737, -0.0661770211,
                                 -0.4772957633, 1.3614785395,  0.1001599918,
                                 -0.0411839968, 0.3103035015,  0.5718121924 };
    vector<double> neutralRGBVector( neutralRGB, neutralRGB + 3 );
    vector<double> result = di->findXYZtoCameraMtx( neutralRGBVector );

    rawProcessor.recycle();
    delete di;

    FORI( countSize( matrix ) )
    BOOST_CHECK_CLOSE( result[i], matrix[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_ColorTemperatureToXYZ )
{

    DNGIdt        *di     = new DNGIdt();
    double         cct    = 6500.0;
    double         XYZ[3] = { 0.3135279229, 0.3235340821, 0.3629379950 };
    vector<double> result = di->colorTemperatureToXYZ( cct );
    delete di;

    FORI( countSize( XYZ ) )
    BOOST_CHECK_CLOSE( result[i], XYZ[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_MatrixRGBtoXYZ )
{

    DNGIdt        *di     = new DNGIdt();
    double         XYZ[9] = { 0.952552395938, 0.000000000000, 0.000093678632,
                              0.343966449765, 0.728166096613, -0.072132546379,
                              0.000000000000, 0.000000000000, 1.008825184352 };
    vector<double> result = di->matrixRGBtoXYZ( chromaticitiesACES );
    delete di;

    FORI( countSize( XYZ ) )
    BOOST_CHECK_CLOSE( result[i], XYZ[i], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_GetDNGCATMatrix )
{
    LibRaw  rawProcessor;
    DNGIdt *di = openFile(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng",
        rawProcessor );

    double matrix[3][3] = { { 0.9907763427, -0.0022862289, 0.0209908807 },
                            { -0.0017882434, 0.9941341374, 0.0083008330 },
                            { 0.0003777587, 0.0015609315, 1.1063201101 } };
    vector<vector<double>> result = di->getDNGCATMatrix();

    rawProcessor.recycle();
    delete di;

    FORIJ( 3, 3 )
    BOOST_CHECK_CLOSE( result[i][j], matrix[i][j], 1e-5 );
};

BOOST_AUTO_TEST_CASE( TestIDT_GetDNGIDTMatrix )
{
    LibRaw  rawProcessor;
    DNGIdt *di = openFile(
        "../../unittest/materials/blackmagic_cinema_camera_cinemadng.dng",
        rawProcessor );

    double matrix[3][3] = { { 1.0536466144, 0.0039044182, 0.0049084502 },
                            { -0.4899562165, 1.3614787986, 0.1020844728 },
                            { -0.0024498461, 0.0060497128, 1.0139159537 } };
    vector<vector<double>> result = di->getDNGIDTMatrix();

    rawProcessor.recycle();
    delete di;

    FORIJ( 3, 3 )
    BOOST_CHECK_CLOSE( result[i][j], matrix[i][j], 1e-5 );
};
