/** @file HealpixArrayIO.h
@brief Define the HealpixArrayIO singleton class 

@author B. Lesnick

$Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/CosineBinner.h,v 1.2 2005/02/06 19:52:23 burnett Exp $
*/

#ifndef map_tools_HealpixArrayIO_h
#define map_tools_HealpixArrayIO_h

#include "astro/HealPix.h"
#include "astro/HealpixArray.h"

#include "map_tools/CosineBinner.h"

#include <vector>


namespace map_tools
{

    /** @class HealpixArrayIO
    @brief Manage I/O of HealpixArray object to persistent storage

    */

    class HealpixArrayIO
    {
        public:
            static HealpixArrayIO & instance();
            void write(const astro::HealpixArray<CosineBinner> & ha,
                        const std::string & outputFile,
                        const std::string & tablename, bool clobber=true);
            astro::HealpixArray<CosineBinner> read(const std::string & inputFile,
                        const std::string & tablename);
                
        private:
            HealpixArrayIO(){}
    };
}
#endif
