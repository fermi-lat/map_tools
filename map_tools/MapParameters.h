/**
* @file MapParameters.h
* @brief Map Parameter Reader
*
* $Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/MapParameters.h,v 1.1.1.1 2004/02/21 21:47:26 burnett Exp $
*/

#ifndef MAPPARAMETERS_H
#define MAPPARAMETERS_H 

#include "Parameters.h"
#include <string>


/**
* @class MapParameters
* @brief Input reader class for count_map tool.
*
* This class provides methods to read user parameters needed for EventBin tool. 
* It uses PIL to read parameters from the par file.
* The description of PIL is available at
* href="http://www-glast.slac.stanford.edu/sciencetools/userInterface/doc/pil.pdf">PIL user
* manual</a>.
*
* $Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/MapParameters.h,v 1.1.1.1 2004/02/21 21:47:26 burnett Exp $
*/

class MapParameters : public Parameters
{
public:
    // Constructors
    MapParameters( int argc, char *argv[]);

    int npix() const                   { return m_npix; }
    int npixX() const                   { return m_npix; }
    int npixY() const                   { return m_npix_y; }
    int imgSize() const                { return m_imgSizeX; }
    int imgSizeX() const                { return m_imgSizeX; }
    int imgSizeY() const                { return m_imgSizeY; }
    float xref() const                 { return m_xref; }
    float yref() const                 { return m_yref; }
    float rot() const                  { return m_rot; }
    std::string projType() const        { return m_projType; }
    std::string raName() const          { return m_raName; }
    std::string decName() const         { return m_decName; }

    bool uselb()const           {return m_use_lb;}


private:

    int             m_npix, m_npix_y;
    int             m_imgSizeX, m_imgSizeY;
    float           m_xref;
    float           m_yref;
    float           m_rot;
    std::string     m_projType;
    bool            m_use_lb;
    std::string     m_raName, m_decName;

};

#endif
