/** @file Exposure.cxx
    @brief Implementation of class Exposure

   $Header: /nfs/slac/g/glast/ground/cvs/ScienceTools-scons/map_tools/src/Exposure.cxx,v 1.37 2009/05/20 00:40:14 burnett Exp $
*/
#include "map_tools/Exposure.h"
#include "healpix/HealpixArrayIO.h"
#include "tip/Table.h"
#include "astro/EarthCoordinate.h"
#include "astro/PointingTransform.h"

#include <memory>
#include <algorithm>
#include <exception>
#include <stdexcept>
#include <cstdlib>
#include <string>

#ifdef _OPENMP
#include <omp.h>
#endif

using namespace map_tools;
using healpix::HealpixArrayIO;
using healpix::CosineBinner;
using healpix::Healpix;


Exposure::Exposure(const std::string& inputfile, const std::string& tablename)
: SkyExposure(SkyBinner(2))
, m_nthreads(1)
{
   setData( HealpixArrayIO::instance().read(inputfile, tablename));
}

/// return the closest power of 2 for the side parameter
/// 41252 square degrees for the sphere
/// nside=64 is about one degee: 49152 pixels
inline int side_from_degrees(double pixelsize){ 
    int n = 1;
    while( 12*n*n < 41252/(pixelsize*pixelsize) && n < 512){
        n *=2;
    }
    return n; 
} 

Exposure::Exposure(double pixelsize, double cosbinsize, double zcut, bool weighted, double zmaxcut)
: SkyExposure(
    SkyBinner(Healpix(
      side_from_degrees(pixelsize),  // nside
      Healpix::NESTED, 
      astro::SkyDir::EQUATORIAL) )
  )
, m_zcut(zcut), m_zmaxcut(zmaxcut), m_lost(0)
, m_weighted(weighted)
, m_nthreads(1)
{
    unsigned int cosbins = static_cast<unsigned int>(1./cosbinsize);
    if( cosbins != CosineBinner::nbins() ) {
        // bins per pixel: depends on phi or not
        unsigned int allbins(cosbins);
        if( CosineBinner::nphibins() > 0 ){
            // add size for extra phi bins.
            allbins += cosbins * healpix::CosineBinner::nphibins();
        }
        
        SkyBinner::iterator is = data().begin();
        for( ; is != data().end(); ++is){ // loop over all pixels
            CosineBinner & pixeldata= *is; // get the contents of this pixel
            pixeldata.resize(allbins);
            CLHEP::Hep3Vector pixdir = data().dir(is)();
        }
        CosineBinner::setBinning(0, cosbins);
    }
    create_cache();
}

void Exposure::create_cache()
{
    size_t datasize(data().size());
    m_dir_cache.reserve(datasize);

    SkyBinner::iterator is = data().begin();
    for( ; is != data().end(); ++is){ // loop over all pixels
        Simple3Vector pixdir(data().dir(is)());
        m_dir_cache.push_back(std::make_pair(&*is, pixdir));
    }
}

/** @class Filler
    @brief private helper class used in for_each to fill a CosineBinner object
*/
class Exposure::Filler {
public:

    /** @brief ctor
        @param deltat time to add
        @param dirz direction to use to determine angle (presumably the spacecraft z-axis)
        @param dirx direction of x-axis
        @param zenith optional zenith direction for potential cut
        @param zcut optional cut: if -1, ignore
    */
   Filler( double deltat, const astro::SkyDir& dirz, const astro::SkyDir& dirx, astro::SkyDir zenith=astro::SkyDir(), double zcut=-1, double zmaxcut=1)
        : m_dirz(dirz())
        , m_rot(astro::PointingTransform(dirz,dirx).localToCelestial().inverse())
        , m_zenith(zenith())
        , m_deltat(deltat)
        , m_zcut(zcut)
        , m_zmaxcut(zmaxcut)
        , m_total(0), m_lost(0)
        , m_use_phi(CosineBinner::nphibins()>0)
    {}
   Filler( double deltat, const astro::SkyDir& dirz, astro::SkyDir zenith=astro::SkyDir(), double zcut=-1, double zmaxcut=1)
        : m_dirz(dirz())
        , m_zenith(zenith())
        , m_deltat(deltat)
        , m_zcut(zcut)
        , m_zmaxcut(zmaxcut)
        , m_total(0), m_lost(0)
        , m_use_phi(false)
    {}
    void operator()( const std::pair<CosineBinner*, Simple3Vector> & x)
    {
        // check if we are making a horizon cut:
        bool ok( m_zcut==-1 && m_zmaxcut==1);
        if( ! ok) {
            double z(x.second.dot(m_zenith));
            ok = z > m_zcut && z < m_zmaxcut;
        }
        if( ok) {
            // if ok, add to the angle histogram
            const Simple3Vector& pixeldir(x.second);
            if( m_use_phi) {
                CLHEP::Hep3Vector instrument_dir( pixeldir.transform(m_rot) );
                 double costheta(instrument_dir.z()), phi(instrument_dir.phi());
                x.first->fill( costheta, phi , m_deltat);
            }else{
                x.first->fill( pixeldir.dot(m_dirz), m_deltat);
            }

            m_total += m_deltat;
        }else{
            m_lost += m_deltat;
        }
    }
    double total()const{return m_total;}
    double lost()const{return m_lost;}
    double deltat()const{return m_deltat;}

