/**  @file  Fits_IO.cxx
    @brief Implementaion of Fits_IO 

    @author Toby Burnett
    Code orginally written by Riener Rohlfs

    $Header$
*/

#include "Fits_IO.h"
#include "IOElement.h"
#include "Image.h"

#include "fitsio.h"
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <typeinfo>

class BaseImage;

//_____________________________________________________________________________
// Fits_IO 
//    This class is  internal,  used by IOElement to read and store data into
//    FITS files.

static const char* errMsg[] = {
    "Cannot open the file %s. FITS error: %d",
        "Cannot write/update keyword %s of %s in file %s. FITS error: %d",
        "HDU %s does not exist in file %s",
        "HDU number %d does not exist in file %s",
        "HDU number %d in file %s has name %s, but expecting name %s",
        "Found %s in file %s, but expected a %s",
        "Can neither create nor open the file %s",
        "Neither name nor HDU number is defined. Cannot open an element in file %s"
};

static void HeaderFits2Root(fitsfile * fptr, IOElement * element, int * status);
static void HeaderRoot2Fits(IOElement * element, fitsfile * fptr, int * status);

IOElement * MakeImage(fitsfile * fptr, int * status);
int           CreateFitsImage(fitsfile* fptr, IOElement* image);
int           SaveImage(fitsfile* fptr, BaseImage* image);


//_____________________________________________________________________________
Fits_IO::Fits_IO( IOElement * element, const std::string & fileName)
: VirtualIO(element)
{
    // opens a file or creates a new file if the file does not exist and creates
    // a new HDU in the FITS file.


    int status = 0;
    // tries to create a FITS file. It will fail if it already exist
    fits_create_file((fitsfile**)&m_fptr, fileName.c_str(), &status);

    // try to open a file if the creation failed
    if (status != 0)
    {
        status = 0;
        fits_open_file((fitsfile**)&m_fptr, fileName.c_str(), READWRITE, &status);
        if (status != 0)
        {
            report_error(status);
            //TFError::SetError("Fits_IO::Fits_IO", errMsg[6], fileName); 
            m_fptr = NULL;
        }
    }
    else
    {
        // create a primary array, which we will not use
        long   axis[1] = {0};
        fits_create_img((fitsfile*)m_fptr, SHORT_IMG, 0, axis, &status);

    }
}
//_____________________________________________________________________________
Fits_IO::Fits_IO( IOElement * element, void * fptr, int cycle)
: VirtualIO(element), m_fptr(fptr), m_cycle(cycle)
{
    // constructor

}

