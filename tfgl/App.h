// App class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#include <memory>
#include <string>

namespace boost{
    namespace program_options {
        class variables_map;
    }
}

class GLFWwindow;

namespace tfgl {


    // A small application shell that loads OpenGL, 
    // starts up a window, etc.  Inherit and override.
    class App {
    public:
        App();

        App(const App&) = delete;
        App& operator=(const App&) = delete;
        ~App() = default;

        // This is the top level call that drives the application.
        // This function calls InitImpl and loops on DrawImpl:
        // if (InitImpl())
        //      while(DrawImpl()) {}
        void Run(int argc, char** argv);

        const boost::program_options::variables_map& GetSettings() const {
            return *settings_;
        }

    private:
        int screenWidth_, screenHeight_;
        GLFWwindow* window_;
        std::unique_ptr<boost::program_options::variables_map> settings_;

        // Top level initialize that ends up calling InitImpl.
        bool Init(int argc, char** argv);
        void InitGL();
        bool Draw() { return DrawImpl(); }

        // Overrides

        virtual std::string GetVersion() const { return "App 1.0"; }

        virtual bool InitImpl() { return true; }
        virtual bool DrawImpl() { return true; } 
    };


}
