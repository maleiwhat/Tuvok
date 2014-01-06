#pragma once

#ifndef STLGEOCONVERTER_H
#define STLGEOCONVERTER_H

#include "../StdTuvokDefines.h"
#include "AbstrGeoConverter.h"

namespace tuvok {

  class StLGeoConverter : public AbstrGeoConverter {
  public:
    StLGeoConverter();
    virtual ~StLGeoConverter() {}
    virtual std::shared_ptr<Mesh>
      ConvertToMesh(const std::string& strFilename);


    virtual bool ConvertToNative(const Mesh& m,
                                 const std::string& strTargetFilename);

    virtual bool ConvertToNative(const Mesh& m,
                                 const std::string& strTargetFilename,
                                 bool bASCII);

    virtual bool CanExportData() const { return true; }
    virtual bool CanImportData() const { return true; }
  private:
    FLOATVECTOR3 ComputeFaceNormal(const Mesh& m, size_t i,
                                   bool bHasNormals) const;

  };
}
#endif // STLGEOCONVERTER_H

/*
   The MIT License

   Copyright (c) 2013 Jens Krueger

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
