/** @file exposure_cube.cxx
@brief build the exposure_cube application

@author Toby Burnett
$Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/exposure_cube/exposure_cube.cxx,v 1.12 2004/03/31 13:58:10 burnett Exp $
*/

#include "map_tools/MapParameters.h"
#include "map_tools/Exposure.h"
#include "map_tools/ExposureHyperCube.h"
#include "astro/SkyDir.h"
#include "astro/SkyDir.h"
#include "tip/Table.h"
#include "tip/IFileSvc.h"
#include "st_app/IApp.h"

#include <iostream>
using namespace map_tools;


class ExposureCubeApp : public st_app::IApp {
public:
    ExposureCubeApp() : st_app::IApp("exposure_cube"){}

    //--------------------------------------------------------------------------
    void LoadExposureFromGlast( const MapParameters& pars,   Exposure& exp )
    {
        using tip::Table;

        double 
            tstart = pars["tstart"], 
            tstop = pars["tstop"];

        // connect to  input data
        Table & table = *tip::IFileSvc::getSvc().editTable(pars.inputFile(), "Ext1");

        for (Table::Iterator it = table.begin(); it != table.end(); ++it) {

            const Table::Record & record = *it;

            double start, stop;
            record["start"].get(start);
            record["stop"].get(stop);

            if( start < tstart ) continue;
            if( stop > tstop ) break;

            double ra, dec, livetime;
            record["ra_scz"].get(ra);
            record["dec_scz"].get(dec);
            record["livetime"].get(livetime);

            double deltat = livetime > 0 ? livetime : stop-start;
            exp.add(astro::SkyDir(ra, dec), deltat); 
        }
    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void run()
    {
        // read in, or prompt for, all necessary parameters
        MapParameters pars( IApp::hoopsGetParGroup());

        // create the exposure, and fill it from the history file
        Exposure ex( pars["pixelsize"], pars["costhetabinsize"]);

        LoadExposureFromGlast(  pars, ex); 

        // create the fits output file from the Exposure file
        ExposureHyperCube cube(ex, pars.outputFile());

    }
}application;
