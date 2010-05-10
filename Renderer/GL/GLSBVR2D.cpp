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
  \file    GLSBVR2D.cpp
  \author  Jens Krueger
           SCI Institute
           University of Utah
  \date    August 2008
*/
#include "GLInclude.h"
#include "GLSBVR2D.h"

#include "Controller/Controller.h"
#include "Renderer/GL/GLSLProgram.h"
#include "Renderer/GL/GLTexture1D.h"
#include "Renderer/GL/GLTexture2D.h"
#include "Renderer/GL/GLVolume2DTex.h"
#include "Renderer/GPUMemMan/GPUMemMan.h"
#include "Renderer/TFScaling.h"

using namespace std;
using namespace tuvok;

GLSBVR2D::GLSBVR2D(MasterController* pMasterController, bool bUseOnlyPowerOfTwo, bool bDownSampleTo8Bits, bool bDisableBorder) :
  GLRenderer(pMasterController, bUseOnlyPowerOfTwo, bDownSampleTo8Bits, bDisableBorder),
  m_pProgramIsoNoCompose(NULL),
  m_pProgramColorNoCompose(NULL),
  m_bUse3DTexture(false)
{
}

GLSBVR2D::~GLSBVR2D() {
}

void GLSBVR2D::CleanupShaders() {
  GLRenderer::CleanupShaders();
  if (m_pProgramIsoNoCompose)   {m_pMasterController->MemMan()->FreeGLSLProgram(m_pProgramIsoNoCompose); m_pProgramIsoNoCompose =NULL;}
  if (m_pProgramColorNoCompose) {m_pMasterController->MemMan()->FreeGLSLProgram(m_pProgramColorNoCompose); m_pProgramColorNoCompose =NULL;}
}

void GLSBVR2D::SetUse3DTexture(bool bUse3DTexture) {

  if (bUse3DTexture != m_bUse3DTexture) {
    m_bUse3DTexture = bUse3DTexture;
    CleanupShaders();
    LoadShaders();
  }
}

void GLSBVR2D::BindVolumeStringsToTexUnit(GLSLProgram* program, 
                                          int iUnit0, int iUnit1) {
  if (m_bUse3DTexture ) {
    program->SetUniformVector("texVolume",iUnit0);
  } else {
    program->SetUniformVector("texSlice0",iUnit0);
    program->SetUniformVector("texSlice1",iUnit1);
  }
}

