#include "Utility.h"
#include "Exception.h"

#include <glsw.h>

#include <string.h>
#include <math.h>
#include <assert.h>

using namespace vmath;

struct ProgramsRec {
    GLuint Advect;
    GLuint Jacobi;
    GLuint SubtractGradient;
    GLuint ComputeDivergence;
    GLuint ApplyImpulse;
    GLuint ApplyBuoyancy;
} Programs;

void CreateObstacles(SurfacePod dest, int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glViewport(0, 0, width, height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    GLuint program = LoadProgram("Fluid.Vertex", 0, "Fluid.Fill");
    glUseProgram(program);

    const int DrawBorder = 1;
    if (DrawBorder) {
        #define T 0.9999f
        float positions[] = { -T, -T, T, -T, T,  T, -T,  T, -T, -T };
        #undef T
        GLuint vbo;
        GLsizeiptr size = sizeof(positions);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);
        GLsizeiptr stride = 2 * sizeof(positions[0]);
        glEnableVertexAttribArray(SlotPosition);
        glVertexAttribPointer(SlotPosition, 2, GL_FLOAT, GL_FALSE, stride, 0);
        glDrawArrays(GL_LINE_STRIP, 0, 5);
        glDeleteBuffers(1, &vbo);
    }

    const int DrawCircle = 1;
    if (DrawCircle) {
        const int slices = 64;
        float positions[slices*2*3];
        float twopi = 8*atan(1.0f);
        float theta = 0;
        float dtheta = twopi / (float) (slices - 1);
        float* pPositions = &positions[0];
        for (int i = 0; i < slices; i++) {
            *pPositions++ = 0;
            *pPositions++ = 0;

            *pPositions++ = 0.25f * cos(theta) * height / width;
            *pPositions++ = 0.25f * sin(theta);
            theta += dtheta;

            *pPositions++ = 0.25f * cos(theta) * height / width;
            *pPositions++ = 0.25f * sin(theta);
        }
        GLuint vbo;
        GLsizeiptr size = sizeof(positions);
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, size, positions, GL_STATIC_DRAW);
        GLsizeiptr stride = 2 * sizeof(positions[0]);
        glEnableVertexAttribArray(SlotPosition);
        glVertexAttribPointer(SlotPosition, 2, GL_FLOAT, GL_FALSE, stride, 0);
        glDrawArrays(GL_TRIANGLES, 0, slices * 3);
        glDeleteBuffers(1, &vbo);
    }

    // Cleanup
    glDeleteProgram(program);
    glDeleteVertexArrays(1, &vao);

}