    /** @brief The same test and binning as operator(), for fill_block:
        add m_deltat to b and, if wb isn't null, wdeltat to wb, using one
        geometry calculation for both. Returns false (nothing added) if
        the pixel fails the zenith cut. Doesn't touch m_total/m_lost, so
        one Filler can be shared by several threads.
    */
    bool fill_pixel(const Simple3Vector& pixeldir, CosineBinner& b,
                    CosineBinner* wb, double wdeltat) const
    {
        bool ok( m_zcut==-1 && m_zmaxcut==1);
        if( ! ok) {
            double z(pixeldir.dot(m_zenith));
            ok = z > m_zcut && z < m_zmaxcut;
        }
        if( ! ok) return false;
        if( m_use_phi) {
            CLHEP::Hep3Vector instrument_dir( pixeldir.transform(m_rot) );
            double costheta(instrument_dir.z()), phi(instrument_dir.phi());
            b.fill( costheta, phi, m_deltat);
            if( wb) wb->fill( costheta, phi, wdeltat);
        }else{
            double costheta(pixeldir.dot(m_dirz));
            b.fill( costheta, m_deltat);
            if( wb) wb->fill( costheta, wdeltat);
        }
        return true;
    }
private:
    Simple3Vector m_dirz;
    CLHEP::HepRotation m_rot;
    Simple3Vector m_zenith;
   double m_deltat, m_zcut, m_zmaxcut;
    mutable double m_total, m_lost;
    bool m_use_phi;
};


namespace {
#ifdef _OPENMP
    /** @brief The schedule of the threaded pixel loops, for benchmarking.

        MAP_TOOLS_EXPOSURE_SCHEDULE selects it: "static" (the default,
        also when the variable is unset or not understood), "dynamic,N"
        or "guided,N" (N optional). Read once. The schedule can't change
        the cubes: each pixel is still binned by one thread, rows in
        order. See doc/proposals/ltcube-pixel-threading.md (hybrid CPUs).
    */
    void loop_schedule(omp_sched_t & kind, int & chunk) {
        static omp_sched_t s_kind(omp_sched_static);
        static int s_chunk(0);
        static bool s_read(false);
#pragma omp critical(exposure_schedule)
        if (!s_read) {
            const char * env(std::getenv("MAP_TOOLS_EXPOSURE_SCHEDULE"));
            if (env) {
                std::string v(env);
                std::string::size_type comma(v.find(','));
                std::string name(v.substr(0, comma));
                if (name == "dynamic") s_kind = omp_sched_dynamic;
                else if (name == "guided") s_kind = omp_sched_guided;
                if (comma != std::string::npos) {
                    s_chunk = std::atoi(v.c_str() + comma + 1);
                }
            }
            s_read = true;
        }
        kind = s_kind;
        chunk = s_chunk;
    }

    /// Sets the calling thread's runtime schedule (used by the
    /// schedule(runtime) loops below) and restores it on destruction, so
    /// other OpenMP code in the process isn't affected.
    class ScheduleGuard {
    public:
        ScheduleGuard() {
            omp_get_schedule(&m_kind, &m_chunk);
            omp_sched_t kind;
            int chunk;
            loop_schedule(kind, chunk);
            omp_set_schedule(kind, chunk);
        }
        ~ScheduleGuard() { omp_set_schedule(m_kind, m_chunk); }
    private:
        omp_sched_t m_kind;
        int m_chunk;
    };
#endif