bool GLSBVR2D::LoadShaders() {
  // do not call GLRenderer::LoadShaders as we want to control 
  // what volume access function is linked (Volume3D or Volume2D)

  string volumeAccessFunction = m_bUse3DTexture ? "Volume3D.glsl" 
                                                : "Volume2D.glsl";

  if(!LoadAndVerifyShader(&m_pProgramTrans, m_vShaderSearchDirs,
                          "Transfer-VS.glsl", "Transfer-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgram1DTransSlice, m_vShaderSearchDirs,
                          "Transfer-VS.glsl", "1D-slice-FS.glsl", volumeAccessFunction.c_str(), NULL) ||
     !LoadAndVerifyShader(&m_pProgram2DTransSlice, m_vShaderSearchDirs,
                          "Transfer-VS.glsl", "2D-slice-FS.glsl", volumeAccessFunction.c_str(), NULL) ||
     !LoadAndVerifyShader(&m_pProgramMIPSlice, m_vShaderSearchDirs,
                          "Transfer-VS.glsl", "MIP-slice-FS.glsl", volumeAccessFunction.c_str(), NULL) ||
     !LoadAndVerifyShader(&m_pProgram1DTransSlice3D, m_vShaderSearchDirs,
                          "SlicesIn3D.glsl", "1D-slice-FS.glsl", volumeAccessFunction.c_str(), NULL) ||
     !LoadAndVerifyShader(&m_pProgram2DTransSlice3D, m_vShaderSearchDirs,
                          "SlicesIn3D.glsl", "2D-slice-FS.glsl", volumeAccessFunction.c_str(), NULL) ||
     !LoadAndVerifyShader(&m_pProgramTransMIP, m_vShaderSearchDirs,
                          "Transfer-VS.glsl", "Transfer-MIP-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgramIsoCompose, m_vShaderSearchDirs,
                          "Transfer-VS.glsl", "Compose-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgramColorCompose, m_vShaderSearchDirs,
                          "Transfer-VS.glsl", "Compose-Color-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgramCVCompose, m_vShaderSearchDirs,
                          "Transfer-VS.glsl", "Compose-CV-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgramComposeAnaglyphs, m_vShaderSearchDirs,
                          "Transfer-VS.glsl", "Compose-Anaglyphs-FS.glsl",
                          NULL)                                              ||
     !LoadAndVerifyShader(&m_pProgramComposeScanlineStereo,
                          m_vShaderSearchDirs, "Transfer-VS.glsl",
                          "Compose-Scanline-FS.glsl", NULL))
  {
      T_ERROR("Error loading transfer function shaders.");
      return false;
  } else {

    m_pProgramTrans->Enable();
    m_pProgramTrans->SetUniformVector("texColor",0);
    m_pProgramTrans->SetUniformVector("texDepth",1);
    m_pProgramTrans->Disable();

    m_pProgram1DTransSlice->Enable();
    BindVolumeStringsToTexUnit(m_pProgram1DTransSlice,0,2);
    m_pProgram1DTransSlice->SetUniformVector("texTrans1D",1);
    m_pProgram1DTransSlice->Disable();

    m_pProgram2DTransSlice->Enable();
    BindVolumeStringsToTexUnit(m_pProgram2DTransSlice,0,2);
    m_pProgram2DTransSlice->SetUniformVector("texTrans2D",1);
    m_pProgram2DTransSlice->Disable();

    m_pProgram1DTransSlice3D->Enable();
    BindVolumeStringsToTexUnit(m_pProgram1DTransSlice3D,0,2);
    m_pProgram1DTransSlice3D->SetUniformVector("texTrans1D",1);
    m_pProgram1DTransSlice3D->Disable();

    m_pProgram2DTransSlice3D->Enable();
    BindVolumeStringsToTexUnit(m_pProgram2DTransSlice3D,0,2);
    m_pProgram2DTransSlice3D->SetUniformVector("texTrans2D",1);
    m_pProgram2DTransSlice3D->Disable();

    m_pProgramMIPSlice->Enable();
    BindVolumeStringsToTexUnit(m_pProgramMIPSlice,0,2);
    m_pProgramMIPSlice->Disable();

    m_pProgramTransMIP->Enable();
    m_pProgramTransMIP->SetUniformVector("texLast",0);
    m_pProgramTransMIP->SetUniformVector("texTrans1D",1);
    m_pProgramTransMIP->Disable();

    FLOATVECTOR2 vParams = m_FrustumCullingLOD.GetDepthScaleParams();

    m_pProgramIsoCompose->Enable();
    m_pProgramIsoCompose->SetUniformVector("texRayHitPos",0);
    m_pProgramIsoCompose->SetUniformVector("texRayHitNormal",1);
    m_pProgramIsoCompose->SetUniformVector("vProjParam",vParams.x, vParams.y);
    m_pProgramIsoCompose->Disable();

    m_pProgramColorCompose->Enable();
    m_pProgramColorCompose->SetUniformVector("texRayHitPos",0);
    m_pProgramColorCompose->SetUniformVector("texRayHitNormal",1);
    m_pProgramColorCompose->SetUniformVector("vProjParam",vParams.x, vParams.y);
    m_pProgramColorCompose->Disable();

    m_pProgramCVCompose->Enable();
    m_pProgramCVCompose->SetUniformVector("texRayHitPos",0);
    m_pProgramCVCompose->SetUniformVector("texRayHitNormal",1);
    m_pProgramCVCompose->SetUniformVector("texRayHitPos2",2);
    m_pProgramCVCompose->SetUniformVector("texRayHitNormal2",3);
    m_pProgramCVCompose->SetUniformVector("vProjParam",vParams.x, vParams.y);
    m_pProgramCVCompose->Disable();

    m_pProgramComposeAnaglyphs->Enable();
    m_pProgramComposeAnaglyphs->SetUniformVector("texLeftEye",0);
    m_pProgramComposeAnaglyphs->SetUniformVector("texRightEye",1);
    m_pProgramComposeAnaglyphs->Disable();

    m_pProgramComposeScanlineStereo->Enable();
    m_pProgramComposeScanlineStereo->SetUniformVector("texLeftEye",0);
    m_pProgramComposeScanlineStereo->SetUniformVector("texRightEye",1);
    m_pProgramComposeScanlineStereo->Disable();    
  }


  if(!LoadAndVerifyShader(&m_pProgram1DTrans[0], m_vShaderSearchDirs,
                          "GLSBVR-VS.glsl", "GLSBVR-1D-FS.glsl", volumeAccessFunction.c_str(), NULL) ||
     !LoadAndVerifyShader(&m_pProgram1DTrans[1], m_vShaderSearchDirs,
                          "GLSBVR-VS.glsl", "lighting.glsl", volumeAccessFunction.c_str(),
                          "GLSBVR-1D-light-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgram2DTrans[0], m_vShaderSearchDirs,
                          "GLSBVR-VS.glsl", "GLSBVR-2D-FS.glsl", volumeAccessFunction.c_str(), NULL) ||
     !LoadAndVerifyShader(&m_pProgram2DTrans[1], m_vShaderSearchDirs,
                          "GLSBVR-VS.glsl", "lighting.glsl", volumeAccessFunction.c_str(),
                          "GLSBVR-2D-light-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgramHQMIPRot, m_vShaderSearchDirs,
                          "GLSBVR-VS.glsl", "GLSBVR-MIP-Rot-FS.glsl", volumeAccessFunction.c_str(), NULL) ||
     !LoadAndVerifyShader(&m_pProgramIso, m_vShaderSearchDirs,
                          "GLSBVR-VS.glsl", "lighting.glsl", volumeAccessFunction.c_str(),
                          "GLSBVR-ISO-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgramColor, m_vShaderSearchDirs,
                          "GLSBVR-VS.glsl", "lighting.glsl", volumeAccessFunction.c_str(),
                          "GLSBVR-Color-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgramIsoNoCompose, m_vShaderSearchDirs,
                          "GLSBVR-VS.glsl", "lighting.glsl", volumeAccessFunction.c_str(),
                          "GLSBVR-ISO-NC-FS.glsl", NULL) ||
     !LoadAndVerifyShader(&m_pProgramColorNoCompose, m_vShaderSearchDirs,
                          "GLSBVR-VS.glsl", "GLSBVR-Color-NC-FS.glsl", volumeAccessFunction.c_str(), NULL))
  {
      Cleanup();

      T_ERROR("Error loading a shader.");
      return false;
  } else {
    m_pProgram1DTrans[0]->Enable();
    BindVolumeStringsToTexUnit(m_pProgram1DTrans[0],0,2);
    m_pProgram1DTrans[0]->SetUniformVector("texTrans1D",1);
    m_pProgram1DTrans[0]->Disable();

    m_pProgram1DTrans[1]->Enable();
    BindVolumeStringsToTexUnit(m_pProgram1DTrans[1],0,2);
    m_pProgram1DTrans[1]->SetUniformVector("texTrans1D",1);
    m_pProgram1DTrans[1]->Disable();

    m_pProgram2DTrans[0]->Enable();
    BindVolumeStringsToTexUnit(m_pProgram2DTrans[0],0,2);
    m_pProgram2DTrans[0]->SetUniformVector("texTrans2D",1);
    m_pProgram2DTrans[0]->Disable();

    m_pProgram2DTrans[1]->Enable();
    BindVolumeStringsToTexUnit(m_pProgram2DTrans[1],0,2);
    m_pProgram2DTrans[1]->SetUniformVector("texTrans2D",1);
    m_pProgram2DTrans[1]->Disable();

    m_pProgramIso->Enable();
    BindVolumeStringsToTexUnit(m_pProgramIso,0,2);
    m_pProgramIso->Disable();

    m_pProgramColor->Enable();
    BindVolumeStringsToTexUnit(m_pProgramColor,0,2);
    m_pProgramColor->Disable();

    m_pProgramHQMIPRot->Enable();
    BindVolumeStringsToTexUnit(m_pProgramHQMIPRot,0,2);
    m_pProgramHQMIPRot->Disable();

    m_pProgramIsoNoCompose->Enable();
    BindVolumeStringsToTexUnit(m_pProgramIsoNoCompose,0,2);
    m_pProgramIsoNoCompose->Disable();

    m_pProgramColorNoCompose->Enable();
    BindVolumeStringsToTexUnit(m_pProgramColorNoCompose,0,2);
    m_pProgramColorNoCompose->Disable();

    UpdateColorsInShaders();
  }

  return true;
}

