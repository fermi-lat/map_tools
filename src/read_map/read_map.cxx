/** @file read_map.cxx
    @brief build the read_map application

     @author Toby Burnett

     $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/read_map/read_map.cxx,v 1.6 2004/03/31 13:32:45 burnett Exp $
*/

#include "map_tools/SkyImage.h"
#include "map_tools/MapParameters.h"
#include "astro/SkyFunction.h"
#include "astro/SkyDir.h"
#include <iostream>

using namespace map_tools;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class Request 
    @brief function class requests a point from the image
*/
class Request : public astro::SkyFunction
{
public:
    Request(const SkyImage& image )
        : m_image(image)
    {}
        double operator()(const astro::SkyDir& s)const{
            return m_image.pixelValue(s);
        }
private:
    const SkyImage& m_image;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char * argv[]) {
    int rc = 0;
    try{ 
  
        // read in, or prompt for, all necessary parameters
        MapParameters pars(argc, argv);

        std::cout << "Reading FITS input file " << pars.inputFile() << std::endl;
        SkyImage image(pars.inputFile(), pars.tableName()); 

        std::cout << "Value of pixel at l, b: "<< pars["ra"]<< ", "<< pars["dec"] << ": "
        <<image.pixelValue(astro::SkyDir(pars["ra"],pars["dec"],astro::SkyDir::GALACTIC)) 
        << std::endl;

        std::cout << "Creating copy at file " << pars.outputFile() << std::endl;
        SkyImage copy(pars);
        copy.fill(Request(image));

    }catch( const std::exception& e){
        std::cerr << "caught exception: " << e.what() << std::endl;
        rc=1;
    }
    return rc;
}

/** @page read_map_guide read_map users's Guide

 A simple application that reads a value from a map, and copies the map to another file.
 Code is in read_map.cxx.

 @verbinclude read_map.par

*/
