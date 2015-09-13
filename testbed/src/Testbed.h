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
}


namespace tft {

    class Testbed : public tfgl::App {
    public:
        Testbed() = default;

        // for pimpl.
        ~Testbed();

    private:
        std::unique_ptr<tfgl::Buffer> vbo_;

        virtual std::string GetVersion() const override { return "Testbed 1.0"; }

        virtual bool InitImpl() override;
        virtual bool DrawImpl() override;
    };

}