void GLSBVR2D::SetDataDepShaderVars() {
  GLRenderer::SetDataDepShaderVars();

  if (m_eRenderMode == RM_ISOSURFACE && m_bAvoidSeperateCompositing) {
    GLSLProgram* shader = (m_pDataset->GetComponentCount() == 1) ? m_pProgramIsoNoCompose : m_pProgramColorNoCompose;

    FLOATVECTOR3 d = m_cDiffuse.xyz()*m_cDiffuse.w;

    shader->Enable();
    shader->SetUniformVector("fIsoval", static_cast<float>
                                        (this->GetNormalizedIsovalue()));
    // this is not really a data dependent var but as we only need to
    // do it once per frame we may also do it here
    shader->SetUniformVector("vLightDiffuse",d.x*m_vIsoColor.x,d.y*m_vIsoColor.y,d.z*m_vIsoColor.z);
    shader->Disable();
  }

  if(m_eRenderMode == RM_1DTRANS && m_TFScalingMethod == SMETH_BIAS_AND_SCALE) {
    std::pair<float,float> bias_scale = scale_bias_and_scale(*m_pDataset);
    MESSAGE("setting TF bias (%5.3f) and scale (%5.3f)", bias_scale.first,
            bias_scale.second);
    m_pProgram1DTrans[0]->Enable();
    m_pProgram1DTrans[0]->SetUniformVector("TFuncBias", bias_scale.first);
    m_pProgram1DTrans[0]->SetUniformVector("fTransScale", bias_scale.second);
    m_pProgram1DTrans[0]->Disable();
  }
}

