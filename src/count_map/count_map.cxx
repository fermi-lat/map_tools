/** @file count_map.cxx
@brief build the count_map application

$Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/count_map/count_map.cxx,v 1.10 2004/04/05 17:04:45 burnett Exp $
*/

#include "map_tools/SkyImage.h"
#include "map_tools/MapParameters.h"

#include "tip/Table.h"
#include "tip/IFileSvc.h"
#include "astro/SkyDir.h"

#include "image/Image.h" 
#include "st_app/IApp.h"

#include <algorithm>
using namespace map_tools;

/** @class CountMap 
@brief The count_map application
*/
class CountMap : public st_app::IApp {
public:

    CountMap():st_app::IApp("count_map"){}

    void run(){
        using tip::Table;
        // read in, or prompt for, all necessary parameters
        MapParameters pars(hoopsGetParGroup());

        // connect to  input data, specifying filter
        const Table & table = *tip::IFileSvc::instance().readTable(pars.inputFile(), "", pars.filter() );
        if( pars.chatter()>0) {
            std::cout << "Reading file " << pars.inputFile() ;
            if( ! pars.filter().empty() ) std::cout << "\n\tfiltered by " << pars.filter() ;
            std::cout<< std::endl;
        }

        // create the image object
        SkyImage image(pars);

        for (Table::ConstIterator it = table.begin(); it != table.end(); ++it) {

            // Create local reference to the record to which the iterator refers:
            const Table::Record & record = *it;
            // Get the current values
            double ra = record[pars.raName()].get(), 
                dec=record[pars.decName()].get();

            image.addPoint(astro::SkyDir(ra, dec) );
        }
        if( pars.chatter()>0) {
            std::cout << "Total added to image: " << image.total() 
                <<" at file\n\t" << pars.outputFile() << std::endl; }
    }

} app;
