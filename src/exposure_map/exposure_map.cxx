/** @file exposure_map.cxx
@brief build the exposure_map application

@author Toby Burnett

$Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/exposure_map/exposure_map.cxx,v 1.11 2004/03/31 13:58:11 burnett Exp $
*/

#include "map_tools/SkyImage.h"
#include "map_tools/MapParameters.h"
#include "map_tools/Exposure.h"

#include "st_app/StApp.h"
#include "st_app/StAppFactory.h"
#include "st_app/AppParGroup.h"

#include "astro/SkyDir.h"
using namespace map_tools;

/** @class ExposureMapApp
@brief the exposure_map application class

*/

class ExposureMapApp : public  st_app::StApp  {
public:
     //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
   /** \brief Create AppExample2 object, performing initializations needed for running the application.
    */
    ExposureMapApp(): st_app::StApp(), m_par_group(st_app::StApp::getParGroup("exposure_map")) {
      // Prompt for all parameters.
      m_par_group.Prompt();

      // Save the values just prompted for.
      m_par_group.Save();
    }

    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /** @class Aeff
    @brief function class implements effective area
    */
    class Aeff : public Exposure::Aeff{
    public:
        Aeff(double cutoff=0.25):m_cutoff(cutoff){}

        double operator()(double costh) const
        {
            if(m_cutoff==2.) return 1.0;
            return costh<m_cutoff? 0 : (costh-m_cutoff)/(1.-m_cutoff);
        }
        double m_cutoff;
    };
    //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /** @class RequestExposure 
    @brief function class requests a point from the exposure
    */
    class RequestExposure : public astro::SkyFunction
    {
    public:
        RequestExposure(const Exposure& exp, const Aeff& aeff, double norm=1.0)
            : m_exp(exp)
            , m_aeff(aeff)
            , m_norm(norm)
        {}
        double operator()(const astro::SkyDir& s)const{
            return m_norm*m_exp(s, m_aeff);
        }
    private:
        const Exposure& m_exp;
        const Aeff& m_aeff;
        double m_norm;
    };
 
    void run() {
        // read in, or prompt for, all necessary parameters
        MapParameters pars( m_par_group);

        // create the exposure, read it in from the FITS input file
        Exposure ex(pars.inputFile() ); 

        // create the image object, fill it from the exposure, write out
        SkyImage image(pars); 

        RequestExposure req(ex, Aeff(), 1.0);
        image.fill(req);

    }

private:
    st_app::AppParGroup & m_par_group;


};
// Factory which can create an instance of the class above.
st_app::StAppFactory<ExposureMapApp> g_factory;