void GLSBVR2D::SetBrickDepShaderVars(RenderRegion3D& region,
                                     const Brick& currentBrick) {
  FLOATVECTOR3 vStep(1.0f/currentBrick.vVoxelCount.x,
                     1.0f/currentBrick.vVoxelCount.y,
                     1.0f/currentBrick.vVoxelCount.z);

  float fSampleRateModifier = m_fSampleRateModifier /
    (region.decreaseSamplingRateNow ? m_fSampleDecFactor : 1.0f);
  float fStepScale =  1.414213562f/fSampleRateModifier * //1.414213562 = sqrt(2)
    (FLOATVECTOR3(m_pDataset->GetDomainSize()) /
     FLOATVECTOR3(m_pDataset->GetDomainSize(m_iCurrentLOD))).maxVal();


  switch (m_eRenderMode) {
    case RM_1DTRANS: {
      GLSLProgram *shader = m_pProgram1DTrans[m_bUseLighting ? 1 : 0];
      shader->SetUniformVector("fStepScale", fStepScale);
      if (m_bUseLighting) {
        m_pProgram1DTrans[1]->SetUniformVector("vVoxelStepsize",
                                               vStep.x, vStep.y, vStep.z);
      }
      break;
    }
    case RM_2DTRANS: {
      GLSLProgram *shader = m_pProgram2DTrans[m_bUseLighting ? 1 : 0];
      shader->SetUniformVector("fStepScale", fStepScale);
      shader->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
      break;
    }
    case RM_ISOSURFACE: {
      GLSLProgram *shader;
      if (m_bAvoidSeperateCompositing) {
        shader = (m_pDataset->GetComponentCount() == 1) ?
                 m_pProgramIsoNoCompose : m_pProgramColorNoCompose;
      } else {
        shader = (m_pDataset->GetComponentCount() == 1) ?
                 m_pProgramIso : m_pProgramColor;
      }
      shader->SetUniformVector("vVoxelStepsize", vStep.x, vStep.y, vStep.z);
      break;
    }
    case RM_INVALID: T_ERROR("Invalid rendermode set"); break;
  }
}

void GLSBVR2D::EnableClipPlane(RenderRegion *renderRegion) {
  if(!m_bClipPlaneOn) {
    AbstrRenderer::EnableClipPlane(renderRegion);
    m_SBVRGeogen.EnableClipPlane();
    PLANE<float> plane(m_ClipPlane.Plane());
    m_SBVRGeogen.SetClipPlane(plane);
  }
}

void GLSBVR2D::DisableClipPlane(RenderRegion *renderRegion) {
  if(m_bClipPlaneOn) {
    AbstrRenderer::DisableClipPlane(renderRegion);
    m_SBVRGeogen.DisableClipPlane();
  }
}

void GLSBVR2D::Render3DPreLoop(RenderRegion3D& region) {

  m_SBVRGeogen.SetSamplingModifier(m_fSampleRateModifier / ((region.decreaseSamplingRateNow) ? m_fSampleDecFactor : 1.0f));

  if(m_bClipPlaneOn) {
    m_SBVRGeogen.EnableClipPlane();
    PLANE<float> plane(m_ClipPlane.Plane());
    m_SBVRGeogen.SetClipPlane(plane);
  } else {
    m_SBVRGeogen.DisableClipPlane();
  }

  switch (m_eRenderMode) {
    case RM_1DTRANS    :  m_p1DTransTex->Bind(1);
                          m_pProgram1DTrans[m_bUseLighting ? 1 : 0]->Enable();
                          glEnable(GL_BLEND);
                          glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                          break;
    case RM_2DTRANS    :  m_p2DTransTex->Bind(1);
                          m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->Enable();
                          glEnable(GL_BLEND);
                          glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                          break;
    case RM_ISOSURFACE :  if (m_bAvoidSeperateCompositing) {
                            if (m_pDataset->GetComponentCount() == 1)
                              m_pProgramIsoNoCompose->Enable();
                            else
                              m_pProgramColorNoCompose->Enable();
                            glEnable(GL_BLEND);
                            glBlendFunc(GL_ONE_MINUS_DST_ALPHA, GL_ONE);
                          } else {
                            glEnable(GL_DEPTH_TEST);
                          }
                          break;
    default    :  T_ERROR("Invalid rendermode set");
                          break;
  }

  m_SBVRGeogen.SetLODData( UINTVECTOR3(m_pDataset->GetDomainSize(m_iCurrentLOD))  );
}

void GLSBVR2D::RenderProxyGeometry() {
  if (!m_pGLVolume) {
    T_ERROR("Volume data invalid, unable to render.");
    return;
  }

  if (m_bUse3DTexture) RenderProxyGeometry3D(); else RenderProxyGeometry2D();
}