GLuint LoadProgram(const char* vsKey, const char* gsKey, const char* fsKey)
{
    static int first = 1;
    if (first) {
        glswInit();
        glswAddPath("../", ".glsl");
        glswAddPath("./", ".glsl");

        char qualifiedPath[128];
        strcpy(qualifiedPath, PezResourcePath());
        strcat(qualifiedPath, "/");
        glswAddPath(qualifiedPath, ".glsl");

        glswAddDirective("*", "#version 150");

        first = 0;
    }
    
    const char* vsSource = glswGetShader(vsKey);
    const char* gsSource = glswGetShader(gsKey);
    const char* fsSource = glswGetShader(fsKey);

    const char* msg = "Can't find %s shader: '%s'.\n";
    PezCheckCondition(vsSource != 0, msg, "vertex", vsKey);
    PezCheckCondition(gsKey == 0 || gsSource != 0, msg, "geometry", gsKey);
    PezCheckCondition(fsKey == 0 || fsSource != 0, msg, "fragment", fsKey);
    
    GLint compileSuccess;
    GLchar compilerSpew[256];
    GLuint programHandle = glCreateProgram();

    GLuint vsHandle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vsHandle, 1, &vsSource, 0);
    THROW_ON_GL_ERROR();

    glCompileShader(vsHandle);
    THROW_ON_GL_ERROR();

    glGetShaderiv(vsHandle, GL_COMPILE_STATUS, &compileSuccess);
    glGetShaderInfoLog(vsHandle, sizeof(compilerSpew), 0, compilerSpew);
    PezCheckCondition(compileSuccess, "Can't compile %s:\n%s", vsKey, compilerSpew);
    glAttachShader(programHandle, vsHandle);
    THROW_ON_GL_ERROR();

    GLuint gsHandle;
    if (gsKey) {
        gsHandle = glCreateShader(GL_GEOMETRY_SHADER);
        glShaderSource(gsHandle, 1, &gsSource, 0);
        glCompileShader(gsHandle);
        glGetShaderiv(gsHandle, GL_COMPILE_STATUS, &compileSuccess);
        glGetShaderInfoLog(gsHandle, sizeof(compilerSpew), 0, compilerSpew);
        PezCheckCondition(compileSuccess, "Can't compile %s:\n%s", gsKey, compilerSpew);
        glAttachShader(programHandle, gsHandle);
    }
    
    GLuint fsHandle;
    if (fsKey) {
        fsHandle = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fsHandle, 1, &fsSource, 0);
        glCompileShader(fsHandle);
        glGetShaderiv(fsHandle, GL_COMPILE_STATUS, &compileSuccess);
        glGetShaderInfoLog(fsHandle, sizeof(compilerSpew), 0, compilerSpew);
        PezCheckCondition(compileSuccess, "Can't compile %s:\n%s", fsKey, compilerSpew);
        glAttachShader(programHandle, fsHandle);
    }

    glBindAttribLocation(programHandle, SlotPosition, "Position");
    glBindAttribLocation(programHandle, SlotTexCoord, "TexCoord");
    glLinkProgram(programHandle);
    
    GLint linkSuccess;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkSuccess);
    glGetProgramInfoLog(programHandle, sizeof(compilerSpew), 0, compilerSpew);

    if (!linkSuccess) {
        PezDebugString("Link error.\n");
        if (vsKey) PezDebugString("Vertex Shader: %s\n", vsKey);
        if (gsKey) PezDebugString("Geometry Shader: %s\n", gsKey);
        if (fsKey) PezDebugString("Fragment Shader: %s\n", fsKey);
        PezDebugString("%s\n", compilerSpew);
    }
    
    return programHandle;
}

SlabPod CreateSlab(GLsizei width, GLsizei height)
{
    SlabPod slab;
    slab.Ping = CreateSurface(width, height);
    slab.Pong = CreateSurface(width, height);
    return slab;
}

SurfacePod CreateSurface(GLsizei width, GLsizei height)
{
    GLuint fboHandle;
    glGenFramebuffers(1, &fboHandle);
    glBindFramebuffer(GL_FRAMEBUFFER, fboHandle);

    GLuint textureHandle;
    glGenTextures(1, &textureHandle);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_HALF_FLOAT, 0);
    PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to create normals texture");

    GLuint colorbuffer;
    glGenRenderbuffers(1, &colorbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, colorbuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureHandle, 0);
    PezCheckCondition(GL_NO_ERROR == glGetError(), "Unable to attach color buffer");
    
    PezCheckCondition(GL_FRAMEBUFFER_COMPLETE == glCheckFramebufferStatus(GL_FRAMEBUFFER), "Unable to create FBO.");
    SurfacePod surface = { fboHandle, textureHandle };

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return surface;
}

static void ResetState()
{
    glActiveTexture(GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_BLEND);
}

void InitSlabOps()
{
    Programs.Advect = LoadProgram("Fluid.Vertex", 0, "Fluid.Advect");
    Programs.Jacobi = LoadProgram("Fluid.Vertex", 0, "Fluid.Jacobi");
    Programs.SubtractGradient = LoadProgram("Fluid.Vertex", 0, "Fluid.SubtractGradient");
    Programs.ComputeDivergence = LoadProgram("Fluid.Vertex", 0, "Fluid.ComputeDivergence");
    Programs.ApplyImpulse = LoadProgram("Fluid.Vertex", 0, "Fluid.Splat");
    Programs.ApplyBuoyancy = LoadProgram("Fluid.Vertex", 0, "Fluid.Buoyancy");
}

void SwapSurfaces(SlabPod* slab)
{
    SurfacePod temp = slab->Ping;
    slab->Ping = slab->Pong;
    slab->Pong = temp;
}

