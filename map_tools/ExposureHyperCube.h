/** @file ExposureHyperCube.h
    @brief declare class ExposureHyperCube 

    @author Toby Burnett
    $Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/ExposureHyperCube.h,v 1.13 2005/03/04 15:33:50 burnett Exp $

*/

#ifndef TOOLS_EXPOSUREHYPERCUBE_H
#define TOOLS_EXPOSUREHYPERCUBE_H
#include "Exposure.h"
#include <iostream>

namespace map_tools {


/** @class ExposureHyperCube 
    @brief Set up an exposure map hypercube

    */
class ExposureHyperCube  {
public:
    ExposureHyperCube( const Exposure& exp, std::string outfile)
        :m_exposure(exp),m_outfile(outfile), m_written(false)
    {
    }
    void save()
    {
        if(!m_written) m_exposure.write(m_outfile);
        m_written=true;
    }

    ~ExposureHyperCube()
    {
        save();
    }
    const Exposure& m_exposure;
    std::string m_outfile;
    bool m_written;
};
}// namespace map_tools
#endif //TOOLS_EXPOSUREHYPERCUBE_H