Fits_IO::~Fits_IO()
{
    // destructor

    int status = 0;
    if (m_fptr)
        fits_close_file((fitsfile*)m_fptr, &status);
}
//_____________________________________________________________________________
IOElement * Fits_IO::read(const std::string & fileName, const std::string & name,
                          int cycle, FMode mode)
{
    // open the fits file
    int status = 0;
    fitsfile * fptr;
    fits_open_file(&fptr, fileName.c_str(), mode == Read ? READONLY : READWRITE, 
        &status);
    if (status != 0)   
    {
        Fits_IO::report_error(status);
        return NULL;
    }

    // move to the required HDU
    if (cycle <= 0) {
        if ( name.empty() ) {
            throw std::invalid_argument(errMsg[7]);
            //Error::SetError("Fits_IO::TFRead", errMsg[7], fileName); 
            return NULL;
        }

        // move by name
        fits_movnam_hdu(fptr, ANY_HDU, const_cast<char*>(name.c_str()), -cycle, &status);

        if (status != 0) {
            status = 0;
            // try GROUPING
            char gr[10];
            strcpy(gr, "GROUPING");
            fits_movnam_hdu(fptr, ANY_HDU, gr, 0, &status);
            if ( status != 0 ){             

                //TFError::SetError("Fits_IO::TFRead", errMsg[2], name, fileName); 
                fits_close_file(fptr, &status);
                Fits_IO::report_error(status);
            }
        }

        // get the current hdu number = cycle
        int hduNum;
        fits_get_hdu_num(fptr, &hduNum);
        cycle = hduNum;
    }
    else {
        // move to HDU number (the correct name will be tested later)
        fits_movabs_hdu(fptr, cycle, NULL, &status);
        if (status != 0)  {
            //TFError::SetError("Fits_IO::TFRead", errMsg[3], cycle, fileName); 
            status = 0;
            fits_close_file(fptr, &status);
            throw std::invalid_argument(errMsg[3]);
            return NULL;
        }
    }

    // get the type of element
    IOElement * element=0;
    char elementType[20];
    char keyname[20];
    strcpy(keyname, "XTENSION");
    fits_read_keyword(fptr, keyname, elementType, NULL, &status);

    if (status == 0 && strcmp(elementType , "'BINTABLE'") == 0){
        throw std::invalid_argument("read table, can't instantiate yet");
        element = 0;;
    }
    else if ( (status == 0 && strstr(elementType , "IMAGE") != NULL ) ||
        status == 202  /* XTENSION keyword does not exist */      )
    {
        status = 0;
        element = MakeImage(fptr, &status);
    }
    else {
        element = 0; //THB new IOElement("no name");
        status = 0;
    }

    // copy the header and set the element name
    HeaderFits2Root(fptr, element, &status);


    if (element) {
        element->setIO(new Fits_IO(element, fptr, cycle));
        element->setFileAccess(mode);
    }
    return element;

}
//_____________________________________________________________________________
bool Fits_IO::isOpen()
{

    return m_fptr != NULL;
}
//_____________________________________________________________________________
const std::string  Fits_IO::getFileName()
{

    static std::string null;
    if (m_fptr == 0)
        return null;

    int status = 0;
    static char fileName[512];
    fits_file_name((fitsfile*)m_fptr, fileName, &status);
    return status == 0 ?  std::string(fileName) : null;
}
//_____________________________________________________________________________
int Fits_IO::getCycle()
{
    return m_cycle;
}
//_____________________________________________________________________________
void Fits_IO::createElement()
{
    // creates a new element
    if (m_fptr == NULL)
        return;

    fitsfile * fptr = (fitsfile*)m_fptr;
    int status =0;

    if( m_element->isImage()) {
        status = CreateFitsImage(fptr, m_element );
    }else{
        throw std::invalid_argument("Tables not yet implemented");
    }


    if (status == 0) {         
        // get the current hdu number = cycle
        int hduNum;
        fits_get_hdu_num(fptr, &hduNum);
        m_cycle = (unsigned int)hduNum;
    }   
    else{
        status = 0;
        fits_close_file(fptr, &status);
        m_fptr = NULL;
    }

}
//_____________________________________________________________________________
int Fits_IO::deleteElement()
{

    if (m_fptr == NULL)
        return 0;

    fitsfile * fptr = (fitsfile*)m_fptr;
    int status = 0;

    int numHdus;
    fits_get_num_hdus(fptr, &numHdus, &status);
    if (status != 0)  return -1;

    if (numHdus == 1)
    {
        // There is only one HDU left, delete the file
        fits_delete_file(fptr, &status);
    }
    else
    {
        fits_delete_hdu(fptr, NULL, &status);
        // fptr points now to an other HDU which we dont want.
        // We close it.
        fits_close_file(fptr, &status);
    }   
    m_fptr = 0;

    return 0;
}
//_____________________________________________________________________________
int Fits_IO::saveElement()
{
    // saves the current element into the FITS file

    if (m_fptr == 0) return 0;

    fitsfile * fptr = (fitsfile*)m_fptr;
    int status = 0;

    if( m_element->isImage() ) {
        status = SaveImage(fptr, dynamic_cast<BaseImage*>(m_element));
    }else {
        throw std::runtime_error("table writing not yet implemented");
    }

    HeaderRoot2Fits( element(), fptr, &status);

    fits_write_chksum(fptr, &status);
    //   if (status == 232)
    //      status = 0;   // this happens with a new table

    return status == 0 ? 0 : -1;
}
//_____________________________________________________________________________
void Fits_IO::report_error(int status)
{
    if(status==0) return;
    std::stringstream msg; msg << "fits error #: " << status;
    static char buffer[240]; 
    while ( fits_read_errmsg(buffer)!=0 ) msg << "\n" << buffer;
    std::cerr << msg.str() << std::endl;
    throw std::runtime_error(msg.str());
}