void GLSBVR2D::RenderProxyGeometry2D() {
  GLVolume2DTex* pGLVolume =  static_cast<GLVolume2DTex*>(m_pGLVolume);

  for (UINT32 i = 0;i<3;i++) {

    switch (m_SBVRGeogen.m_vSliceTrianglesOrder[i]) {
      case SBVRGeogen2D::DIRECTION_X : {

        // set coordinate shuffle matrix
        float m[16] = {0,0,1,0,
                       0,1,0,0,
                       1,0,0,0,
                       0,0,0,1};
        glActiveTextureARB(GL_TEXTURE0);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glLoadMatrixf(m);

        int iLastTexID = -1;
        glBegin(GL_TRIANGLES);
        for (size_t i = 0;i<m_SBVRGeogen.m_vSliceTrianglesX.size();i++) {
          float depth = m_SBVRGeogen.m_vSliceTrianglesX[i].m_vTex.x;
          int iCurrentTexID =  int(depth*pGLVolume->GetSizeX());
          if (iCurrentTexID != iLastTexID) {
            glEnd();        
            pGLVolume->Bind(0, iCurrentTexID, 0);
            pGLVolume->Bind(2, iCurrentTexID+1, 0); 
            iLastTexID = iCurrentTexID;
            glBegin(GL_TRIANGLES);
          }

          float fraction = depth*pGLVolume->GetSizeX() - iCurrentTexID;

          glTexCoord3f(m_SBVRGeogen.m_vSliceTrianglesX[i].m_vTex.z,
                       m_SBVRGeogen.m_vSliceTrianglesX[i].m_vTex.y,
                       fraction);
          glVertex3f(m_SBVRGeogen.m_vSliceTrianglesX[i].m_vPos.x,
                     m_SBVRGeogen.m_vSliceTrianglesX[i].m_vPos.y,
                     m_SBVRGeogen.m_vSliceTrianglesX[i].m_vPos.z);
        }
        glEnd();
      } break;
      case SBVRGeogen2D::DIRECTION_Y : {

        // set coordinate shuffle matrix
        float m[16] = {1,0,0,0,
                       0,0,1,0,
                       0,1,0,0,
                       0,0,0,1};
        glActiveTextureARB(GL_TEXTURE0);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glLoadMatrixf(m);


        int iLastTexID = -1;
        glBegin(GL_TRIANGLES);
        for (size_t i = 0;i<m_SBVRGeogen.m_vSliceTrianglesY.size();i++) {
          float depth = m_SBVRGeogen.m_vSliceTrianglesY[i].m_vTex.y;
          int iCurrentTexID =  int(depth*pGLVolume->GetSizeY());
          if (iCurrentTexID != iLastTexID) {
            glEnd();        
            pGLVolume->Bind(0, iCurrentTexID, 1);
            pGLVolume->Bind(2, iCurrentTexID+1, 1); 
            iLastTexID = iCurrentTexID;
            glBegin(GL_TRIANGLES);
          }

          float fraction = depth*pGLVolume->GetSizeY() - iCurrentTexID;

          glTexCoord3f(m_SBVRGeogen.m_vSliceTrianglesY[i].m_vTex.x,
                       m_SBVRGeogen.m_vSliceTrianglesY[i].m_vTex.z,
                       fraction);
          glVertex3f(m_SBVRGeogen.m_vSliceTrianglesY[i].m_vPos.x,
                     m_SBVRGeogen.m_vSliceTrianglesY[i].m_vPos.y,
                     m_SBVRGeogen.m_vSliceTrianglesY[i].m_vPos.z);
        }
        glEnd();
      } break;
      case SBVRGeogen2D::DIRECTION_Z : {

        // set coordinate shuffle matrix
        glActiveTextureARB(GL_TEXTURE0);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();


        int iLastTexID = -1;
        glBegin(GL_TRIANGLES);
        for (size_t i = 0;i<m_SBVRGeogen.m_vSliceTrianglesZ.size();i++) {
          float depth = m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vTex.z;
          int iCurrentTexID =  int(depth*pGLVolume->GetSizeZ());
          if (iCurrentTexID != iLastTexID) {
            glEnd();        
            pGLVolume->Bind(0, iCurrentTexID, 2);
            pGLVolume->Bind(2, iCurrentTexID+1, 2);
            iLastTexID = iCurrentTexID;
            glBegin(GL_TRIANGLES);
          }

          float fraction = depth*pGLVolume->GetSizeZ() - iCurrentTexID;

          glTexCoord3f(m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vTex.x,
                       m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vTex.y,
                       fraction);
          glVertex3f(m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vPos.x,
                     m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vPos.y,
                     m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vPos.z);
        }
        glEnd();
      } break;
    }
  }
}

void GLSBVR2D::RenderProxyGeometry3D() {

  for (UINT32 i = 0;i<3;i++) {

    switch (m_SBVRGeogen.m_vSliceTrianglesOrder[i]) {
      case SBVRGeogen2D::DIRECTION_X : {
                                          glBegin(GL_TRIANGLES);
                                            for (size_t i = 0;i<m_SBVRGeogen.m_vSliceTrianglesX.size();i++) {
                                              glTexCoord3f(m_SBVRGeogen.m_vSliceTrianglesX[i].m_vTex.x,
                                                           m_SBVRGeogen.m_vSliceTrianglesX[i].m_vTex.y,
                                                           m_SBVRGeogen.m_vSliceTrianglesX[i].m_vTex.z);
                                              glVertex3f(m_SBVRGeogen.m_vSliceTrianglesX[i].m_vPos.x,
                                                         m_SBVRGeogen.m_vSliceTrianglesX[i].m_vPos.y,
                                                         m_SBVRGeogen.m_vSliceTrianglesX[i].m_vPos.z);
                                            }
                                          glEnd();
                                       } break;
      case SBVRGeogen2D::DIRECTION_Y : {
                                          glBegin(GL_TRIANGLES);
                                            for (size_t i = 0;i<m_SBVRGeogen.m_vSliceTrianglesY.size();i++) {
                                              glTexCoord3f(m_SBVRGeogen.m_vSliceTrianglesY[i].m_vTex.x,
                                                           m_SBVRGeogen.m_vSliceTrianglesY[i].m_vTex.y,
                                                           m_SBVRGeogen.m_vSliceTrianglesY[i].m_vTex.z);
                                              glVertex3f(m_SBVRGeogen.m_vSliceTrianglesY[i].m_vPos.x,
                                                         m_SBVRGeogen.m_vSliceTrianglesY[i].m_vPos.y,
                                                         m_SBVRGeogen.m_vSliceTrianglesY[i].m_vPos.z);
                                            }
                                          glEnd();
                                       } break;
      case SBVRGeogen2D::DIRECTION_Z : {
                                          glBegin(GL_TRIANGLES);
                                            for (size_t i = 0;i<m_SBVRGeogen.m_vSliceTrianglesZ.size();i++) {
                                              glTexCoord3f(m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vTex.x,
                                                           m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vTex.y,
                                                           m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vTex.z);
                                              glVertex3f(m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vPos.x,
                                                         m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vPos.y,
                                                         m_SBVRGeogen.m_vSliceTrianglesZ[i].m_vPos.z);
                                            }
                                          glEnd();
                                       } break;
    }
  }



}

