/** @file SkyFunction.h

    @brief declare  the class SkyFunction
    @author Toby Burnett <tburnett@u.washington.edu>
    $Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/SkyFunction.h,v 1.1 2004/03/08 00:17:37 burnett Exp $

*/

#ifndef MAP_TOOLS_SKYFUNCTION_H
#define MAP_TOOLS_SKYFUNCTION_H

namespace astro { class SkyDir; }

namespace map_tools {

/**
    @class SkyFunction
    @brief abstract base class for 

*/
class SkyFunction 
{
public:

    //! @brief  coordinates of a point in the sky
    //! @return value at that point
    virtual double operator()(const astro::SkyDir& bincenter)const=0;
    virtual ~SkyFunction(){}
protected:    
    SkyFunction(){}    
};
} //namesace map_tools

#endif
