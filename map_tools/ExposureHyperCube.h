/** @file ExposureHyperCube.h
    @brief declare class ExposureHyperCube 

    @author Toby Burnett
    $Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/ExposureHyperCube.h,v 1.4 2004/02/28 21:04:54 burnett Exp $

*/

#ifndef TOOLS_EXPOSUREHYPERCUBE_H
#define TOOLS_EXPOSUREHYPERCUBE_H

//#include "image/Image.h"
#include "Exposure.h"

/** @class ExposureHyperCube 
    @brief Set up an exposure map hypercube

    It is defined as a hypercube in ra, dec, sqrt(1-costheta) bins.

    */
class ExposureHyperCube  {
public:
    ExposureHyperCube( const Exposure& exp, std::string outfile);
   
   //! @brief add a string or douuble key to the image 
   void setKey(std::string name, double value, std::string unit="", 
               std::string comment="");

   void setKey(std::string name, std::string value,
               std::string unit="", std::string comment="");

   void save(std::string outfile="");

   ~ExposureHyperCube();

private:
//    FloatImg* m_image;
   void * m_image;

};
#endif //TOOLS_EXPOSUREHYPERCUBE_H