    /** @brief Apply one row's Filler to every pixel, splitting the pixels
        across OpenMP threads.

        Each thread works on its own copy of the Filler (the row's
        geometry is read-only; the copy holds that thread's total/lost),
        and each pixel's CosineBinner is touched by exactly one thread, so
        no locking is needed. Every bin gets the same additions in the
        same order as the serial loop, so the cube is bit-identical for
        any thread count. Only total and lost, sums over pixels, can
        differ in the last bits.

        With nthreads <= 1 this is the original serial loop and the OpenMP
        region is skipped: running the region on one thread made the
        serial run 7-10% slower on the cheap (phibins=0) configurations,
        while this path times the same as a build without OpenMP.
    */
    template <class Cache, class F>
    void parallel_fill(Cache & cache, const F & proto, int nthreads,
                       double & total, double & lost)
    {
        if (nthreads <= 1) {
            F filler = std::for_each(cache.begin(), cache.end(), proto);
            total = filler.total();
            lost = filler.lost();
            return;
        }
        const long npix(static_cast<long>(cache.size()));
        double my_total(0), my_lost(0);
        std::exception_ptr error;
#ifdef _OPENMP
        ScheduleGuard schedule;
#endif
#pragma omp parallel num_threads(nthreads) \
                     reduction(+:my_total, my_lost)
        {
            F filler(proto);
#pragma omp for schedule(runtime)
            for (long i = 0; i < npix; ++i) {
                try {
                    filler(cache[i]);
                } catch (...) {
                    // An exception must not leave the parallel region
                    // (that calls std::terminate). Keep the first one and
                    // rethrow it on the calling thread.
#pragma omp critical(exposure_fill_error)
                    if (!error) error = std::current_exception();
                }
            }
            my_total += filler.total();
            my_lost += filler.lost();
        }
        if (error) std::rethrow_exception(error);
        total = my_total;
        lost = my_lost;
    }
}

int Exposure::numThreads()const
{
#ifdef _OPENMP
    if (m_nthreads <= 0) return omp_get_max_threads();
    return m_nthreads;
#else
    return 1;
#endif
}

void Exposure::fill(const astro::SkyDir& dirz, double deltat)
{
    // As before, the total is deltat whatever the Filler adds up.
    double total, lost;
    parallel_fill(m_dir_cache, Filler(deltat, dirz), numThreads(), total, lost);
    addtotal(deltat);
}


void Exposure::fill(const astro::SkyDir& dirz, const astro::SkyDir& zenith, double deltat)
{
    double total, lost;
    parallel_fill(m_dir_cache, Filler(deltat, dirz, zenith, m_zcut, m_zmaxcut),
                  numThreads(), total, lost);
    addtotal(total);
    m_lost += lost;
}

void Exposure::fill_zenith(const astro::SkyDir& dirz,const astro::SkyDir& dirx, 
                           const astro::SkyDir& zenith, 
                           double deltat)
{
    double total, lost;
    parallel_fill(m_dir_cache, Filler(deltat, dirz, dirx, zenith, m_zcut, m_zmaxcut),
                  numThreads(), total, lost);
    addtotal(total);
    m_lost += lost;
}


namespace {
    /// Running totals for fill_block, per cube.
    struct BlockSums {
        double total, lost, wtotal, wlost;
        BlockSums() : total(0), lost(0), wtotal(0), wlost(0) {}
    };

    /// Apply every row of a block, in order, to one pixel of both cubes
    /// (x is the pixel's m_dir_cache entry, wb its weighted bins or 0).
    /// A template for the same reason as parallel_fill: the types are
    /// private to Exposure.
    template <class FillerVec, class Pixel>
    inline void fill_block_pixel(const FillerVec& fillers,
                                 const std::vector<double>& wdeltat,
                                 const Pixel& x, CosineBinner* wb,
                                 BlockSums& sums)
    {
        const size_t nrows(fillers.size());
        for (size_t k = 0; k < nrows; ++k) {
            if (fillers[k].fill_pixel(x.second, *x.first, wb, wdeltat[k])) {
                sums.total += fillers[k].deltat();
                sums.wtotal += wdeltat[k];
            } else {
                sums.lost += fillers[k].deltat();
                sums.wlost += wdeltat[k];
            }
        }
    }
}

