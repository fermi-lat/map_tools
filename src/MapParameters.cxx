/** @file MapParameters.cxx
*   @brief Implementation for class that reads parameters for image description
* @author Toby Burnett 
*
* $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/MapParameters.cxx,v 1.2 2004/02/23 02:44:36 burnett Exp $
*/

#include "map_tools/MapParameters.h"

using namespace map_tools;

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

