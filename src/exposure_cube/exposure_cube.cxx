/** @file exposure_cube.cxx
    @brief build the exposure_cube application

    @author Toby Burnett
    $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/exposure_cube/exposure_cube.cxx,v 1.3 2004/02/28 14:20:22 burnett Exp $
*/
#include "tuple/ITable.h"

#include "map_tools/MapParameters.h"
#include "map_tools/Exposure.h"
#include "map_tools/ExposureHyperCube.h"

#include <iostream>

namespace ecube{
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class ExposureMapParameters : public MapParameters 
{
public:
    ExposureMapParameters(int argc, char* argv[])
        : MapParameters(argc, argv)
        {
            getDouble("costhetabinsize", 0.025); // 40 bins in sqrt(1-cos(thetat))
            getDouble("skybinsize", 1.0); // 1 degree bins in ra and dec
            getDouble("tstart");
            getDouble("tstop");
        }
    };
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int main(int argc, char * argv[]) {
    using namespace ecube;
    try{

        // read in, or prompt for, all necessary parameters
        ExposureMapParameters pars(argc, argv);

        // create the exposure, and fill it from the history file
        Exposure ex( pars["skybinsize"], pars["costhetabinsize"]);

        tuple::ITable::Factory& factory = *tuple::ITable::Factory::instance();

        ex.load( *factory(pars.eventFile(),"Ext1"), pars["tstart"] , pars["tstop"]); 

        // create the fits output file from the Exposure file
        ExposureHyperCube cube(ex, pars.outputFile());

    }catch( const std::exception& e){
        std::cerr << "Error: ending with exception: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