void GLSBVR2D::Render3DInLoop(RenderRegion3D& renderRegion,
                              size_t iCurrentBrick, int iStereoID) {
  const Brick& b = (iStereoID == 0) ? m_vCurrentBrickList[iCurrentBrick] : m_vLeftEyeBrickList[iCurrentBrick];

  // setup the slice generator
  m_SBVRGeogen.SetBrickData(b.vExtension, b.vVoxelCount,
                            b.vTexcoordsMin, b.vTexcoordsMax);
  FLOATMATRIX4 maBricktTrans;
  maBricktTrans.Translation(b.vCenter.x, b.vCenter.y, b.vCenter.z);
  FLOATMATRIX4 maBricktModelView = maBricktTrans * renderRegion.modelView[iStereoID];
  m_mProjection[iStereoID].setProjection();
  maBricktModelView.setModelview();

  m_SBVRGeogen.SetWorld(maBricktTrans * renderRegion.rotation * renderRegion.translation);
  m_SBVRGeogen.SetView(m_mView[iStereoID], true);

  if (! m_bAvoidSeperateCompositing && m_eRenderMode == RM_ISOSURFACE) {
    glDisable(GL_BLEND);
    GLSLProgram* shader = (m_pDataset->GetComponentCount() == 1) ? m_pProgramIso : m_pProgramColor;

    m_TargetBinder.Bind(m_pFBOIsoHit[iStereoID], 0, m_pFBOIsoHit[iStereoID], 1);

    if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shader->Enable();
    SetBrickDepShaderVars(renderRegion, b);
    shader->SetUniformVector("fIsoval", static_cast<float>
                                        (this->GetNormalizedIsovalue()));
    RenderProxyGeometry();
    shader->Disable();

    if (m_bDoClearView) {
      m_TargetBinder.Bind(m_pFBOCVHit[iStereoID], 0, m_pFBOCVHit[iStereoID], 1);

      if (m_iBricksRenderedInThisSubFrame == 0) glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      m_pProgramIso->Enable();
      m_pProgramIso->SetUniformVector("fIsoval", static_cast<float>
                                                 (GetNormalizedCVIsovalue()));
      RenderProxyGeometry();
      m_pProgramIso->Disable();
    }
  } else {
    m_TargetBinder.Bind(m_pFBO3DImageCurrent[iStereoID]);

    glDepthMask(GL_FALSE);
    SetBrickDepShaderVars(renderRegion, b);
    RenderProxyGeometry();
    glDepthMask(GL_TRUE);
  }
  m_TargetBinder.Unbind();
}


void GLSBVR2D::Render3DPostLoop() {
  GLRenderer::Render3DPostLoop();

  // disable the shader
  switch (m_eRenderMode) {
    case RM_1DTRANS    :  m_pProgram1DTrans[m_bUseLighting ? 1 : 0]->Disable();
                          glDisable(GL_BLEND);
                          break;
    case RM_2DTRANS    :  m_pProgram2DTrans[m_bUseLighting ? 1 : 0]->Disable();
                          glDisable(GL_BLEND);
                          break;
    case RM_ISOSURFACE :  if (m_bAvoidSeperateCompositing) {
                            if (m_pDataset->GetComponentCount() == 1)
                              m_pProgramIsoNoCompose->Disable();
                            else
                              m_pProgramColorNoCompose->Disable();
                             glDisable(GL_BLEND);
                          }
                          break;
    case RM_INVALID    :  T_ERROR("Invalid rendermode set"); break;
  }
}

void GLSBVR2D::RenderHQMIPPreLoop(RenderRegion2D& region) {
  GLRenderer::RenderHQMIPPreLoop(region);
  m_pProgramHQMIPRot->Enable();

  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_MAX);
  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
}

void GLSBVR2D::RenderHQMIPInLoop(RenderRegion2D&, const Brick& b) {
  m_SBVRGeogen.SetBrickData(b.vExtension, b.vVoxelCount, b.vTexcoordsMin, b.vTexcoordsMax);
  FLOATMATRIX4 maBricktTrans;
  maBricktTrans.Translation(b.vCenter.x, b.vCenter.y, b.vCenter.z);
  if (m_bOrthoView) {
    // here we push the volume back by one to make sure 
    // the viewing direction computation in the geometry generator
    // works 
    FLOATMATRIX4 m;
    m.Translation(0,0,1);
    m_SBVRGeogen.SetView(m);
 }  else
    m_SBVRGeogen.SetView(m_mView[0]);

  m_SBVRGeogen.SetWorld(maBricktTrans * m_maMIPRotation, true);

  RenderProxyGeometry();
}

