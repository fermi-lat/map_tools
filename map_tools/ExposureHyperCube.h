/** @file ExposureHyperCube.h
@brief declare class ExposureHyperCube 

@author Toby Burnett
$Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/ExposureHyperCube.h,v 1.9 2005/01/01 18:56:30 burnett Exp $

*/

#ifndef TOOLS_EXPOSUREHYPERCUBE_H
#define TOOLS_EXPOSUREHYPERCUBE_H
#include "Exposure.h"
namespace tip { class Image; }

namespace map_tools {


    /** @class ExposureHyperCube 
    @brief Set up an exposure map hypercube, wrapping an Exposure object
    as a multilayer FITS image

    It is defined as a hypercube in ra, dec, sqrt(1-costheta) bins.
    @todo: allow other binning function

    */
    class ExposureHyperCube  {
    public:
        //! ctor
        ExposureHyperCube( const Exposure& exp, std::string outfile, bool clobber=true);

        //! dtor updates the image file
        ~ExposureHyperCube();

        //! saves the image
        void save();
    private:
        //! pointer to the associated tip Image class
        tip::Image* m_image;
    };
}// namespace map_tools
#endif //TOOLS_EXPOSUREHYPERCUBE_H
