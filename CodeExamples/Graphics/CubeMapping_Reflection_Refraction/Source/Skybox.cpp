/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Skybox.cpp, implementation for skybox
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#include "Skybox.h"
#include "SimpleScene_Quad.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <SOIL2.h>

extern glm::mat4 pers;
extern glm::mat4 view;
extern Scene* scene;

Skybox::Skybox(std::vector<std::string>& textures) : textureId(6, 0), Object("Skybox")
{
  for (size_t i = 0; i < 6; i++)
  {
    textureId[i] = CreateTexture(textures[i]);
  }

  mesh = MeshManager::Instance()->meshes["cube"];
  transform.position = glm::vec3();
  transform.scale = glm::vec3(10.0f, 10.0f, 10.0f);
  drawType = DrawType::Fill;
  colour = glm::vec3(1.0f, 1.0f, 1.0f);

  glActiveTexture(GL_TEXTURE5);
  glActiveTexture(GL_TEXTURE6);
  glActiveTexture(GL_TEXTURE7);
  glActiveTexture(GL_TEXTURE8);
  glActiveTexture(GL_TEXTURE9);
  glActiveTexture(GL_TEXTURE10);
}

GLuint Skybox::CreateTexture(std::string& texture)
{
  std::string directory = std::string("../Common/textures/");
  directory += texture;

  int width = 0, height = 0, channels = 0;
  unsigned char* texturebuf = SOIL_load_image(directory.data(), &width, &height, NULL, SOIL_LOAD_RGB);

  GLuint texBuffer = 0;
  glCreateTextures(GL_TEXTURE_2D, 1, &texBuffer);

  if (texBuffer)
  {
    glTextureStorage2D(texBuffer, 1, GL_RGB8, width, height);
    glTextureSubImage2D(texBuffer, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, texturebuf);

    glTextureParameteri(texBuffer, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(texBuffer, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTextureParameteri(texBuffer, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(texBuffer, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }

  return texBuffer;
}

void Skybox::Draw()
{
  glBindTextureUnit(TOPTEX, textureId[TOP]);
  glBindTextureUnit(DOWNTEX, textureId[DOWN]);
  glBindTextureUnit(RIGHTTEX, textureId[RIGHT]);
  glBindTextureUnit(LEFTTEX, textureId[LEFT]);
  glBindTextureUnit(FRONTTEX, textureId[FRONT]);
  glBindTextureUnit(BACKTEX, textureId[BACK]);

  transform.Update();
  transform.transformMatrix = view * transform.transformMatrix;

  SimpleScene_Quad* curr = reinterpret_cast<SimpleScene_Quad*>(scene);
  GLuint program = curr->skyBox;

  glUseProgram(program);

  glBindVertexArray(mesh->VAO);

  uniforms(program);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  glDrawElements(GL_TRIANGLES, mesh->NumberOfElements, GL_UNSIGNED_INT, 0);
}

void Skybox::uniforms(GLuint programID)
{
  GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &transform.transformMatrix[0][0]);
  }

  vTransformLoc = glGetUniformLocation(programID, "ProjTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &pers[0][0]);
  }

  vTransformLoc = glGetUniformLocation(programID, "texTop");
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