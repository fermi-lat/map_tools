/**
* @file Parameters.cxx
* @brief Implementation for class that reads parameters needed for tools
* @author Toby Burnett
*
* $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/Parameters.cxx,v 1.6 2004/03/02 17:46:54 jchiang Exp $
*/

#include <sstream>

#include "facilities/Util.h"

#include "map_tools/Parameters.h"


using namespace map_tools;


//! Constructor
Parameters::Parameters( int argc, char *argv[]) : hoopsUtil::ParametersBase(argc, argv)
{
    m_chatter = getValue<long>("chatter");

    m_clobber = getValue<bool>("clobber");

    // Read name of the file containing events data and expand any
    // environment variables.
    m_inFile = getValue<std::string>("infile");
    facilities::Util::expandEnvVar(&m_inFile);

    m_filter = getValue<std::string>("filter", "");

    m_outFile = getValue<std::string>("outfile");
    facilities::Util::expandEnvVar(&m_outFile);

    if( m_clobber ) m_outFile= "!"+m_outFile;  // FITS convention to rewrite file

}

