/** @file test_main.cxx
@brief test various classes

$Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/test/test_main.cxx,v 1.24 2005/01/01 22:27:22 burnett Exp $

*/
#include "map_tools/Exposure.h"
#include "map_tools/MapParameters.h"
#include "map_tools/ExposureHyperCube.h"
#include "map_tools/SkyImage.h"



#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cassert>
using namespace map_tools;

class TestAeff : public Exposure::Aeff {
public:
    TestAeff(double slope=0): m_slope(slope){}
    double operator()(double ct)const{
        //        std::cout << " theta = " << acos(ct)*180/M_PI << std::endl;
        return 1.-(1-ct)*m_slope;
    }
    double m_slope;
};




int main(int argc, char** argv ){
    try{
        // read a pil file--and make sure that a few simple things work
        MapParameters par(argc, argv);
        double xref = par["xref"] ;
        if(  xref !=0 ) {
            std::cerr << "Read wrong value for parameter xref" << std::endl;
            return 1;
        }

        std::cout << "Testing exposure calculation with binning function "
            << Exposure::Index::thetaBinning() << std::endl;
        Exposure e( 10,  0.1);
        double total=0;

        // make a quick uniform cube.
        for( double ra=0.5; ra<360; ra+=2.0) {
            for (double st = -0.95; st < 1.0; st += 0.05){
                double dec = asin(st)*180/M_PI;
                e.add( astro::SkyDir(ra, dec), 1.0);
                total += 1.0;
            }
        }

        double test = e(0,0, TestAeff()) / total;
        if ( fabs(test-0.5)> 0.01 ){
            std::cerr << "bad cosine integral: " << test << std::endl;
            return 1;
        }
        test = e(0,89, TestAeff()) / total;
        if ( fabs(test-0.5)> 0.01 ){
            std::cerr << "bad cosine integral: " << test << std::endl;
            return 1;
        }
        test = e(180, 89, TestAeff()) / total;
        if ( fabs(test-0.5)> 0.01 ){
            std::cerr << "bad cosine integral: " << test << std::endl;
            return 1;
        }
        test = e(0,0, TestAeff(1.0) ) /total;

        if ( fabs(test-0.25)> 0.01 ){
            std::cerr << "bad cosine integral: " << test << std::endl;
            return 1;
        }

        // Write out the cube...delete any existing file first.
        // THB: this does not delete files: std::remove(par.inputFile().c_str());
        ExposureHyperCube cube(e, par.inputFile());
        cube.save();

        // Check the Exposure(fitsfile) constructor.
        Exposure e2(par.inputFile());

        // Write this out as a separate file for an external diff.
        ExposureHyperCube cube2(e2, par.outputFile());
        cube2.save();

        // create an image to access cells
        SkyImage exp3(par.inputFile(),"");
        double tt = exp3.pixelValue(astro::SkyDir(0,0));
        assert(tt=36.0); 


    }catch( const std::exception& e){
        std::cerr << "Failed test because caught " <<typeid(e).name()<<" \""  
            << e.what() << "\"" << std::endl;
        return 1;
    }
    std::cout << "tests OK" << std::endl;
    return 0;
}

