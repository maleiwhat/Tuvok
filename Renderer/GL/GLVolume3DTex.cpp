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

/**
  \file    GLVolume3DTex.cpp
  \author  Jens Krueger
           DFKI Saarbruecken & SCI Institute University of Utah
  \date    May 2010
*/

#include "GLVolume3DTex.h"
#include "GLTexture3D.h"

using namespace tuvok;

GLVolume3DTex::GLVolume3DTex() :
  m_pTexture(NULL)
{
}

GLVolume3DTex::GLVolume3DTex(UINT32 iSizeX, UINT32 iSizeY, UINT32 iSizeZ,
                             GLint internalformat, GLenum format, GLenum type,
                             UINT32 iSizePerElement,
                             const GLvoid *pixels,
                             GLint iMagFilter,
                             GLint iMinFilter,
                             GLint wrapX,
                             GLint wrapY,
                             GLint wrapZ) :
  m_pTexture(new GLTexture3D(iSizeX,  iSizeY,  iSizeZ,
                             internalformat,  format,  type,
                             iSizePerElement, pixels, iMagFilter,
                             iMinFilter, wrapX, wrapY, wrapZ))
{
}

GLVolume3DTex::~GLVolume3DTex() {
}

void GLVolume3DTex::Bind(UINT32 iUnit) {
  m_pTexture->Bind(iUnit);
}

void GLVolume3DTex::FreeGLResources() {
  if (m_pTexture) {
    m_pTexture->Delete();
    delete m_pTexture;
  }
  m_pTexture = NULL;
}

void GLVolume3DTex::SetData(const void *voxels) {
  if (m_pTexture) m_pTexture->SetData(voxels);
}

UINT64 GLVolume3DTex::GetCPUSize() {
  return (m_pTexture) ? m_pTexture->GetCPUSize() : 0;
}

UINT64 GLVolume3DTex::GetGPUSize() {
  return (m_pTexture) ? m_pTexture->GetGPUSize() : 0;
}