void ClearSurface(SurfacePod s, float v)
{
    glBindFramebuffer(GL_FRAMEBUFFER, s.FboHandle);
    glClearColor(v, v, v, v);
    glClear(GL_COLOR_BUFFER_BIT);
}
/*
void Advect(SurfacePod velocity, SurfacePod source, SurfacePod obstacles, SurfacePod dest, float dissipation)
{
    GLuint p = Programs.Advect;
    glUseProgram(p);

    GLint inverseSize = glGetUniformLocation(p, "InverseSize");
    GLint timeStep = glGetUniformLocation(p, "TimeStep");
    GLint dissLoc = glGetUniformLocation(p, "Dissipation");
    GLint sourceTexture = glGetUniformLocation(p, "SourceTexture");
    GLint obstaclesTexture = glGetUniformLocation(p, "Obstacles");

    glUniform2f(inverseSize, 1.0f / GridWidth, 1.0f / GridHeight);
    glUniform1f(timeStep, TimeStep);
    glUniform1f(dissLoc, dissipation);
    glUniform1i(sourceTexture, 1);
    glUniform1i(obstaclesTexture, 2);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocity.TextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, source.TextureHandle);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, obstacles.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void Jacobi(SurfacePod pressure, SurfacePod divergence, SurfacePod obstacles, SurfacePod dest)
{
    GLuint p = Programs.Jacobi;
    glUseProgram(p);

    GLint alpha = glGetUniformLocation(p, "Alpha");
    GLint inverseBeta = glGetUniformLocation(p, "InverseBeta");
    GLint dSampler = glGetUniformLocation(p, "Divergence");
    GLint oSampler = glGetUniformLocation(p, "Obstacles");

    glUniform1f(alpha, -CellSize * CellSize);
    glUniform1f(inverseBeta, 0.25f);
    glUniform1i(dSampler, 1);
    glUniform1i(oSampler, 2);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, pressure.TextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, divergence.TextureHandle);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, obstacles.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void SubtractGradient(SurfacePod velocity, SurfacePod pressure, SurfacePod obstacles, SurfacePod dest)
{
    GLuint p = Programs.SubtractGradient;
    glUseProgram(p);

    GLint gradientScale = glGetUniformLocation(p, "GradientScale");
    glUniform1f(gradientScale, GradientScale);
    GLint halfCell = glGetUniformLocation(p, "HalfInverseCellSize");
    glUniform1f(halfCell, 0.5f / CellSize);
    GLint sampler = glGetUniformLocation(p, "Pressure");
    glUniform1i(sampler, 1);
    sampler = glGetUniformLocation(p, "Obstacles");
    glUniform1i(sampler, 2);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocity.TextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, pressure.TextureHandle);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, obstacles.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void ComputeDivergence(SurfacePod velocity, SurfacePod obstacles, SurfacePod dest)
{
    GLuint p = Programs.ComputeDivergence;
    glUseProgram(p);

    GLint halfCell = glGetUniformLocation(p, "HalfInverseCellSize");
    glUniform1f(halfCell, 0.5f / CellSize);
    GLint sampler = glGetUniformLocation(p, "Obstacles");
    glUniform1i(sampler, 1);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocity.TextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, obstacles.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void ApplyImpulse(SurfacePod dest, Vector2 position, float value)
{
    GLuint p = Programs.ApplyImpulse;
    glUseProgram(p);

    GLint pointLoc = glGetUniformLocation(p, "Point");
    GLint radiusLoc = glGetUniformLocation(p, "Radius");
    GLint fillColorLoc = glGetUniformLocation(p, "FillColor");

    glUniform2f(pointLoc, (float) position.X, (float) position.Y);
    glUniform1f(radiusLoc, SplatRadius);
    glUniform3f(fillColorLoc, value, value, value);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glEnable(GL_BLEND);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}

void ApplyBuoyancy(SurfacePod velocity, SurfacePod temperature, SurfacePod density, SurfacePod dest)
{
    GLuint p = Programs.ApplyBuoyancy;
    glUseProgram(p);

    GLint tempSampler = glGetUniformLocation(p, "Temperature");
    GLint inkSampler = glGetUniformLocation(p, "Density");
    GLint ambTemp = glGetUniformLocation(p, "AmbientTemperature");
    GLint timeStep = glGetUniformLocation(p, "TimeStep");
    GLint sigma = glGetUniformLocation(p, "Sigma");
    GLint kappa = glGetUniformLocation(p, "Kappa");

    glUniform1i(tempSampler, 1);
    glUniform1i(inkSampler, 2);
    glUniform1f(ambTemp, AmbientTemperature);
    glUniform1f(timeStep, TimeStep);
    glUniform1f(sigma, SmokeBuoyancy);
    glUniform1f(kappa, SmokeWeight);

    glBindFramebuffer(GL_FRAMEBUFFER, dest.FboHandle);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, velocity.TextureHandle);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, temperature.TextureHandle);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, density.TextureHandle);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    ResetState();
}
*/
GLuint CreatePointVbo(float x, float y, float z)
{
    float p[] = {x, y, z};
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(p), &p[0], GL_STATIC_DRAW);
    return vbo;
}

