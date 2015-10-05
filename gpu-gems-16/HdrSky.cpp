// HDR Sky class.
// 
// Combines a floating point texture and an HDR shader from GPU Gems II, Chapter 16.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include "HdrSky.h"
#include "PBuffer.h"

#include <tfgl/Program.h>
// #include <tfgl/FrameBuffer.h>

#include <GLFW/glfw3.h>


using namespace sky;


HdrSky::HdrSky() : pBuffer_(std::make_unique<PBuffer>()) {}


// For pimpl
HdrSky::~HdrSky() {}


void HdrSky::Init(int width, int height, int flags) {
   // tfgl::FrameBuffer fb(256, 256, GL_RGB, GL_FLOAT);

   pBuffer_->Init(1024, 1024, 0);

   pBuffer_->MakeCurrent();
   ::glEnable(GL_DEPTH_TEST);
   ::glDepthFunc(GL_LEQUAL);
   ::glEnable(GL_CULL_FACE);

   exposureProg_ = tfgl::MakeProgram("HDR.vert", "HDRRect.frag");
}


void HdrSky::SetContext() const {
   pBuffer_->MakeCurrent();
   ::glViewport(0, 0, 1024, 1024);
}


void HdrSky::Bind() const {
   exposureProg_->Bind();
   exposureProg_->SetUniform("s2Test", 0);
   exposureProg_->SetUniform("fExposure", exposure_);

   pBuffer_->BindTexture(exposure_, true);
}

void HdrSky::Unbind() const {
   pBuffer_->ReleaseTexture();
   exposureProg_->Unbind();
}

