/**
* @file Parameters.cxx
* @brief Implementation for class that reads parameters needed for tools
* @author Toby Burnett
*
* $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/Parameters.cxx,v 1.7 2004/03/08 22:58:30 burnett Exp $
*/

#include <sstream>

#include "facilities/Util.h"

#include "map_tools/Parameters.h"


using namespace map_tools;


//! Constructor
Parameters::Parameters( int argc, char *argv[]) 
:  hoops::ParPromptGroup(argc, argv)
{   
// Prompt for all parameters in the order in the par file:
    Prompt();
    m_chatter = (*this)["chatter"];

    m_clobber = (*this)["clobber"];

    // Read name of the file containing events data and expand any
    // environment variables.
    m_inFile = std::string( (*this)["infile"] );
    facilities::Util::expandEnvVar(&m_inFile);

  //  m_filter = std::string( (*this)["filter"]);

    m_outFile =std::string( (*this)["outfile"]);
    facilities::Util::expandEnvVar(&m_outFile);

    if( m_clobber ) m_outFile= "!"+m_outFile;  // FITS convention to rewrite file

}