//_____________________________________________________________________________
static void HeaderFits2Root(fitsfile * fptr, IOElement * element, int * status)
{
    if (*status != 0)  return;

    char name[30], value[100], comment[100], unit[40];


    int nkeys;
    fits_get_hdrspace(fptr, &nkeys, NULL, status);
    for (int num = 0; num < nkeys && *status == 0; num++)
    {
        fits_read_keyn(fptr, num + 1, name, value, comment, status);
        if (strncmp(name, "TTYPE", 5) == 0  || 
            strncmp(name, "TFORM", 5) == 0  || 
            strncmp(name, "TNULL", 5) == 0  ||
            strncmp(name, "TZERO", 5) == 0  ||
            strncmp(name, "TSCAL", 5) == 0  ||
            strncmp(name, "NAXIS", 5) == 0  ||
            strcmp (name, "XTENSION") == 0  ||
            strcmp (name, "TFIELDS") == 0   ||
            strcmp (name, "BITPIX") == 0    ||
            strcmp (name, "PCOUNT") == 0    ||
            strcmp (name, "GCOUNT") == 0    ||
            strcmp (name, "BLANK") == 0     ||
            strcmp (name, "BSCALE") == 0    ||
            strcmp (name, "BZERO") == 0     ||
            strcmp (name, "CHECKSUM") == 0  ||
            strcmp (name, "DATASUM") == 0      )
            continue;

        fits_read_key_unit(fptr, name, unit, status);

        if (value[0] == '\'' && value[strlen(value) - 1] == '\'')
        {
            int len = strlen(value) - 2;
            while (value[len] == ' ') len--;
            value[len+1] = 0;
            std::string svalue(value+1);
            if (strcmp("EXTNAME", name) != 0 && strcmp("GRPNAME", name) != 0)
                element->addAttribute(StringAttr(name, svalue, unit, comment), false);

            // set the element name
            if (strcmp("EXTNAME", name) == 0 && strcmp("GROUPING", value+1) != 0)
                element->setName(svalue);
            else if (strcmp("GRPNAME", name) == 0)
                element->setName(svalue);

        }
        else if (strcmp(value, "T") == 0)
            element->addAttribute(BoolAttr(name, true, unit, comment), false);
        else if (strcmp(value, "F") == 0)
            element->addAttribute(BoolAttr(name, false, unit, comment), false);
        else if (strlen(value) < 9 && value[0] != 0 && strchr(value, '.') == NULL)
            element->addAttribute(IntAttr(name, atoi(value), unit, comment), false);
        else if (strlen(value) < 9 && value[0] != 0)
        {
            double val;
            sscanf(value, "%lg", &val);
            element->addAttribute(DoubleAttr(name, val, unit, comment), false);
        }
        else
            element->addAttribute(StringAttr(name, std::string(value), unit, comment), false);
    }

}
//_____________________________________________________________________________
static void HeaderRoot2Fits(IOElement * element, fitsfile * fptr, int * status)
{
    if (*status != 0)
        return;
    for( Header::iterator it = element->begin(); it != element->end(); ++it){
        BaseAttr& attr = **it;

           std::string tname(typeid(attr).name());
           int k = tname.find_first_of("<")+1;

           if (tname.substr(k,4)=="bool") {

               int val = (dynamic_cast< Attr<bool>& >(attr)).value();
               fits_update_key(fptr, TLOGICAL, (char*)attr.name().c_str(), 
                   &val, (char*)attr.comment().c_str(), status);

           }else if (tname.substr(k,6)=="double") {

               double val = (dynamic_cast< Attr<double>& >(attr)).value();
               fits_update_key(fptr, TDOUBLE, (char*)attr.name().c_str(), 
                   &val, (char*)attr.comment().c_str(), status);

           }else if (tname.substr(k,3)=="int") {

               int val = (int)(dynamic_cast< Attr<double>& >(attr)).value();
               fits_update_key(fptr, TINT, (char*)attr.name().c_str(), 
                   &val, (char*)attr.comment().c_str(), status);
    
           }else if (tname.substr(k,23)=="class std::basic_string") {

               Attr<std::string>& tattr = (Attr<std::string>&)attr;
                  // dynamic_cast<class Attr<class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> > > >(attr).value();
                    
               const char * v = tattr.value().c_str();
               fits_update_key(fptr, TSTRING, (char*)attr.name().c_str(), 
                   (char*)v, (char*)attr.comment().c_str(), status);
           }else if (tname.substr(k,4)=="char") {

               const char* val = (dynamic_cast< Attr<char *>& >(attr)).value();
                fits_update_key(fptr, TSTRING, (char*)attr.name().c_str(), 
                    (char*)val, (char*)attr.comment().c_str(), status);

           }else {
               throw std::runtime_error(std::string("Unexpected attribute type:")+tname);
           }


        if (! attr.unit().empty() ) {
            fits_write_key_unit(fptr, (char*)attr.name().c_str(), 
            (char*)attr.unit().c_str(), status);
        }
        if (*status != 0) {
            Fits_IO::report_error(*status);
        }
    }

}

#if 0 //do we really need this?
namespace {
// this unused function ensures that the cfitsio function ffgiwcs and ffgtwcs are linked
// into the libtf.so library.
static void never_used()
{
    int status;
    fitsfile * fptr;
    char * header;

    ffgiwcs(fptr, &header, &status);
    ffgtwcs(fptr, 1, 2, &header, &status);
}
}
#endif
