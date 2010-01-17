/*
   For more information, please see: http://software.sci.utah.edu

   The MIT License

   Copyright (c) 2008 Scientific Computing and Imaging Institute,
   University of Utah.


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

#pragma once

#ifndef RENDERREGION_H
#define RENDERREGION_H

namespace tuvok {

class AbstrRenderer;
class GLRenderer;

  class RenderRegion {
  public:

    enum EWindowMode {
      WM_SAGITTAL = 0,
      WM_AXIAL    = 1,
      WM_CORONAL  = 2,
      WM_3D,
      WM_INVALID
    };

    UINTVECTOR2 minCoord, maxCoord;
    EWindowMode windowMode;
    bool redrawMask;

    RenderRegion(EWindowMode mode):
      windowMode(mode),
      redrawMask(true)
    {
    }
    virtual ~RenderRegion() { /* nothing to destruct */ }

    virtual bool is2D() const { return false; }
    virtual bool is3D() const { return false; }

    bool ContainsPoint(UINTVECTOR2 pos) {
      return (minCoord[0] < pos[0] && pos[0] < maxCoord[0]) &&
        (minCoord[1] < pos[1] && pos[1] < maxCoord[1]);
    }

  protected:
    //These methods should be accessed through AbstrRenderer
    friend class AbstrRenderer;
    friend class GLRenderer;
    virtual bool GetUseMIP() const = 0;
    virtual void SetUseMIP(bool) = 0;
    virtual UINT64 GetSliceIndex() const = 0;
    virtual void SetSliceIndex(UINT64 index) = 0;
    virtual void GetFlipView(bool &flipX, bool &flipY) const = 0;
    virtual void SetFlipView(bool flipX, bool flipY) = 0;
  };

  class RenderRegion2D : public RenderRegion {
  public:

    VECTOR2<bool>       flipView;

    RenderRegion2D(EWindowMode mode, UINT64 sliceIndex) :
      RenderRegion(mode),
      useMIP(false),
      sliceIndex(sliceIndex)
    {
      flipView = VECTOR2<bool>(false, false);
    }
    virtual ~RenderRegion2D() { /* nothing to destruct */ }

    virtual bool is2D() const { return true; }

  protected:
    //These methods should be accessed through AbstrRenderer
    friend class AbstrRenderer;
    friend class GLRenderer;
    virtual bool GetUseMIP() const { return useMIP; }
    virtual void SetUseMIP(bool useMIP_) { useMIP = useMIP_; }
    virtual UINT64 GetSliceIndex() const { return sliceIndex; }
    virtual void SetSliceIndex(UINT64 index) { sliceIndex = index; }
    virtual void GetFlipView(bool &flipX, bool &flipY) const {
      flipX = flipView.x;
      flipY = flipView.y;
    }
    virtual void SetFlipView(bool flipX, bool flipY) {
      flipView.x = flipX;
      flipView.y = flipY;
    }

    bool useMIP;
    UINT64 sliceIndex;
  };

  class RenderRegion3D : public RenderRegion {
  public:
    RenderRegion3D() : RenderRegion(WM_3D) { /*no code */ }
    virtual ~RenderRegion3D() { /* nothing to destruct */ }

    virtual bool is3D() const { return true; }

  protected:
    //These methods should be accessed through AbstrRenderer
    friend class AbstrRenderer;
    friend class GLRenderer;
    // 3D regions don't do the following things:
    virtual bool GetUseMIP() const { assert(false); return false; }
    virtual void SetUseMIP(bool) { assert(false); }
    virtual UINT64 GetSliceIndex() const { assert(false); return 0; }
    virtual void SetSliceIndex(UINT64) { assert(false); }
    virtual void SetFlipView(bool, bool) { assert(false); }
    virtual void GetFlipView(bool &, bool &) const {assert(false); }
  };
};

#endif // RENDERREGION_H
