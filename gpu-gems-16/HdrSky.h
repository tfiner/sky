// HDR Sky class.
// 
// Combines a floating point texture and an HDR shader from GPU Gems II, Chapter 16.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//


#pragma once

#include <memory>


// class PBuffer;

namespace tfgl {
   class Program;
   class FrameBuffer;
}

namespace sky {


   class HdrSky {
   public:
      HdrSky();
      ~HdrSky();

      void Init(int width, int height, int flags);

      float GetExposure() const { return exposure_; }
      void SetExposure(float exposure) { exposure_ = exposure;  }
      void SetContext() const;

      void Bind() const;
      void Unbind() const;

      void BindTexture() const;
      void UnbindTexture() const;


   private:
      std::unique_ptr<tfgl::FrameBuffer> buffer_;
//      std::unique_ptr<PBuffer> pBuffer_;
      float exposure_;

      std::unique_ptr<tfgl::Program> exposureProg_;
   };


}

