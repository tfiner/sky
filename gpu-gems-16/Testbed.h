// Testbed tfgl::App derivative.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//


#include "tfgl/App.h"


#include <memory>

class CGameEngine;

namespace tfgl {
    class Buffer;
    class Program;
    class VertexArrayObject;    
}


namespace tft {

    class Testbed : public tfgl::App {
    public:
        // Can't create any OpenGL objects until OpenGL has been initialized.
        Testbed();

        // for pimpl.
        ~Testbed();

    private:
        // std::unique_ptr<tfgl::Program>              program_;
       std::unique_ptr<CGameEngine> gameEngine_;

        virtual std::string GetVersion() const override { return "Testbed 1.0"; }

        virtual bool InitImpl() override;
        virtual bool DrawImpl() override;
    };

}
