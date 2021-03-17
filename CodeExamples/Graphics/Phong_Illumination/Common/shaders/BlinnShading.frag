/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: BlinnShading.frag, implementation of phongShading, calculates
light and texture
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#version 460 core

struct Light
{
  vec3 position;
  int LightType;

  vec3 direction;
  float halfFieldOfView;

  vec3 ambientColour;
  float ambientStrength;

  vec3 diffuseColour;
  float diffuseStrength;

  vec3 specularColour;
  float specularStrength;

  float padding;
  float shininess;
  float innerAngle;
  float fallOff;
};

layout(std140, binding = 3) uniform Lights
{
  Light lights[16];
};

in vec3 fragNormal;
in vec3 position;
in vec2 uv;
in vec3 posUV;

uniform mat4 vertexTransform;
uniform vec3 modelColour;
uniform int numLights;
uniform vec3 attunations;
uniform vec3 Emissive;
uniform vec3 Fog;

uniform int textured;
uniform int textureType;
uniform int textureEntity;
uniform int CPUGenUV;

uniform sampler2D texDiffuse;
uniform sampler2D texSpecular;
uniform vec3 maxPoint;
uniform vec3 minPoint;
uniform vec3 cameraPos;
uniform float near;
uniform float far;
uniform vec3 objectPos;

out vec3 fragColor;

vec3 GetPointLight(Light light, vec3 lightVec, vec3 fragNormal);
vec3 GetDirectionalLight(Light light, vec3 lightVec, vec3 fragNormal);
vec3 GetSpotLight(Light light, vec3 lightVec, vec3 fragNormal);
vec2 GetUV();

vec3 GetColourFromLight(Light light, vec3 viewVec)
{
  vec3 lightVec = light.position - position;
  float lenLight = length(lightVec);
  lightVec /= lenLight;

  vec3 halfVec = lightVec + viewVec;
  halfVec = normalize(halfVec);

  //ambient
  vec3 ambientColour = light.ambientColour * light.ambientStrength;

  //diffuse
  vec3 diffuseColour = vec3(0.0f);
  //point
  if(light.LightType == 0)
  {
    diffuseColour = GetPointLight(light, lightVec, fragNormal);
  }
  //directional
  else if(light.LightType == 1)
  {
    diffuseColour = GetDirectionalLight(light, lightVec, fragNormal);
  }
  //spot
  else if(light.LightType == 2)
  {
    diffuseColour = GetSpotLight(light, lightVec, fragNormal);
  }

  //specular
  float specularShine = pow(max(dot(halfVec, fragNormal), 0.0f), light.shininess);
  vec3 specularColour = light.specularColour * light.specularStrength * specularShine;

  if(textured == 1)
  {
    specularColour = light.specularColour * texture(texSpecular, GetUV()).xyz * light.specularStrength * specularShine;
  }

  //local
  float c0 = attunations.x;
  float c1 = attunations.y;
  float c2 = attunations.z;
  float attunation = min(1.0f / ((c0 + (c1 * lenLight) + 
                                (c2 * lenLight * lenLight))), 1.0f);

  return (ambientColour + diffuseColour + specularColour) * attunation;
}

void main()
{
  vec3 colour = vec3(0.0f, 0.0f, 0.0f);
  vec3 emissive = Emissive / 255.0f;
  vec3 fog = Fog / 255.0f;

  float zfar = far;
  float znear = near;

  vec3 viewVec = cameraPos - position;
  float viewLen = length(viewVec);
  viewVec /= viewLen;

  for(int i = 0; i < numLights; i++)
  {
    colour += GetColourFromLight(lights[i], viewVec);
  }

  //local
  colour += emissive;

  //final
  float s = (zfar - viewLen) / (zfar - znear);
  colour = (s * colour) + ((1.0f - s) * fog);

  fragColor = modelColour * colour;
}

vec3 GetPointLight(Light light, vec3 lightVec, vec3 fragNormal)
{
  float diffuse = max(dot(lightVec, fragNormal), 0.0f);
  vec3 diffuseColour = light.diffuseColour * diffuse * light.diffuseStrength;

  if(textured == 1)
  {
    diffuseColour = light.diffuseColour * texture(texDiffuse, GetUV()).xyz * diffuse;
  }

  return diffuseColour;
}

vec3 GetDirectionalLight(Light light, vec3 lightVec, vec3 fragNormal)
{
  float direction = dot(fragNormal, lightVec);

  if(direction < 0.0f)
  {
    return vec3(0.0f);
  }

  vec3 dir = normalize(light.direction);
  float diffuse = max(dot(dir, -lightVec), 0.0f);
  vec3 diffuseColour = light.diffuseColour * diffuse * light.diffuseStrength * direction;

  if(textured == 1)
  {
    diffuseColour = light.diffuseColour * texture(texDiffuse, GetUV()).xyz * diffuse * direction;
  }

  return diffuseColour;
}

vec3 GetSpotLight(Light light, vec3 lightVec, vec3 fragNormal)
{
  float direction = dot(fragNormal, lightVec);

  if(direction < 0.0f)
  {
    return vec3(0.0f);
  }

  vec3 dir = normalize(light.direction);
  float angleBetween = float(acos(dot(dir, -lightVec)));
  angleBetween = degrees(angleBetween);
  vec3 diffuseColour = vec3(0.0f);

  if(angleBetween > 0.0f && angleBetween < light.halfFieldOfView)
  {
    diffuseColour = light.diffuseColour * light.diffuseStrength * pow(smoothstep(light.halfFieldOfView, light.innerAngle, angleBetween), light.fallOff) * direction;

    if(textured == 1)
    {
      diffuseColour = light.diffuseColour * texture(texDiffuse, GetUV()).xyz * pow(smoothstep(light.halfFieldOfView, light.innerAngle, angleBetween), light.fallOff) * direction;
    }
  }

  return diffuseColour;
}

vec2 CalculateCubeUV(vec3 entity)
{
  vec2 temp = vec2(0.0f);
  vec3 abs = abs(entity);


  if (abs.x >= abs.y && abs.x >= abs.z)
  {
    temp.x = entity.y / abs.x;
    temp.y = entity.z / abs.x;
  }
  else if (abs.y >= abs.x && abs.y >= abs.z)
  {
    temp.x = entity.x / abs.y;
    temp.y = entity.z / abs.y;
  }
  else if (abs.z >= abs.x && abs.z >= abs.y)
  {
    temp.x = entity.x / abs.z;
    temp.y = entity.y / abs.z;
  }

  temp += 1.0f;
  temp *= 0.5f;

  return temp;
}

vec2 GetUV()
{
  if(CPUGenUV == 1)
  {  
    return uv;
  }

  vec3 temp = posUV;
  vec3 max = maxPoint;
  vec3 min = minPoint;

  float angle = atan(temp.y, temp.x);
  angle *= 2.0f;
  angle = angle + radians(180.0f);

  float z = (temp.z - min.z) / (max.z - min.z);

  float u = angle / radians(360.0f);
  float v = 0;

  //cylindrical
  if(textureType == 0)
  {
   v = z;
  }

  //spherical
  else if(textureType == 1)
  {
    float phi = acos(z / length(temp));
    phi /= radians(180.0f);

    v = 1.0f - phi;
  }

  //planar
  else if(textureType == 2)
  {
    vec2 uv = CalculateCubeUV(temp);
    u = uv.x;
    v = uv.y;
  }

  return vec2(u, v);
}