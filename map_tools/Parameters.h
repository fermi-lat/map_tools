/**
* @file Parameters.h
* @brief Tool Input Parameter Reader base class
*
* $Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/Parameters.h,v 1.3 2004/02/28 14:20:22 burnett Exp $
*/
#ifndef PARAMETERS_H
#define PARAMETERS_H 

#include <string>
#include <map>
namespace map_tools {
/**
* @class Parameters
* @brief Input reader base class for tools
*
* It uses PIL to read parameters from the par file.
* The description of PIL is available at
* href="http://www-glast.slac.stanford.edu/sciencetools/userInterface/doc/pil.pdf">PIL user
* manual</a>.
*
* @author Toby Burnett [originally from Sandhia Bansall]
*
* $Header: /nfs/slac/g/glast/ground/cvs/map_tools/map_tools/Parameters.h,v 1.3 2004/02/28 14:20:22 burnett Exp $
*/
class Parameters
{
public:
    // Constructors
    Parameters( int argc, char *argv[]);
    ~Parameters();


    // Accessor Methods
    const std::string &eventFile() const   { return m_eventFile; }
    const std::string &inputFile() const   { return m_eventFile; }
    const std::string &filter() const      { return m_filter; }
    const std::string &outputFile() const  { return m_oname; }
    static bool verboseMode()              { return s_verboseMode; }
    static bool clobber()                  { return s_clobber; }
    static short chatter()                 { return s_chatter; }

    // special lookup by name
    double operator[](const std::string& name)const;


protected:  // make accessible to sub class
    std::string getString(const std::string& name);
    int         getInt(const std::string& name);
    double      getDouble(const std::string& name);
    bool        getBool(const std::string& name);

    std::string getString(const std::string& name, const std::string& deflt);
    int         getInt(const std::string& name,   int deflt);
    double      getDouble(const std::string& name, double deflt);
    bool        getBool(const std::string& name, bool deflt);

    // special
    typedef std::map<std::string,double> DoubleDict;
private:
    // Data Members
    std::string   m_eventFile;
    std::string   m_oname;
    std::string   m_filter;
    DoubleDict m_dictionary;

    static std::string s_tool;
    static bool        s_verboseMode;
    static bool        s_clobber;
    static int         s_chatter;
};
} // namespace map_tools
#endif
