/**
* @file Parameters.cxx
* @brief Implementation for class that reads parameters needed for tools
* @author Toby Burnett
*
* $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/Parameters.cxx,v 1.16 2004/04/26 11:38:20 burnett Exp $
*/

#include <sstream>

#include "facilities/Util.h"

#include "map_tools/Parameters.h"
#include "hoops/hoops_prompt_group.h"

using  map_tools::Parameters;


//! Constructor
Parameters::Parameters( int argc, char *argv[]) 
:  m_par(*new hoops::ParPromptGroup(argc, argv))
,  m_own_ppg(true)
{  
     setup();
}
//! Constructor
Parameters::Parameters( hoops::IParGroup & par) 
:  m_par(par), m_own_ppg(false)
{  
    setup();
}

Parameters::~Parameters() throw() {
    if( m_own_ppg) delete &m_par;
}

void Parameters::setup()
{
      // Prompt for all parameters in the order in the par file:

  dynamic_cast<hoops::ParPromptGroup&>(m_par).Prompt();
   dynamic_cast<hoops::ParPromptGroup&>(m_par).Save();
    m_chatter = m_par["chatter"];

    m_clobber = m_par["clobber"];

    // Read name of the file containing events data and expand any
    // environment variables.
    std::string infile = m_par["infile"];
    m_inFile = infile;

    facilities::Util::expandEnvVar(&m_inFile);
    std::string filter = m_par["filter"];
    m_filter = filter;
    std::string outfile = m_par["outfile"];
    m_outFile = outfile;
    facilities::Util::expandEnvVar(&m_outFile);

    std::string table_name = m_par["table_name"];
    m_table_name = table_name;

    if( m_clobber ) m_outFile= "!"+m_outFile;  // FITS convention to rewrite file

}


