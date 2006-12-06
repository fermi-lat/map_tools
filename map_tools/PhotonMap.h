/** @file PhotonMap.h
@brief definition of class PhotonMap

$Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/PhotonMap.h,v 1.9 2006/06/26 21:11:46 burnett Exp $

*/
#ifndef map_tools_PhotonMap_h
#define map_tools_PhotonMap_h

#include "astro/Healpix.h"
#include "astro/HealPixel.h"
#include "astro/SkyFunction.h"

#include "astro/Photon.h"
// forward declaration
namespace tip{ class Table; }

#include <map>


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
        @param minlevel [6] HealPixel level of first bin

        */
        PhotonMap(double emin=100, double eratio=2.35, int nlevels=8, int minlevel=6);

        /// Create PhotonMap object from saved fits file
        PhotonMap(const std::string & inputFile, const std::string & tablename);

        ~PhotonMap();

        /// add a photon to the map with the given energy and direction
        void addPhoton(const astro::Photon& gamma);

        /// @return density for a given direction, in photons/area of the base pixel.
        double density (const astro::SkyDir & sd) const;

        //! Count the photons within a given pixel.
        double photonCount(const astro::HealPixel & px, bool includeChildren=false,
            bool weighted=false) const;

        //! Count the photons within a given pixel, weighted with children.  Also return weighted direction.
        double photonCount(const astro::HealPixel & px, astro::SkyDir & NewDir) const;

        ///  implement the SkyFunction class by returning density
        double operator()(const astro::SkyDir & sd) const{ return density(sd);}

        /// the binning function: return a HealPixel corresponding to the 
        /// direction and energy
        astro::HealPixel pixel(const astro::Photon& gamma);

        /** @brief extract a subset around a given direction. include selected pixels and all children.
        @param radius The maximum radius (deg). Set to >=180 for all
        @param vec the vector to fill with (healpixel, count ) pairs
        @param summary_level [-1] selection level: default is the minimum level 
        @param select_level [-1] set to return only pixels at this level
        @return the total number of photons (sum of count)
        */
        int extract(const astro::SkyDir& dir, double radius,
            std::vector<std::pair<astro::HealPixel, int> >& vec,
            int summary_level = -1, int select_level = -1) const;

        /** @brief extract a subset around a given direction.  single level only.
        @param radius The maximum radius (deg). Set to >=180 for all
        @param vec the vector to fill with (healpixel, count ) pairs
        @param select_level [-1] return only pixels at this level. default is the minimum level
        @param include_all [false] True: return all possible select_level pixels within radius.  False: return only pixels found in current PhotonMap.
        @return the total number of photons (sum of count)
        */
        int extract(const astro::SkyDir& dir, double radius,
            std::vector<std::pair<astro::HealPixel, int> >& vec,
            int select_level = -1, bool include_all = false) const;

        int photonCount()const { return m_photons;} ///< current number of photons
        int pixelCount()const { return m_pixels; } ///< current nubmer of pixesl

        int minLevel()const { return m_minlevel;} ///< minimum Healpixel level
        int levels()const {return m_levels;};  ///< number of energy bins

        /// @return a vector of the left edges of the energy bins
        std::vector<double> energyBins()const;


        /**@brief Write a PhotonMap object to a fits file
        @param outputFile Fully qualified fits output file name
        @param tablename Fits secondary extension name
        @param clobber Whether to delete an existing file first 
        @return an auto pointer to the TIP table used to generate the fits outFile
        */
        std::auto_ptr<tip::Table> PhotonMap::write(const std::string & outputFile,
            const std::string & tablename="PHOTONMAP",
            bool clobber = true) const;


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
