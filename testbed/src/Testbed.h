// Testbed tfgl::App derivative.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//


#include "App.h"


#include <memory>


namespace tfgl {
    class Buffer;
    class Program;
    class VertexArrayObject;    
}


namespace tft {

    class Testbed : public tfgl::App {
    public:
        // Can't create any OpenGL objects until OpenGL has been initialized.
        Testbed() = default;

        // for pimpl.
        ~Testbed();

    private:
        std::unique_ptr<tfgl::Program>              program_;
        std::unique_ptr<tfgl::VertexArrayObject>    vao_;

        // It might not be strictly necessary to hold this buffer.  I like
        // the analog that this object represents some state that when this
        // object is destroyed, so is the object in the video card.
        std::unique_ptr<tfgl::Buffer>               buf_;

        virtual std::string GetVersion() const override { return "Testbed 1.0"; }

        virtual bool InitImpl() override;
        virtual bool DrawImpl() override;
    };

}
