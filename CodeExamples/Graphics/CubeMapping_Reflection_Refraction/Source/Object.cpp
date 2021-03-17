/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Object.cpp, implementation for Object class (holds object information)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/

#include "Object.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

#include "Environment.h"

extern int numberOfSpheresDraw;
extern float attunation[3];
extern float emissive[3];
extern float fog[3];

extern GLuint diffuseTexture;
extern GLuint specularTexture;

extern Environment* environment;
extern float interpolationFactor;

glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 4.0f);
float near = 1.0f;
float far = 100.0f;

glm::mat4 pers = glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, near, far);
glm::mat4 view = glm::lookAt(cameraPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

//glm::mat4 pers = glm::perspective(glm::radians(90.0f), 1.0f, near, far);
//glm::vec3 cameraDirection = glm::vec3(0.0f, 0.0f, -1.0f);
//glm::mat4 view = glm::lookAt(glm::vec3(), cameraDirection, glm::vec3(0.0f, 1.0f, 0.0f));

extern int TextureSize;
extern int textureType;

extern bool CPUUV;

void Object::SetUniforms(GLuint programID)
{
  GLint vTransformLoc = glGetUniformLocation(programID, "vertexTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &transform.transformMatrix[0][0]);
  }

  vTransformLoc = glGetUniformLocation(programID, "RotTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &transform.RotationMatrix[0][0]);
  }

  vTransformLoc = glGetUniformLocation(programID, "ProjTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &pers[0][0]);
  }

  vTransformLoc = glGetUniformLocation(programID, "ViewTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &view[0][0]);
  }


  vTransformLoc = glGetUniformLocation(programID, "modelColour");

  if (vTransformLoc >= 0)
  {
    glUniform3f(vTransformLoc, colour.r, colour.g, colour.b);
  }

  vTransformLoc = glGetUniformLocation(programID, "numLights");

  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, numberOfSpheresDraw);
  }

  vTransformLoc = glGetUniformLocation(programID, "cameraPos");

  if (vTransformLoc >= 0)
  {
    glUniform3f(vTransformLoc, cameraPos.x, cameraPos.y, cameraPos.z);
  }

  vTransformLoc = glGetUniformLocation(programID, "near");

  if (vTransformLoc >= 0)
  {
    glUniform1f(vTransformLoc, near);
  }

  vTransformLoc = glGetUniformLocation(programID, "far");

  if (vTransformLoc >= 0)
  {
    glUniform1f(vTransformLoc, far);
  }

  vTransformLoc = glGetUniformLocation(programID, "attunations");

  if (vTransformLoc >= 0)
  {
    glUniform3f(vTransformLoc, attunation[0], attunation[1], attunation[2]);
  }

  vTransformLoc = glGetUniformLocation(programID, "Emissive");

  if (vTransformLoc >= 0)
  {
    glUniform3f(vTransformLoc, emissive[0], emissive[1], emissive[2]);
  }

  vTransformLoc = glGetUniformLocation(programID, "Fog");

  if (vTransformLoc >= 0)
  {
    glUniform3f(vTransformLoc, fog[0], fog[1], fog[2]);
  }

  vTransformLoc = glGetUniformLocation(programID, "texDiffuse");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, DIFFUSE);
  }

  vTransformLoc = glGetUniformLocation(programID, "texSpecular");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, SPECULAR);
  }

  vTransformLoc = glGetUniformLocation(programID, "minPoint");
  if (vTransformLoc >= 0)
  {
    glUniform3f(vTransformLoc, mesh->min.x, mesh->min.y, mesh->min.z);
  }

  vTransformLoc = glGetUniformLocation(programID, "maxPoint");
  if (vTransformLoc >= 0)
  {
    glUniform3f(vTransformLoc, mesh->max.x, mesh->max.y, mesh->max.z);
  }

  vTransformLoc = glGetUniformLocation(programID, "textured");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, texture);
  }

  vTransformLoc = glGetUniformLocation(programID, "CPUGenUV");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, CPUUV);
  }

  vTransformLoc = glGetUniformLocation(programID, "textureType");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, textureType);
  }

  vTransformLoc = glGetUniformLocation(programID, "textureEntity");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, textureEntity);
  }

  vTransformLoc = glGetUniformLocation(programID, "objectPos");
  if (vTransformLoc >= 0)
  {
    glUniform3f(vTransformLoc, transform.position.x, transform.position.y, transform.position.z);
  }

  vTransformLoc = glGetUniformLocation(programID, "environmentEntity");
  if (vTransformLoc >= 0)
  {
    glUniform1i(vTransformLoc, int(environment->mapping));
  }

  vTransformLoc = glGetUniformLocation(programID, "refractionIndex");
  if (vTransformLoc >= 0)
  {
    glUniform1f(vTransformLoc, environment->refectionIndex);
  }

  vTransformLoc = glGetUniformLocation(programID, "interpolationFactor");
  if (vTransformLoc >= 0)
  {
    glUniform1f(vTransformLoc, interpolationFactor);
  }

  vTransformLoc = glGetUniformLocation(programID, "FresnelPower");
  if (vTransformLoc >= 0)
  {
    glUniform1f(vTransformLoc, environment->fresnal);
  }

  GLint lights_index = glGetUniformBlockIndex(programID, "Lights");

  if (lights_index >= 0)
  {
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, UBO);
  }

  environment->SendToShader(programID);
}

Object::Object(const char* Name) : name(Name), mesh(nullptr), drawType(DrawType::Fill), UBO(-1), shaderToUse(Shaders::simple),
colour(glm::vec3(1.0f)), textureEntity()
{
  texture = false;
  rotateObject = false;
}

Object::~Object()
{

}

void Object::Update()
{
  if (rotateObject)
  {
    transform.rotation.y += 0.01f;
  }

  //transform.rotation.z += 0.01f;
  transform.Update();

  //glm::mat4 ortho = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
  transform.transformMatrix = view * transform.transformMatrix;
}

void Object::Draw(GLuint programID)
{
  if (!mesh)
  {
    return;
  }

  glBindTextureUnit(DIFFUSE, diffuseTexture);
  glBindTextureUnit(SPECULAR, specularTexture);

  glUseProgram(programID);

  glBindVertexArray(mesh->VAO);

  // Uniform matrix
  // Uniform transformation (vertex shader)
  SetUniforms(programID);

  switch (drawType)
  {
  case DrawType::Fill:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  case DrawType::Line:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case DrawType::Point:
    glPointSize(5);
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    break;
  default:
    break;
  }

  glDrawElements(GL_TRIANGLES, mesh->NumberOfElements, GL_UNSIGNED_INT, 0);
}

void Object::DrawFaceNormal(GLuint programid)
{
  glBindVertexArray(mesh->FaceVAO);

  GLint vTransformLoc = glGetUniformLocation(programid, "vertexTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &transform.transformMatrix[0][0]);
  }

  vTransformLoc = glGetUniformLocation(programid, "projTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &pers[0][0]);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_LINES, 0, mesh->FaceNumberOfElements);
}

void Object::DrawVertexNormal(GLuint programid)
{
  glBindVertexArray(mesh->VertexVAO);

  GLint vTransformLoc = glGetUniformLocation(programid, "vertexTransform");

  if (vTransformLoc >= 0)
  {
    glUniformMatrix4fv(vTransformLoc, 1, GL_FALSE, &transform.transformMatrix[0][0]);
  }

  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glDrawArrays(GL_LINES, 0, mesh->VertexNumberOfElements);
}