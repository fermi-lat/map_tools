/**
* @file Parameters.cxx
* @brief Implementation for class that reads parameters needed for tools
* @author Toby Burnett
*
* $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/Parameters.cxx,v 1.12 2004/03/31 13:32:45 burnett Exp $
*/

#include <sstream>

#include "facilities/Util.h"

#include "map_tools/Parameters.h"

#include "hoops/hoops_prompt_group.h"

using namespace map_tools;


//! Constructor
Parameters::Parameters( int argc, char *argv[]) 
:  m_par(*new hoops::ParPromptGroup(argc, argv))
,  m_own_ppg(true)
{  
  // Prompt for all parameters in the order in the par file:
    dynamic_cast<hoops::ParPromptGroup&>(m_par).Prompt();

    setup();
}
//! Constructor
Parameters::Parameters( hoops::IParGroup & par) 
:  m_par(par), m_own_ppg(false)
{  
    setup();
}

Parameters::~Parameters(){
    if( m_own_ppg) delete &m_par;
}

void Parameters::setup()
{
    m_chatter = m_par["chatter"];

    m_clobber = m_par["clobber"];

    // Read name of the file containing events data and expand any
    // environment variables.
    std::string infile = m_par["infile"];
    m_inFile = infile;

    facilities::Util::expandEnvVar(&m_inFile);
#if 0
    std::string filter = m_par["filter"];
    m_filter = filter;
#endif
    std::string outfile = m_par["outfile"];
    m_outFile = outfile;
    facilities::Util::expandEnvVar(&m_outFile);

    if( m_clobber ) m_outFile= "!"+m_outFile;  // FITS convention to rewrite file

}


