// $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/mainpage.h,v 1.5 2004/03/06 15:14:33 burnett Exp $
// Mainpage for doxygen

/*! \mainpage package map_tools

   \authors Toby Burnett

   \section introduction Introduction

   This package contains utility classes for managing GLAST images for count and exposure maps, 
   and the following applications.

   \section applications Applications
    
    - count_map, defined in count_map.cxx, much of analysis in class
	SkyImage, with parameters from class CountMapParameters, data selection from class SkyMap
    - exposure_cube, defined in exposure_cube.cxx. Also uses  CountMapParameters, Exposure. Create a hypercube 
    in ra, dec, costheta, save as a multi-image FITS file.
    - exposure_map, defined in exposure_map.cxx. Also uses SkyImage, MapParameters, Exposure. 
    Read in a hypercube, make a sky map using a basic function to describe the effective area vs. angle.
<br>
    Each application has an example  .par file in the pfiles folder.
    <hr>

    \section fits FITS Image interface

    The package contains a folder src/image which has a subset of the 
    <a href="http://isdc.unige.ch/Soft/AstroRoot/">astroroot</a> 
    TFcontainer code, written by Reiner Rohlfs.
    It has been stripped of ROOT dependence and simplified. 
    It does not support tables, or any IO except for FITS. Base classes are BaseAttr, VirtualIO and Header.
    
    The interface is not made public, since it will be eventually replaced by James Peachy's table package.
  
  \section notes release notes
  release.notes
  \section requirements requirements
  \include requirements

*/


/**
 @page userGuide User's Guide

@section par par files
 The format of the entries in a .par file is given in the <a
 href="http://www-glast.slac.stanford.edu/sciencetools/userInterface/doc/pil.pdf">PIL user
 manual</a>.  The ordering of the fields in each line is
 - variable name
 - variable type (s=string, r=real, i=integer, b=boolean)
 - query mode (q=ask, h=hidden, l=learn)
 - default value
 - lower bound or a list of allowed selections delimited by "|"
 - upper bound
 - prompt string

 @section env Environment variables

 The package requirements file defines the following environment variables
- INFILES  input files, subfolder "data"
- PFILES   parameter files, subfolder "pfiles" - PIL will expect the parameter file to be here
- OUTFILES output files, subfolder "output"

 @section countmap count_map

This application reads (ra,dec) pairs from a tuple, from either FITS or ROOT, and makes a FITS image file.
The user can specify a selection string, but this can be slow, and for large input files it is better to use
another tool, such a ftselect (FITS) or the root command line, to prepare a new input file.  

 @verbinclude count_map.par

 @section exposurecube exposure_cube

This applicaton creates a FITS "hypercube" image file. Each layer is a bin in cos(theta). 

 @verbinclude exposure_cube.par

 @section exposuremap exposure_map

 Here we create exposure maps for a specific effective area cos theta function. A linear function is wired into this 
 application, see Aeff,  but is is quite representative of the function for most energies. A map for a specific energy, 
 or integrated over energy with a power-law weighting, could be done easily by replacing the Aeff functor.


 @verbinclude exposure_map.par

 @section readmap read_map

 A simple application that reads a value from a map.
 @verbinclude read_map.par

*/
/** @page devguide Developer's Guide

[in preparation]
*/