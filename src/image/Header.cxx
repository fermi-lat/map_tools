/**
 * @file Header.cxx
 * @brief Header class implementation.
 * @authors T. Burnett, J. Chiang
 * Original code from Riener Rohlfs
 *
 * $Header: /nfs/slac/g/glast/ground/cvs/map_tools/src/image/Header.cxx,v 1.1 2004/03/02 23:24:57 jchiang Exp $
 */

#include "Header.h"

Header::~Header() {
   BaseAttrMap::iterator it = this->begin();
   for ( ; it!= this->end(); ++it) {
      delete it->second;
   }
}

void Header::addAttribute(const BaseAttr& attribute, bool replace) {
   std::string name = attribute.name();
   BaseAttrMap::iterator it = find(name);
   if (it == end()) {
      insert(BaseAttrMap::value_type(name, attribute.clone()));
      return;
   } 
   if (replace) {
      delete it->second;
      it->second = attribute.clone();
      return;
   } else {
      throw std::runtime_error("Header::addAttribute: attribute "
                               + name + " already exists.");
   }
}

const BaseAttr * Header::operator[](const std::string & name) const {
   BaseAttrMap::const_iterator it = find(name);
   if (it != end()) {
      return it->second;
   } else {
      return 0;
   }
}
