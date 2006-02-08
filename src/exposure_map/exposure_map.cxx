/** @file exposure_map.cxx
@brief Classes specific to the exposure_map application

@author Toby Burnett

See the <a href="exposure_map_guide.html"> user's guide </a>.

$Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/exposure_map/exposure_map.cxx,v 1.23 2006/02/01 19:43:56 peachey Exp $
*/

#include "map_tools/SkyImage.h"
#include "map_tools/Exposure.h"

#include "irfInterface/IAeff.h"
#include "irfInterface/Irfs.h"
#include "irfInterface/IrfsFactory.h"
#include "irfLoader/Loader.h"

#include "st_app/StApp.h"
#include "st_app/StAppFactory.h"
#include "st_app/AppParGroup.h"
#include "st_stream/StreamFormatter.h"
#include "st_stream/st_stream.h"



#include "astro/SkyDir.h"

#include <stdexcept>
using namespace map_tools;

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class IrfAeff
@brief function class implements effective area, as adapter to irfInterface::IAeff
*/
class IrfAeff { //: public Exposure::Aeff{
public:
    /**
    @param aeff an object from the CALDB stuff. If zero, implement linear 
    @param energy energy to evaluate
    @param cutoff limit for cos(theta)
    */
    IrfAeff(const irfInterface::IAeff* aeff, double energy, double cutoff=0.25)
        :m_aeff(aeff),m_energy(energy), m_cutoff(cutoff)
    {}

    double operator()(double costh) const
    {
        if( m_aeff==0 ){
            return costh<m_cutoff? 0 : (costh-m_cutoff)/(1.-m_cutoff);
        }

        return m_aeff->value(m_energy, acos(costh)*180/M_PI, 0);
    }
    const irfInterface::IAeff* m_aeff;
    double m_energy;
    double m_cutoff;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class RequestExposure 
@brief function class requests a point from the exposure
*/
template< class F>
class RequestExposure : public astro::SkyFunction
{
public:
    RequestExposure(const Exposure& exp, const F& aeff, double norm=1.0)
        : m_exp(exp)
        , m_aeff(aeff)
        , m_norm(norm)
    {}
    double operator()(const astro::SkyDir& s)const{
        return m_norm*m_exp(s, m_aeff);
    }
private:
    const Exposure& m_exp;
    const F& m_aeff;
    double m_norm;
};

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/** @class ExposureMapApp
@brief the exposure_map application class

*/
class ExposureMapApp : public  st_app::StApp  {
public:
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /** \brief Create an application object, performing initializations needed for running the application.
    */
    ExposureMapApp()
        : st_app::StApp()
        , m_f("ExposureMapApp", "", 2)
        , m_pars(st_app::StApp::getParGroup("exposure_map")) 
    {
    }
        ~ExposureMapApp() throw() {} // required by StApp with gcc

    /**
    */


