/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: PhongLighting.vert, implementation of phonglighting, calculates
normals, approprite positions, lights and textures
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#version 460 core

uniform mat4 vertexTransform;
uniform mat4 RotTransform;
uniform mat4 ProjTransform;
uniform vec3 modelColour;

in layout(location = 0) vec3 modelPosition;
in layout(location = 1) vec3 vertexNormal;
in layout(location = 2) vec2 UV;

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

vec3 GetPointLight(Light light, vec3 lightVec, vec3 fragNormal);
vec3 GetDirectionalLight(Light light, vec3 lightVec, vec3 fragNormal);
vec3 GetSpotLight(Light light, vec3 lightVec, vec3 fragNormal);

uniform int numLights;
out vec3 colour;
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

vec3 GetColourFromLight(Light light, vec3 viewVec);
vec2 GetUV();

vec3 position;
vec3 fragNormal;
vec2 uv;
vec3 posUV;

void main()
{
  vec4 tempNorm = RotTransform * vec4(vertexNormal, 0.0f);
  fragNormal = tempNorm.xyz;
  gl_Position = (ProjTransform * vertexTransform) * vec4( modelPosition, 1.0f );
  position = (vertexTransform * vec4( modelPosition, 1.0f )).xyz;

  uv = UV;
  posUV = modelPosition;

  //entity = normal
  if(textureEntity == 1)
  {
    posUV = normalize(vertexNormal);
  }

  vec3 colourVert = vec3(0.0f, 0.0f, 0.0f);
  vec3 emissive = Emissive / 255.0f;
  vec3 fog = Fog / 255.0f;

  float zfar = far;
  float znear = near;

  vec3 viewVec = cameraPos - position;
  float viewLen = length(viewVec);
  viewVec /= viewLen;

  for(int i = 0; i < numLights; i++)
  {
    colourVert += GetColourFromLight(lights[i], viewVec);
  }

  //local
  colourVert += emissive;

  //final
  float s = (zfar - viewLen) / (zfar - znear);
  colourVert = (s * colourVert) + ((1.0f - s) * fog);

  colour = modelColour * colourVert;
}

vec3 GetColourFromLight(Light light, vec3 viewVec)
{
  vec3 lightVec = light.position - position;
  float lenLight = length(lightVec);
  lightVec /= lenLight;

  vec3 reflecVec = (2.0f * dot(lightVec, fragNormal) * fragNormal) - lightVec;
  reflecVec = normalize(reflecVec);

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
  float specularShine = pow(max(dot(reflecVec, viewVec), 0.0f), light.shininess);
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
    (entity.x < 0.0 ) ? ( temp.x = entity.z) : ( temp.x = -entity.z);
    temp.y = entity.y;
    temp = temp / abs.x;
  }
  else if (abs.y >= abs.x && abs.y >= abs.z)
  {
    (entity.y < 0.0 ) ? ( temp.y = entity.z) : ( temp.y = -entity.z);
    temp.x = entity.x;
    temp = temp / abs.y;
  }
  else if (abs.z >= abs.x && abs.z >= abs.y)
  {
    (entity.z < 0.0 ) ? ( temp.x = -entity.x) : ( temp.x = entity.x);
    temp.y = entity.y;
    temp = temp / abs.z;
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