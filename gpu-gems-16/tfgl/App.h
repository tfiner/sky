// App class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#pragma once

#include <memory>
#include <string>


struct GLFWwindow;

namespace tfgl {


    // A small application shell that loads OpenGL, 
    // starts up a window, etc.  Inherit and override.
    class App {
    public:
       App() = default;
        App(const App&) = delete;
        App& operator=(const App&) = delete;
        ~App() = default;

        // This is the top level call that drives the application.
        // This function calls InitImpl and loops on DrawImpl:
        // if (InitImpl())
        //      while(DrawImpl()) {}
        void Run(int argc, char** argv);

    protected:
       GLFWwindow* window_ = nullptr;

    private:
        int screenWidth_    = 800;
        int screenHeight_   = 600;
       
        // Top level initialize that ends up calling InitImpl.
        bool Init(int argc, char** argv);
        void InitGL();
        bool Draw() { return DrawImpl(); }

        // Overrides

        virtual std::string GetVersion() const { return "App 1.0"; }

        virtual bool InitImpl() { return true; }
        virtual bool DrawImpl() { return true; } 

        virtual void OnKeyImpl() {}
    };


}