void Exposure::fill_block(const std::vector<Interval>& rows, bool use_phi,
                          Exposure* weighted)
{
    if (rows.empty()) return;
    if (weighted && (weighted->m_dir_cache.size() != m_dir_cache.size()
                     || weighted->m_zcut != m_zcut
                     || weighted->m_zmaxcut != m_zmaxcut)) {
        throw std::invalid_argument("Exposure::fill_block: the weighted cube "
                                    "doesn't match this one's binning and cuts");
    }
    // Serial pre-pass: the row geometry (astro/CLHEP work) once per row,
    // built exactly as fill_zenith() and fill(dirz, zenith, deltat) build
    // it, so every value binned is the same.
    std::vector<Filler> fillers;
    std::vector<double> wdeltat;
    fillers.reserve(rows.size());
    wdeltat.reserve(rows.size());
    for (std::vector<Interval>::const_iterator r = rows.begin();
         r != rows.end(); ++r) {
        if (use_phi) {
            fillers.push_back(Filler(r->deltat, r->dirz, r->dirx, r->zenith,
                                     m_zcut, m_zmaxcut));
        } else {
            fillers.push_back(Filler(r->deltat, r->dirz, r->zenith,
                                     m_zcut, m_zmaxcut));
        }
        wdeltat.push_back(r->weighted_deltat);
    }

    const long npix(static_cast<long>(m_dir_cache.size()));
    const int nthreads(numThreads());
    BlockSums sums;
    if (nthreads <= 1) {
        // No OpenMP region on one thread (see parallel_fill).
        for (long i = 0; i < npix; ++i) {
            fill_block_pixel(fillers, wdeltat, m_dir_cache[i],
                             weighted ? weighted->m_dir_cache[i].first : 0,
                             sums);
        }
    } else {
        double total(0), lost(0), wtotal(0), wlost(0);
        std::exception_ptr error;
#ifdef _OPENMP
        ScheduleGuard schedule;
#endif
#pragma omp parallel num_threads(nthreads) \
                     reduction(+:total, lost, wtotal, wlost)
        {
            BlockSums my;
#pragma omp for schedule(runtime)
            for (long i = 0; i < npix; ++i) {
                try {
                    fill_block_pixel(fillers, wdeltat, m_dir_cache[i],
                                     weighted ? weighted->m_dir_cache[i].first : 0,
                                     my);
                } catch (...) {
                    // See parallel_fill.
#pragma omp critical(exposure_fill_error)
                    if (!error) error = std::current_exception();
                }
            }
            total += my.total;
            lost += my.lost;
            wtotal += my.wtotal;
            wlost += my.wlost;
        }
        if (error) std::rethrow_exception(error);
        sums.total = total;
        sums.lost = lost;
        sums.wtotal = wtotal;
        sums.wlost = wlost;
    }
    addtotal(sums.total);
    m_lost += sums.lost;
    if (weighted) {
        weighted->addtotal(sums.wtotal);
        weighted->m_lost += sums.wlost;
    }
}

void Exposure::write(const std::string& outputfile, const std::string& tablename)const
{
    healpix::HealpixArrayIO::instance().write(data(), outputfile, tablename);
}

void Exposure::load(const tip::Table * scData, 
                    const GTIvector& gti, 
                    bool verbose) {
   
   tip::Table::ConstIterator it = scData->begin();
   const tip::ConstTableRecord & row = *it;
   long nrows = scData->getNumRecords();

   for (long irow = 0; it != scData->end(); ++it, ++irow) {
      if (verbose && (irow % (nrows/20)) == 0 ) std::cerr << ".";
      if( processEntry( row, gti) )break;
   }
   if (verbose) std::cerr << "!" << std::endl;
}


bool Exposure::processEntry(const tip::ConstTableRecord & row, const GTIvector& gti)
{
    using astro::SkyDir;

    double  start, stop, livetime; 
    row["livetime"].get(livetime);
    if(livetime==0 ) return false; // assume this takes care of any entries during SAA
    row["start"].get(start);
    row["stop"].get(stop);
    double deltat = livetime; 


    double fraction(1); 
    bool  done(false);
    if( !gti.empty() ) {
        fraction = 0;

        GTIvector::const_iterator it  = gti.begin();
        for ( ; it != gti.end(); ++it) {
            double first = it->first,
                second=it->second;

            if( start < first ) {
                if( stop < first) continue; // history interval before gti
                if( stop <= second){
                    fraction = (stop-first)/(stop-start); // overlap start of gti
                    break;
                }
                fraction = (second-first)/(stop-start); // gti subset of history
                break;
            }else {
                if( start > second) continue; // interval after gti 
                if( stop <= second ) {
                    fraction = 1.0; break;  // fully contained
                }
                fraction = (second-start)/(stop-start);  // overlap end of gti
                break;
            }
 
        }
        done = fraction==0 && start > gti.back().second; 
    }
    if( fraction>0. ) {
        deltat *= fraction; // reduce if a boundary
        double ra, dec, razenith, deczenith;
        row["ra_scz"].get(ra);
        row["dec_scz"].get(dec);
        SkyDir scz(ra, dec);
        row["ra_scx"].get(ra);
        row["dec_scx"].get(dec);
        SkyDir scx(ra, dec);
	row["ra_zenith"].get(razenith);
	row["dec_zenith"].get(deczenith);
        SkyDir zenith(razenith, deczenith);
        if( m_weighted ){
            // adjust time by multiplying by livetime fraction
            deltat *= livetime/(stop-start);
        }
        fill_zenith(scz, scx,  zenith, deltat);
    }
    return done; 

}

