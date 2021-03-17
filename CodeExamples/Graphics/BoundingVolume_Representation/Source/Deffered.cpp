/*
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Object.cpp, implementation for Object class (holds object information)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 31-1-20
*/

#include "Deffered.h"
#include <shader.hpp>
#include "Camera.h"

enum class renderTarget
{
  Position = 0,
  Normal,
  UV,
};

extern glm::mat4 pers;
extern Camera camera;

Deferred::Deferred(size_t width, size_t height) : FBO(), depth(), renderTargets(), attachments(),
width(width), height(height), FSQTarget(0), copyDepth(true)
{
  //create and bind FBO to pipeline
  glCreateFramebuffers(1, &FBO);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, FBO);

  //for each render target we want
  for (size_t i = 0; i < NUMTARGETS; i++)
  {
    //create a texture
    glCreateTextures(GL_TEXTURE_2D, 1, &renderTargets[i]);

    //set up size of texture buffer
    glTextureStorage2D(renderTargets[i], 1, GL_RGB16F, width, height);

    //set up properties
    glTextureParameteri(renderTargets[i], GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTextureParameteri(renderTargets[i], GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    //determine attachment location and attach it
    attachments[i] = GL_COLOR_ATTACHMENT0 + i;
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i], GL_TEXTURE_2D, renderTargets[i], 0);

    //activate the textures according to unit
    glActiveTexture(GL_TEXTURE0 + i);
    //bind current texture buffer to unit
    glBindTextureUnit(i, renderTargets[i]);
  }

  //indicate the 3 write buffers to use
  glDrawBuffers(NUMTARGETS, attachments);

  //create render buffer for depth buffer
  glCreateRenderbuffers(1, &depth);
  //bind depth buffer
  glBindRenderbuffer(GL_RENDERBUFFER, depth);
  //set up storage for depth buffer
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
  //attach buffer to frame buffer as depth buffer
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth);

  //check if fbo is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
  {
    printf("gbufferprogram failed to compile");
  }

  //set to default fbo
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  //compile shader for g-buffer
  deferedShader = LoadShaders("../Common/shaders/Deferred.vert",
    "../Common/shaders/Deferred.frag");
}

void Deferred::Render(const std::vector<Object*>& objects)
{
  //bind created fbo to pipeline
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  //clear everything
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //g-buffer shader
  glUseProgram(deferedShader);

  size_t size = objects.size();

  //draw all deferred objects
  for (size_t i = 0; i < size; i++)
  {
    objects[i]->Update();
    objects[i]->Draw(deferedShader);
  }

  //bind default fbo
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Deferred::Render(std::vector<Mesh*>* meshes)
{
  //bind created fbo to pipeline
  glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  //clear everything
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  //g-buffer shader
  glUseProgram(deferedShader);

  size_t size = meshes->size();

  GLint vTransformLoc = glGetUniformLocation(deferedShader, "vertexTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &mat4(1.0f)[0][0]);
  }

  vTransformLoc = glGetUniformLocation(deferedShader, "RotTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &mat4(1.0f)[0][0]);
  }

  vTransformLoc = glGetUniformLocation(deferedShader, "ViewTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &camera.cam_viewMatrix[0][0]);
  }

  vTransformLoc = glGetUniformLocation(deferedShader, "ProjTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &pers[0][0]);
  }

  //draw all deferred objects
  for (size_t i = 0; i < size; i++)
  {
    glBindVertexArray((*meshes)[i]->VAO);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, (*meshes)[i]->NumberOfElements, GL_UNSIGNED_INT, 0);
  }

  //bind default fbo
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Deferred::CopyDepthBuffer()
{
  //write from our fbo to default fbo
  //bind read target to created fbo to pipeline
  glBindFramebuffer(GL_READ_FRAMEBUFFER, FBO);
  //bind write target to default fbo
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  //copy buffer
  glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
  //bind default fbo
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