void GLSBVR2D::RenderHQMIPPostLoop() {
  GLRenderer::RenderHQMIPPostLoop();
  m_pProgramHQMIPRot->Disable();
}


bool GLSBVR2D::LoadDataset(const string& strFilename) {
  if (GLRenderer::LoadDataset(strFilename)) {
    UINTVECTOR3    vSize = UINTVECTOR3(m_pDataset->GetDomainSize());
    FLOATVECTOR3 vAspect = FLOATVECTOR3(m_pDataset->GetScale());
    vAspect /= vAspect.maxVal();

    m_SBVRGeogen.SetVolumeData(vAspect, vSize);
    return true;
  } else return false;
}

void GLSBVR2D::ComposeSurfaceImage(RenderRegion& renderRegion, int iStereoID) {
  if (!m_bAvoidSeperateCompositing)
    GLRenderer::ComposeSurfaceImage(renderRegion, iStereoID);
}


void GLSBVR2D::UpdateColorsInShaders() {
  GLRenderer::UpdateColorsInShaders();

  FLOATVECTOR3 a = m_cAmbient.xyz()*m_cAmbient.w;
  FLOATVECTOR3 d = m_cDiffuse.xyz()*m_cDiffuse.w;
  FLOATVECTOR3 s = m_cSpecular.xyz()*m_cSpecular.w;
  FLOATVECTOR3 dir(0.0f,0.0f,-1.0f);  // so far the light source is always a headlight

  FLOATVECTOR3 scale = 1.0f/FLOATVECTOR3(m_pDataset->GetScale());

  m_pProgramIsoNoCompose->Enable();
  m_pProgramIsoNoCompose->SetUniformVector("vLightAmbient",a.x,a.y,a.z);
  m_pProgramIsoNoCompose->SetUniformVector("vLightDiffuse",d.x,d.y,d.z);
  m_pProgramIsoNoCompose->SetUniformVector("vLightSpecular",s.x,s.y,s.z);
  m_pProgramIsoNoCompose->SetUniformVector("vLightDir",dir.x,dir.y,dir.z);
  m_pProgramIsoNoCompose->SetUniformVector("vDomainScale",scale.x,scale.y,scale.z);
  m_pProgramIsoNoCompose->Disable();

  m_pProgramColorNoCompose->Enable();
  m_pProgramColorNoCompose->SetUniformVector("vLightAmbient",a.x,a.y,a.z);
  m_pProgramColorNoCompose->SetUniformVector("vLightDir",dir.x,dir.y,dir.z);
  m_pProgramColorNoCompose->SetUniformVector("vDomainScale",scale.x,scale.y,scale.z);
  m_pProgramColorNoCompose->Disable();
}

bool GLSBVR2D::BindVolumeTex(const BrickKey& bkey,
                             const UINT64 iIntraFrameCounter) {

  if (m_bUse3DTexture) return GLRenderer::BindVolumeTex(bkey,iIntraFrameCounter);

  m_pGLVolume = m_pMasterController->MemMan()->GetVolume(m_pDataset, bkey,
                                            m_bUseOnlyPowerOfTwo, 
                                            m_bDownSampleTo8Bits, 
                                            m_bDisableBorder,
                                            true,
                                            iIntraFrameCounter, 
                                            m_iFrameCounter);
  if(m_pGLVolume) {
    return true;
  } else {
    return false;
  }
}

bool GLSBVR2D::IsVolumeResident(const BrickKey& key) {
  if (m_bUse3DTexture) return GLRenderer::IsVolumeResident(key);

  return m_pMasterController->MemMan()->IsResident(m_pDataset, key,
                                                m_bUseOnlyPowerOfTwo,
                                                m_bDownSampleTo8Bits,
                                                m_bDisableBorder,
                                                true);
}

