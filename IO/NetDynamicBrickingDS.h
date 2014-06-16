/* This is a "dynamic bricking dataset" which also forwards all the requests it
 * gets to a remote server. */
#ifndef TUVOK_NET_DYNAMIC_BRICKING_DS_H
#define TUVOK_NET_DYNAMIC_BRICKING_DS_H

#include <array>
#include <memory>
#include <vector>
#include "LinearIndexDataset.h"
#include "FileBackedDataset.h"

namespace tuvok {

class DynamicBrickingDS;

class NetDynDS : public LinearIndexDataset, public FileBackedDataset {
public:
  NetDynDS(const char* filename,
           std::array<size_t, 3> maxBrickSize,
           size_t cacheBytes);
  virtual ~NetDynDS();

  virtual std::shared_ptr<const Histogram1D> Get1DHistogram() const;
  virtual std::shared_ptr<const Histogram2D> Get2DHistogram() const;

  /// modifies the cache size used for holding large bricks.
  void SetCacheSize(size_t megabytes);
  /// get the cache size used for holding large bricks in MB
  size_t GetCacheSize() const;

  virtual float MaxGradientMagnitude() const;
  /// Removes all the cache information we've made so far.
  virtual void Clear();

  /// Data access
  ///@{
  virtual bool GetBrick(const BrickKey&, std::vector<uint8_t>&) const;
  virtual bool GetBrick(const BrickKey&, std::vector<uint16_t>&) const;
  virtual bool GetBrick(const BrickKey&, std::vector<uint32_t>&) const;
  ///@}

  /// User rescaling factors.
  ///@{
  void SetRescaleFactors(const DOUBLEVECTOR3&);
  DOUBLEVECTOR3 GetRescaleFactors() const;
  /// If the underlying file format supports it, save the current scaling
  /// factors to the file.  The format should implicitly load and apply the
  /// scaling factors when opening the data set.
  virtual bool SaveRescaleFactors();
  DOUBLEVECTOR3 GetScale() const;
  ///@}

  virtual unsigned GetLODLevelCount() const;
  virtual uint64_t GetNumberOfTimesteps() const;
  virtual UINT64VECTOR3 GetDomainSize(const size_t lod=0,
                                      const size_t ts=0) const;
  virtual UINTVECTOR3 GetBrickOverlapSize() const;
  UINT64VECTOR3 GetEffectiveBrickSize(const BrickKey&) const;

  virtual UINTVECTOR3 GetMaxBrickSize() const;
  virtual UINTVECTOR3 GetBrickLayout(size_t lod, size_t ts) const;

  virtual unsigned GetBitWidth() const;
  virtual uint64_t GetComponentCount() const;
  virtual bool GetIsSigned() const;
  virtual bool GetIsFloat() const;
  virtual bool IsSameEndianness() const;
  virtual std::pair<double,double> GetRange() const;

  /// Acceleration queries.
  virtual bool ContainsData(const BrickKey&, double /*isoval*/) const;
  virtual bool ContainsData(const BrickKey&, double /*fMin*/,
                            double /*fMax*/) const;
  virtual bool ContainsData(const BrickKey&, double /*fMin*/, double /*fMax*/,
                            double /*fMinGradient*/,
                            double /*fMaxGradient*/) const;
  virtual tuvok::MinMaxBlock MaxMinForKey(const BrickKey& k) const;

  /// unimplemented!  Override these if you want tools built on this IO layer
  /// to be able to create data in your format.
  virtual bool Export(uint64_t iLODLevel, const std::string& targetFilename,
                      bool bAppend) const;

  virtual bool ApplyFunction(uint64_t iLODLevel, 
                        bool (*brickFunc)(void* pData, 
                                          const UINT64VECTOR3& vBrickSize,
                                          const UINT64VECTOR3& vBrickOffset,
                                          void* pUserContext),
                        void *pUserContext,
                        uint64_t iOverlap) const;

  /// Virtual constructor.
  virtual NetDynDS* Create(const std::string&, uint64_t,bool) const;

  /// functions for FileBackedDataset interface
  ///@{
  virtual std::string Filename() const;
  virtual const char* Name() const;

  virtual bool CanRead(const std::string&, const std::vector<int8_t>&) const;
  virtual bool Verify(const std::string&) const;
  virtual std::list<std::string> Extensions() const;
  ///@}

private:
  std::unique_ptr<DynamicBrickingDS> ds;
};

}
#endif
/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2014 The ImageVis3D Developers


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