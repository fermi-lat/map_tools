/** @file CosineBinner.h
@brief Define the CosineBinner classixel 

@author T. Burnett

$Header$
*/

#ifndef map_toos_CosineBinner_h
#define map_toos_CosineBinner_h

#include <vector>
namespace map_tools {

    /** @class CosineBinner
        @brief manage a set of bins in cos(theta)

    */

class CosineBinner : public std::vector<float> {
public:
    CosineBinner();
    
    /// the binning function: add value to the selected bin, if costheta in range
    void fill(double costheta, double value);
    
    /// modifiable reference to the contenta of the bin containing the cos(theta) value
    float& operator[](double costheta);
    const float& operator[](double costheta)const;

    /// cos(theta) for the iterator
    double costheta(std::vector<float>::const_iterator i)const;

    /// integral over the range with functor accepting costheta as an arg. 
    template<class F>
        double operator()(const F& f)const
    {   
        double sum=0;
        for(const_iterator it=begin(); it!=end(); ++it){
            sum += (*it)*f(costheta(it));
        }
        return sum; 

    }

    static std::string thetaBinning();

    static double s_cosmin; ///< minimum value of cos(theta)
    static size_t s_nbins;  ///< number of bins
    static bool  s_sqrt_weight; ///< true to use sqrt function, otherwise linear
};

}
#endif
