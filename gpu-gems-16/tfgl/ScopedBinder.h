// Program class, part of a minimal OpenGL library.
//
// Author:  Tim Finer 
// Email:   tfiner@csu.fullerton.edu
// 
// CPSC-597 Fall 2015 Master's Project
//

#pragma once

namespace tfgl {


    // Unbinds objects using scoped.
    template<typename Object>
    class ScopedBinder {        
    public:
        ScopedBinder(Object& obj) : obj_(obj) {
            obj_.Bind();
        }
        
        ScopedBinder(const ScopedBinder&) = delete; 
        ScopedBinder& operator=(const ScopedBinder&) = delete; 

        ~ScopedBinder() {
            obj_.Unbind();
        } 

    private:
        Object& obj_;
    };


}