void SetUniform(const char* name, int value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    if(location == -1) PezDebugString("UNIFORM: '%s' not found.\n", name);
    glUniform1i(location, value);
}

void SetUniform(const char* name, float value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    if(location == -1) PezDebugString("UNIFORM: '%s' not found.\n", name);
    glUniform1f(location, value);
}

void SetUniform(const char* name, double value)
{
   GLuint program;
   glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*)&program);
   GLint location = glGetUniformLocation(program, name);
   if(location == -1) PezDebugString("UNIFORM: '%s' not found.\n", name);
   glUniform1d(location, value);
}

void SetUniform(const char* name, Matrix4 value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    if(location == -1) PezDebugString("UNIFORM: '%s' not found.\n", name);
    glUniformMatrix4fv(location, 1, 0, (float*) &value);
}

void SetUniform(const char* name, Matrix3 nm)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    if(location == -1) PezDebugString("UNIFORM: '%s' not found.\n", name);
    float packed[9] = {
        nm.getRow(0).getX(), nm.getRow(1).getX(), nm.getRow(2).getX(),
        nm.getRow(0).getY(), nm.getRow(1).getY(), nm.getRow(2).getY(),
        nm.getRow(0).getZ(), nm.getRow(1).getZ(), nm.getRow(2).getZ() };
    glUniformMatrix3fv(location, 1, 0, &packed[0]);
}

void SetUniform(const char* name, Vector3 value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    if(location == -1) PezDebugString("UNIFORM: '%s' not found.\n", name);
    glUniform3f(location, value.getX(), value.getY(), value.getZ());
}

void SetUniform(const char* name, float x, float y)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    if(location == -1) PezDebugString("UNIFORM: '%s' not found.\n", name);
    glUniform2f(location, x, y);
}

void SetUniform(const char* name, Vector4 value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    if(location == -1) PezDebugString("UNIFORM: '%s' not found.\n", name);
    glUniform4f(location, value.getX(), value.getY(), value.getZ(), value.getW());
}

void SetUniform(const char* name, Point3 value)
{
    GLuint program;
    glGetIntegerv(GL_CURRENT_PROGRAM, (GLint*) &program);
    GLint location = glGetUniformLocation(program, name);
    if(location == -1) PezDebugString("UNIFORM: '%s' not found.\n", name);
    glUniform3f(location, value.getX(), value.getY(), value.getZ());
}


void DumpUniforms() {
   GLint program;
   glGetIntegerv(GL_CURRENT_PROGRAM, &program);

   GLint numUniforms = 0;
   ::glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &numUniforms);

   GLint bufSize = 0;
   ::glGetProgramiv(program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &bufSize);

   auto buf = std::vector<char>(bufSize + 1, '\0');

   for(auto i = 0; i < numUniforms; ++i) {
      GLint size=0;
      GLenum type;
      glGetActiveUniform(program, i, bufSize, nullptr, &size, &type, buf.data());
      PezDebugString("'%s'\n", buf.data());
   }
}