void GLSBVR2D::RenderSlice(const RenderRegion2D& region, double fSliceIndex,
                             FLOATVECTOR3 vMinCoords, FLOATVECTOR3 vMaxCoords,
                             DOUBLEVECTOR3 vAspectRatio,
                             DOUBLEVECTOR2 vWinAspectRatio) {
  GLVolume2DTex* pGLVolume =  static_cast<GLVolume2DTex*>(m_pGLVolume);
  switch (region.windowMode) {
    case RenderRegion::WM_AXIAL :
    {
      if (region.flipView.x) {
        float fTemp = vMinCoords.x;
        vMinCoords.x = vMaxCoords.x;
        vMaxCoords.x = fTemp;
      }

      if (region.flipView.y) {
        float fTemp = vMinCoords.z;
        vMinCoords.z = vMaxCoords.z;
        vMaxCoords.z = fTemp;
      }

      int iCurrentTexID =  int(fSliceIndex*pGLVolume->GetSizeY());
      pGLVolume->Bind(0, iCurrentTexID, 1);
      pGLVolume->Bind(2, iCurrentTexID+1, 1); 
      float fraction = float(fSliceIndex*pGLVolume->GetSizeY() - iCurrentTexID);

      DOUBLEVECTOR2 v2AspectRatio = vAspectRatio.xz()*DOUBLEVECTOR2(vWinAspectRatio);
      v2AspectRatio = v2AspectRatio / v2AspectRatio.maxVal();
      glBegin(GL_QUADS);
      glTexCoord3d(vMinCoords.x,vMaxCoords.z,fraction);
      glVertex3d(-1.0f*v2AspectRatio.x, +1.0f*v2AspectRatio.y, -0.5f);
      glTexCoord3d(vMaxCoords.x,vMaxCoords.z,fraction);
      glVertex3d(+1.0f*v2AspectRatio.x, +1.0f*v2AspectRatio.y, -0.5f);
      glTexCoord3d(vMaxCoords.x,vMinCoords.z,fraction);
      glVertex3d(+1.0f*v2AspectRatio.x, -1.0f*v2AspectRatio.y, -0.5f);
      glTexCoord3d(vMinCoords.x,vMinCoords.z,fraction);
      glVertex3d(-1.0f*v2AspectRatio.x, -1.0f*v2AspectRatio.y, -0.5f);
      glEnd();
      break;
    }
  case RenderRegion::WM_CORONAL :
    {
      if (region.flipView.x) {
        float fTemp = vMinCoords.x;
        vMinCoords.x = vMaxCoords.x;
        vMaxCoords.x = fTemp;
      }

      if (region.flipView.y) {
        float fTemp = vMinCoords.y;
        vMinCoords.y = vMaxCoords.y;
        vMaxCoords.y = fTemp;
      }

      DOUBLEVECTOR2 v2AspectRatio = vAspectRatio.xy()*DOUBLEVECTOR2(vWinAspectRatio);
      v2AspectRatio = v2AspectRatio / v2AspectRatio.maxVal();

      int iCurrentTexID =  int(fSliceIndex*pGLVolume->GetSizeZ());
      pGLVolume->Bind(0, iCurrentTexID, 2);
      pGLVolume->Bind(2, iCurrentTexID+1, 2); 
      float fraction = float(fSliceIndex*pGLVolume->GetSizeZ() - iCurrentTexID);

      glBegin(GL_QUADS);
      glTexCoord3d(vMinCoords.x,vMaxCoords.y,fraction);
      glVertex3d(-1.0f*v2AspectRatio.x, +1.0f*v2AspectRatio.y, -0.5f);
      glTexCoord3d(vMaxCoords.x,vMaxCoords.y,fraction);
      glVertex3d(+1.0f*v2AspectRatio.x, +1.0f*v2AspectRatio.y, -0.5f);
      glTexCoord3d(vMaxCoords.x,vMinCoords.y,fraction);
      glVertex3d(+1.0f*v2AspectRatio.x, -1.0f*v2AspectRatio.y, -0.5f);
      glTexCoord3d(vMinCoords.x,vMinCoords.y,fraction);
      glVertex3d(-1.0f*v2AspectRatio.x, -1.0f*v2AspectRatio.y, -0.5f);
      glEnd();
      break;
    }
  case RenderRegion::WM_SAGITTAL :
    {
      if (region.flipView.x) {
        float fTemp = vMinCoords.y;
        vMinCoords.y = vMaxCoords.y;
        vMaxCoords.y = fTemp;
      }

      if (region.flipView.y) {
        float fTemp = vMinCoords.z;
        vMinCoords.z = vMaxCoords.z;
        vMaxCoords.z = fTemp;
      }

      int iCurrentTexID =  int(fSliceIndex*pGLVolume->GetSizeX());
      pGLVolume->Bind(0, iCurrentTexID, 0);
      pGLVolume->Bind(2, iCurrentTexID+1, 0); 
      float fraction = float(fSliceIndex*pGLVolume->GetSizeX() - iCurrentTexID);

      DOUBLEVECTOR2 v2AspectRatio = vAspectRatio.yz()*DOUBLEVECTOR2(vWinAspectRatio);
      v2AspectRatio = v2AspectRatio / v2AspectRatio.maxVal();
      glBegin(GL_QUADS);
      glTexCoord3d(vMaxCoords.z,vMinCoords.y,fraction);
      glVertex3d(-1.0f*v2AspectRatio.x, +1.0f*v2AspectRatio.y, -0.5f);
      glTexCoord3d(vMaxCoords.z,vMaxCoords.y,fraction);
      glVertex3d(+1.0f*v2AspectRatio.x, +1.0f*v2AspectRatio.y, -0.5f);
      glTexCoord3d(vMinCoords.z,vMaxCoords.y,fraction);
      glVertex3d(+1.0f*v2AspectRatio.x, -1.0f*v2AspectRatio.y, -0.5f);
      glTexCoord3d(vMinCoords.z,vMinCoords.y,fraction);
      glVertex3d(-1.0f*v2AspectRatio.x, -1.0f*v2AspectRatio.y, -0.5f);
      glEnd();
      break;
    }
  default :  T_ERROR("Invalid windowmode set"); break;
  }
}