    irfInterface::IAeff* findAeff(std::string rspfunc)
    {
        using namespace irfInterface;
        m_f.setMethod("findAeff");

        class AeffSum : public irfInterface::IAeff {
        public:  
            AeffSum(const std::vector<std::string>& irflist){
                for(std::vector<std::string>::const_iterator sit= irflist.begin(); sit!=irflist.end(); ++sit){
                    irfInterface::Irfs* irf=IrfsFactory::instance()->create(*sit);
                    m_aeff.push_back(irf->aeff());
                }
            }

            virtual double value(double /*energy*/, 
                const astro::SkyDir & /*srcDir*/, 
                const astro::SkyDir &/*scZAxis*/,
                const astro::SkyDir &/*scXAxis*/) const 
            {return 0;}

            virtual double value(double energy, double theta, double phi) const 
            {
                // should use accumulate here
                double value = m_aeff[0]->value(energy, theta, phi);
                for( unsigned int i = 1; i < m_aeff.size(); ++i){
                    value+=m_aeff[i]->value(energy, theta, phi);
                }
                return value;
            }
            virtual IAeff * clone(){throw std::runtime_error("clone?"); return 0;};

        private:
            std::vector<const irfInterface::IAeff*> m_aeff;
        };


        // set up irf stuff, and translate the IRF name        

        irfLoader::Loader::go();

        m_f.info() << "Using Aeff(s) " ;

        if( rspfunc=="SIMPLE") {
            m_f.info() << "Simple linear form " << std::endl;
            return 0;
        }
        std::map<std::string, std::vector<std::string> > idMap = irfLoader::Loader::respIds();
        std::vector<std::string> irf_list = idMap[rspfunc];
        if( irf_list.empty()) {
            throw std::invalid_argument(
                std::string("Response function not recognized: "+rspfunc));
        }

        // this could be a simple copy with an ostream_iterator, but the ST interface does not allow it :-(
        for(std::vector<std::string>::const_iterator sit= irf_list.begin(); sit!=irf_list.end(); ++sit){
            m_f.info() << *sit<< ", ";
        }
        m_f.info() << std::endl;
    
        return new AeffSum(irf_list );

    }
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    void run() {

        m_f.setMethod("run()");

        prompt();

        // create the exposure, read it in from the FITS input file
        m_f.info() << "Creating an Exposure object from file " << m_pars["infile"].Value() << std::endl;

        ///todo: create from file, usnig HealpixArrayIO.
        std::string in_file = m_pars["infile"];
        std::string table = m_pars["table"];
        Exposure ex(in_file, table);

        double total_elaspsed = ex.total();
        m_f.info() << "\ttotal elapsed time: " << total_elaspsed << std::endl;

        irfInterface::IAeff* aeff = findAeff(m_pars["resptype"]);

        // create the image object, fill it from the exposure, write out
        std::clog << "Creating an Image, will write to file " << m_pars["outfile"].Value() << std::endl;
        SkyImage image(m_pars); 
        std::vector<double> energy;
        image.getEnergies(energy);
        for ( std::vector<double>::size_type layer = 0; layer != energy.size(); ++layer){
            double norm = aeff!=0? aeff->value(energy[layer],0,0): 1.0; // for normalization
            std::clog << "Generating layer " << layer
                << " at energy " << energy[layer] << " MeV " 
                << " Aeff(0): " << norm << " cm^2"<< std::endl;

            RequestExposure<IrfAeff> req(ex, IrfAeff(aeff, energy[layer]), 1.); 
            image.fill(req, layer);
        }
    }

    void prompt() {
        m_pars.Prompt("infile");
        m_pars.Prompt("cmfile");
        m_pars.Prompt("outfile");
        m_pars.Prompt("resptype");
    
        std::string uc_cm_file = m_pars["cmfile"];
        for (std::string::iterator itor = uc_cm_file.begin(); itor != uc_cm_file.end(); ++itor) *itor = toupper(*itor);
        if ("NONE" == uc_cm_file) {
            m_pars.Prompt("numxpix");
            m_pars.Prompt("numypix");
            m_pars.Prompt("pixscale");
            m_pars.Prompt("coordsys");
            m_pars.Prompt("xref");
            m_pars.Prompt("yref");
            m_pars.Prompt("axisrot");
            m_pars.Prompt("proj");
            m_pars.Prompt("layers");
            m_pars.Prompt("emin");
            m_pars.Prompt("eratio");
        }
    
        m_pars.Prompt("filter");
        m_pars.Prompt("table");
        m_pars.Prompt("chatter");
        m_pars.Prompt("clobber");
        m_pars.Prompt("debug");
        m_pars.Prompt("gui");
        m_pars.Save();
    }

private:
    st_stream::StreamFormatter m_f;
    st_app::AppParGroup& m_pars;

};
// Factory which can create an instance of the class above.
st_app::StAppFactory<ExposureMapApp> g_factory("exposure_map");

/** @page exposure_map_guide exposure_map users's Guide

 - Input: an exposure cube FITS file and an effective area function.
 - Output: an image FITS file with with multiple layers for different energies

 This application  reads an exposure cube, as generated by the 
 <a href="exposure_cube_guide.html">exposure_cube</a> application. The third dimension is bins
 in theta.
 
  It creates an exposure map, or maps, for a specific effective area function, defined by the 
  "rspfunc" parameter. Values are
  @param SIMPLE - simple linear function
  @param DC1F - DC1 parametrization for the front
  @param DC1B - DC1 parametraizatio for the back
  @param DC1FB - sum o the above

  The parameters describing the output image are 
  @param pixelsize degrees per pixel
  @param npix  number of pixels across
  @param projtype CAR for cartesian, AIT for Hammer-Aitoff, etc.


- The parameter file
 @verbinclude exposure_map.par

*/
