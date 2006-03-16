/** @file PhotonMap.h
@brief definition of class PhotonMap

$Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/PhotonMap.h,v 1.1 2006/03/16 04:43:42 burnett Exp $

*/
#ifndef map_tools_PhotonMap_h
#define map_tools_PhotonMap_h

#include "astro/HealPixel.h"
#include "astro/SkyFunction.h"
#include <map>


#include "astro/HealPixel.h"
#include "astro/Photon.h"


namespace map_tools {


#include <map>

/** @class PhotonMap
    @brief represent a set of photons as a map of variable-size pixels

    Each bin is indexed by a HealPixel object, which has the size and the

*/

class PhotonMap : public std::map<astro::HealPixel, unsigned int>,
    public astro::SkyFunction 
{
public:

    /** @brief ctor defines energy binning
        @param emin [100] Minimum energy
        @param eratio [2.35] ratio between bins
        @param nlevels [8] number of levels
        @param minlevel [5] HealPixel level of first bin

    */
    PhotonMap(double emin=100, double eratio=2.35, int nlevels=8, int minlevel=6);
    ~PhotonMap();

    /// add a photon to the map with the given energy and direction
    void addPhoton(const astro::Photon& gamma);

    /// @return density for a given direction, in photons/area of the base pixel.
    double density (const astro::SkyDir & sd) const;

    ///  implement the SkyFunction class by returning density
    double operator()(const astro::SkyDir & sd) const{ return density(sd);}

    /// the binning function: return a HealPixel corresponding to the 
    /// direction and energy
    astro::HealPixel pixel(const astro::Photon& gamma);


    /** @brief extract a subset around a given direction
        @param radius The maximum radius (deg)
        @param vec the vector to fill with (count, distance(rad) ) pairs
        @return the total number of photons (sum of count)
    */
    int extract(int level,const astro::SkyDir& dir, double radius, std::vector<std::pair<int, float> >& vec)const;
    
   
    int photonCount()const { return m_photons;}
    int pixelCount()const { return m_pixels; }

private:
    double m_emin;     ///< minimum energy for first bin
    double m_logeratio;   ///< log10(ratio between energy bins)
    int    m_levels;   ///< number of levels to create
    int    m_minlevel; ///< level number for first pixel 
    int    m_photons;  ///< total number of photons added
    int    m_pixels;   ///< keep track of total number of pixels
};


}// namespace

#endif
