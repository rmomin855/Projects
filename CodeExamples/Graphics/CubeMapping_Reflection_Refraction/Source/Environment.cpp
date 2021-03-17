/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Envionment.cpp, implementation for FBO
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#include "Environment.h"
#include "SimpleScene_Quad.h"
#include "Skybox.h"

#include <glm/glm.hpp>

#include <iostream>

extern int windowWidth;
extern int windowHeight;

extern glm::mat4 view;
extern glm::mat4 pers;
extern Scene* scene;
extern float near;
extern float far;
extern Object* centralObject;

Environment::Environment() : mapping(environmentEntity::reflection), refectionIndex(10.0f), fresnal(1.0f)
{
  glGenFramebuffers(1, &FBO);
  glGenRenderbuffers(1, &RBO);

  glCreateTextures(GL_TEXTURE_2D, 1, &Top);
  glTextureStorage2D(Top, 1, GL_RGB8, windowWidth, windowHeight);

  glTextureParameteri(Top, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(Top, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glCreateTextures(GL_TEXTURE_2D, 1, &Bottom);
  glTextureStorage2D(Bottom, 1, GL_RGB8, windowWidth, windowHeight);

  glTextureParameteri(Bottom, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(Bottom, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glCreateTextures(GL_TEXTURE_2D, 1, &Left);
  glTextureStorage2D(Left, 1, GL_RGB8, windowWidth, windowHeight);

  glTextureParameteri(Left, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(Left, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glCreateTextures(GL_TEXTURE_2D, 1, &Right);
  glTextureStorage2D(Right, 1, GL_RGB8, windowWidth, windowHeight);

  glTextureParameteri(Right, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(Right, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glCreateTextures(GL_TEXTURE_2D, 1, &Front);
  glTextureStorage2D(Front, 1, GL_RGB8, windowWidth, windowHeight);

  glTextureParameteri(Front, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(Front, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glCreateTextures(GL_TEXTURE_2D, 1, &Back);
  glTextureStorage2D(Back, 1, GL_RGB8, windowWidth, windowHeight);

  glTextureParameteri(Back, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTextureParameteri(Back, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Environment::Update()
{
  glm::mat4 oldView = view;
  glm::mat4 oldPers = pers;
  SimpleScene_Quad* curr = reinterpret_cast<SimpleScene_Quad*>(scene);
  curr->withEditor = false;

  pers = glm::perspective(glm::radians(90.0f), 1.0f, near, far);
  glm::vec3 cameraPos = centralObject->transform.position;

  glBindFramebuffer(GL_FRAMEBUFFER, FBO);
  glBindRenderbuffer(GL_RENDERBUFFER, RBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, windowWidth, windowHeight);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);

  //front
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Front, 0);
  glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
  view = glm::lookAt(cameraPos, cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
  curr->Render();

  //back
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Back, 0);
  cameraDirection = glm::vec3(0.0f, 0.0f, 1.0f);
  view = glm::lookAt(cameraPos, cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
  curr->Render();

  //Right
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Right, 0);
  cameraDirection = glm::vec3(1.0f, 0.0f, 0.0f);
  view = glm::lookAt(cameraPos, cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
  curr->Render();

  //left
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Left, 0);
  cameraDirection = glm::vec3(-1.0f, 0.0f, 0.0f);
  view = glm::lookAt(cameraPos, cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));
  curr->Render();

  //top
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Top, 0);
  cameraDirection = glm::vec3(0.0f, 1.0f, 0.0f);
  view = glm::lookAt(cameraPos, cameraDirection, glm::vec3(0.0f, 0.0f, -1.0f));
  curr->Render();

  //down
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, Bottom, 0);
  cameraDirection = glm::vec3(0.0f, -1.0f, 0.0f);
  view = glm::lookAt(cameraPos, cameraDirection, glm::vec3(0.0f, 0.0f, 1.0f));
  curr->Render();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  curr->withEditor = true;
  view = oldView;
  pers = oldPers;
}

void Environment::SendToShader(GLuint programID)
{
  glBindTextureUnit(TOPTEX, Top);
  glBindTextureUnit(DOWNTEX, Bottom);
  glBindTextureUnit(RIGHTTEX, Right);
  glBindTextureUnit(LEFTTEX, Left);
  glBindTextureUnit(FRONTTEX, Front);
  glBindTextureUnit(BACKTEX, Back);

  GLint vTransformLoc = glGetUniformLocation(programID, "texTop");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, TOPTEX);
  }

  vTransformLoc = glGetUniformLocation(programID, "texDown");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, DOWNTEX);
  }

  vTransformLoc = glGetUniformLocation(programID, "texRight");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, RIGHTTEX);
  }

  vTransformLoc = glGetUniformLocation(programID, "texLeft");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, LEFTTEX);
  }

  vTransformLoc = glGetUniformLocation(programID, "texFront");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, FRONTTEX);
  }

  vTransformLoc = glGetUniformLocation(programID, "texBack");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, BACKTEX);
  }
}