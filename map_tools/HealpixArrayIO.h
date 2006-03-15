/** @file HealpixArrayIO.h
@brief Define the HealpixArrayIO singleton class 

@author B. Lesnick

$Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/HealpixArrayIO.h,v 1.4 2005/06/22 17:57:19 burnett Exp $
*/

#ifndef map_tools_HealpixArrayIO_h
#define map_tools_HealpixArrayIO_h

#include "astro/Healpix.h"
#include "astro/HealpixArray.h"

#include "tip/Table.h"

#include "map_tools/CosineBinner.h"

#include <vector>
#include <memory>


namespace map_tools
{

    /** @class HealpixArrayIO (singleton)
    @brief Manage I/O of HealpixArray object to persistent storage
    
    For now, the object being input/output must be of the form HealpixArray<CosineBinner> 
    and the input/output file type is fits.

    */

    class HealpixArrayIO
    {
        public:
            ///@brief Create an instance to access this singleton class
            static HealpixArrayIO & instance();
            
            /**@brief Write a HealpixArray<CosineBinner> object to a fits file
            @param ha Object to be output
            @param outputFile Fully qualified fits output file name
            @param tablename Fits secondary extension name
            @param clobber Whether to delete an existing file first */
            std::auto_ptr<tip::Table> write(const astro::HealpixArray<CosineBinner> & ha,
                        const std::string & outputFile,
                        const std::string & tablename, bool clobber=true);
            ///@brief Write a HealpixArray<float> object to a fits file
            std::auto_ptr<tip::Table> write(const astro::HealpixArray<float> & ha,
                        const std::string & outputFile,
                        const std::string & tablename,
                        const std::string & fieldname, bool clobber=true);
            /**@brief Write a HealpixArray<vector<float> > object to a fits file
            This function is useful for storing a HealpixArray object which has an
            arbitrary number of floating point data elements associated with each pixel.
            One such example is the Wmap data, which has two floats per pixel.
            Note that the number of floats per record written to the file is determined
            by the number of elements passed in the fieldname argument.
            
            @param ha Object to be output
            @param outputFile Fully qualified fits output file name
            @param tablename Fits secondary extension name
            @param fieldname Vector of field names to be written to table.  
            @param clobber Whether to delete an existing file first */
            std::auto_ptr<tip::Table> write(const astro::HealpixArray<std::vector<float> > & ha,
                        const std::string & outputFile,
                        const std::string & tablename,
                        const std::vector<std::string> & fieldname,
                        bool clobber=true);
            
            ///@brief Read a HealpixArray<CosineBinner> object from a fits file
            astro::HealpixArray<CosineBinner> read(const std::string & inputFile,
                        const std::string & tablename);
            ///@brief Read a HealpixArray<float> object from a fits file
            astro::HealpixArray<float> read(const std::string & inputFile,
                        const std::string & tablename, const std::string & fieldname);
            /**@brief Read a HealpixArray<vector<float> > object from a fits file
            This function is useful for inputing a HealpixArray object which has an
            arbitrary number of floating point data elements associated with each pixel.
            One such example is the Wmap data, which has two floats per pixel.
            Note that the number of floats per record read from the file is determined
            by the number of elements passed in the fieldname argument.
            
            @param inputFile Fully qualified fits input file name
            @param tablename Fits secondary extension name
            @param fieldname Vector of field names to be read from the table.*/
            astro::HealpixArray<std::vector<float> > read(const std::string & inputFile,
                        const std::string & tablename,
                        const std::vector<std::string> & fieldname);
                
        private:
            ///@brief Private constructor, due to singleton
            HealpixArrayIO(){}
    };
}
#endif
