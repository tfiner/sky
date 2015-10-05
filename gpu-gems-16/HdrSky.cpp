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
// #include "PBuffer.h"

#include <tfgl/Program.h>
#include <tfgl/FrameBuffer.h>

#include <GLFW/glfw3.h>


using namespace sky;


HdrSky::HdrSky() : buffer_(std::make_unique<tfgl::FrameBuffer>(1024, 1024, GL_RGBA, GL_FLOAT)) {}


// For pimpl
HdrSky::~HdrSky() {}


void HdrSky::Init(int width, int height, int flags) {
   // buffer_->Init(1024, 1024, GL_RGBA, GL_FLOAT);
   // pBuffer_->MakeCurrent();
   exposureProg_ = tfgl::MakeProgram("HDR.vert", "HDRRect.frag");
}


void HdrSky::SetContext() const {
   // pBuffer_->MakeCurrent();

   ::glEnable(GL_DEPTH_TEST);
   ::glDepthFunc(GL_LEQUAL);
   ::glEnable(GL_CULL_FACE);
}


void HdrSky::Bind() const {
   exposureProg_->Bind();
   exposureProg_->SetUniform("s2Test", 0);
   exposureProg_->SetUniform("fExposure", exposure_);

   buffer_->Bind();
}

void HdrSky::Unbind() const {
   buffer_->Unbind();
   exposureProg_->Unbind();
}



void HdrSky::BindTexture() const {
   buffer_->BindTexture();
}


void HdrSky::UnbindTexture() const {
   buffer_->UnbindTexture();
}

