﻿#pragma once

#include <glad/gl.h>
#include "gl_helper.h"

class Context;

class Program {
 public:
  const char *vertProgramFile;
  const char *fragProgramFIle;

 public:
  Program(Context *ctx) : ctx(ctx) {
    vertProgramFile = "../assets/shaders/example.vert";
    fragProgramFIle = "../assets/shaders/example.frag";
  }

  virtual bool load() = 0;
  virtual void doMainLoop() = 0;
  GLuint programId = -1;
  GLuint *VAO = 0;

 protected:
  const Context *ctx;
};

class ExampleProgram : public Program {
 public:
  ExampleProgram(Context *ctx) : Program(ctx) {
    vertProgramFile = "../assets/shaders/example.vert";
    fragProgramFIle = "../assets/shaders/example.frag";
  }
  bool load() override;
  void doMainLoop() override;
};

class LightProgram : public Program {
 public:
  LightProgram(Context *ctx) : Program(ctx) {
    vertProgramFile = "../assets/shaders/light.vert";
    fragProgramFIle = "../assets/shaders/light.frag";
  }
  bool load() override;
  void doMainLoop() override;
};

class SkyboxProgram : public Program {
 public:
  SkyboxProgram(Context *ctx) : Program(ctx) {
    vertProgramFile = "../assets/shaders/skybox.vert";
    fragProgramFIle = "../assets/shaders/skybox.frag";
  }
  bool load() override;
  void doMainLoop() override;
};