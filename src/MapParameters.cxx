/**
* @file MapParameters.cxx
* @brief Implementation for class that reads parameters needed for EventBinning.
* @author Toby Burnett 
*
* $Header: /nfs/slac/g/glast/ground/cvs/users/burnett/map_tools/src/MapParameters.cxx,v 1.2 2004/02/20 19:11:43 burnett Exp $
*/

#include "map_tools/MapParameters.h"


//! Constructor
MapParameters::MapParameters( int argc, char *argv[])
: Parameters(argc, argv)
{

    // Read number of pixels
    m_npix = getInt("npix");
    m_npix_y= getInt("npixy", m_npix);

    // Read Image size
    m_imgSizeX= getInt( "imgsize" );
    m_imgSizeY= getInt( "imgsizey", m_imgSizeX);


    // Read xref
    m_xref = getDouble("xref");
    m_yref = getDouble("yref");

    m_rot = getDouble("rot");

    m_projType = getString("projtype");
    m_use_lb = getBool("uselb", false);

    m_raName= getString("ra_name", "ra");
    m_decName = getString("dec_name", "dec");

}

