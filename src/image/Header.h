/** @file Header.h
    @brief definition of class Header

     @author Toby Burnett
     Code orginally written by Riener Rohlfs

     $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/image/Header.h,v 1.4 2004/02/28 21:04:55 burnett Exp $
*/
#ifndef HEADER_H
#define HEADER_H

#include "Attr.h"
#include <map>
#include <typeinfo>
#include <stdexcept>
#include <vector>
#include <string>

/** @class Header
    @brief Base class for all data objects, contains the header information
*/
class Header : public std::map<std::string, BaseAttr*>
{
public:
   Header() {}
   virtual ~Header();
   void addAttribute(const BaseAttr& attribute, bool replace=true);
   const BaseAttr* operator[](const std::string & name) const;

   template<typename T> void getValue(const std::string & name,
                                      T &value) {
      BaseAttrMap::iterator it = find(name);
      BaseAttr * my_attr = 0;
      if (it != end()) {
         my_attr = it->second;
      }
      if (my_attr) {
         if (dynamic_cast<Attr<T> *>(my_attr)) {
            value = dynamic_cast<Attr<T> *>(my_attr)->value();
         } else {
            throw std::runtime_error("Header::getValue: bad cast of type "
                                     + std::string(typeid(value).name()));
         }
      } else {
         throw std::invalid_argument("Header::getValue: attribute " 
                                     + name + " not found.");
      }         
   }

   void setName(const std::string name){m_name=name;}

private:

   typedef std::map<std::string, BaseAttr*> BaseAttrMap;

   std::string m_name;
};

#endif
