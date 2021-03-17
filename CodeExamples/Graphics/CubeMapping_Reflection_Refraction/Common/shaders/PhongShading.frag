/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: PhongShading.frag, implementation of phongShading, calculates
light and texture
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment2_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 15-11-19
*/

#version 460 core

uniform sampler2D texTop;
uniform sampler2D texDown;
uniform sampler2D texRight;
uniform sampler2D texLeft;
uniform sampler2D texFront;
uniform sampler2D texBack;

#define TOP 5
#define DOWN 6
#define RIGHT 7
#define LEFT 8
#define FRONT 9
#define BACK 10

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

uniform mat4 ViewTransform;

in vec3 fragNormal;
in vec3 position;
in vec2 uv;
in vec3 posUV;

uniform int environmentEntity;
uniform float refractionIndex;
uniform float interpolationFactor;
uniform float FresnelPower;

uniform vec3 modelColour;
uniform int numLights;
uniform vec3 attunations;
uniform vec3 Emissive;
uniform vec3 Fog;

uniform int textured;
uniform int textureType;
uniform int CPUGenUV;

uniform mat4 vertexTransform;
uniform vec3 objectPos;

uniform sampler2D texDiffuse;
uniform sampler2D texSpecular;
uniform vec3 maxPoint;
uniform vec3 minPoint;
uniform vec3 cameraPos;
uniform float near;
uniform float far;

out vec3 fragColor;

vec3 GetPointLight(Light light, vec3 lightVec, vec3 fragNormal);
vec3 GetDirectionalLight(Light light, vec3 lightVec, vec3 fragNormal);
vec3 GetSpotLight(Light light, vec3 lightVec, vec3 fragNormal);
vec3 GetEnvironment(vec3 viewVec, vec3 fragNormal);

vec2 GetUV(vec3 entity);

vec3 GetColourFromLight(Light light, vec3 viewVec)
{
  vec3 lightVec = (inverse(ViewTransform) * vec4(light.position,1.0f)).xyz - position;
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
    specularColour = light.specularColour * texture(texSpecular, GetUV(posUV)).xyz * specularShine;
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
  fragColor = GetEnvironment(viewVec, fragNormal);
  fragColor = mix(fragColor, colour, interpolationFactor);
}

vec3 GetPointLight(Light light, vec3 lightVec, vec3 fragNormal)
{
  float diffuse = max(dot(lightVec, fragNormal), 0.0f);
  vec3 diffuseColour = light.diffuseColour * diffuse * light.diffuseStrength;

  if(textured == 1)
  {
    diffuseColour = light.diffuseColour * texture(texDiffuse, GetUV(posUV)).xyz * diffuse;
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
    diffuseColour = light.diffuseColour * texture(texDiffuse, GetUV(posUV)).xyz * diffuse * direction;
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
      diffuseColour = light.diffuseColour * texture(texDiffuse, GetUV(posUV)).xyz * pow(smoothstep(light.halfFieldOfView, light.innerAngle, angleBetween), light.fallOff) * direction;
    }
  }

  return diffuseColour;
}

int GetPlane(vec3 entity)
{
  vec3 abs = abs(entity);
  int temp = 0;

  if (abs.x >= abs.y && abs.x >= abs.z)
  {
    if(entity.x > 0)
    {
      temp = LEFT;
    }
    else
    {
      temp = RIGHT;
    }
  }
  else if (abs.y >= abs.x && abs.y >= abs.z)
  {
    if(entity.y > 0)
    {
      temp = TOP;
    }
    else
    {
      temp = DOWN;
    }
  }
  else if (abs.z >= abs.x && abs.z >= abs.y)
  {
    if(entity.z < 0)
    {
      temp = FRONT;
    }
    else
    {
      temp = BACK;
    }
  }

  return temp;
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

vec3 GetColourFromTexture(vec2 uv, int plane)
{
  if(plane == TOP)
  {
    //return vec3(1.0f,0.0f,0.0f);
    return texture(texTop, uv).xyz;
  }
  if(plane == DOWN)
  {
    return texture(texDown, uv).xyz;
  }
  if(plane == RIGHT)
  {
    return texture(texRight, uv).xyz;
  }
  if(plane == LEFT)
  {
    return texture(texLeft, uv).xyz;
  }
  if(plane == FRONT)
  {
    return texture(texFront, uv).xyz;
  }
  if(plane == BACK)
  {
    return texture(texBack, uv).xyz;
  }
}

vec2 GetUV(vec3 entity)
{
  if(CPUGenUV == 1)
  {  
    return uv;
  }

  vec3 temp = entity;
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

vec3 GetEnvironment(vec3 viewVec, vec3 fragNormal)
{
  vec3 entity = vec3(0.0f);

  if(environmentEntity == 0)
  {
    entity = (2.0f * dot(viewVec, fragNormal) * fragNormal) - viewVec;
    entity.x = -entity.x;
  }
  else if(environmentEntity == 1)
  {
    float k = 1.000293f / refractionIndex;
    float vDotN = dot(viewVec, fragNormal);

    float temp = k * vDotN;
    temp -= sqrt(1.0f - (k * k)*(1.0f - (vDotN * vDotN)));
    entity = (temp * fragNormal) - (k * viewVec);

    float angC = asin(1.0f / k);
    float angle = acos(vDotN);

    if(angle >= angC)
    {
      entity = (2.0f * dot(viewVec, fragNormal) * fragNormal) - viewVec;
      entity.x = -entity.x;
    }
  }
  else if(environmentEntity == 2)
  {
    entity = (2.0f * dot(viewVec, fragNormal) * fragNormal) - viewVec;
    entity.x = -entity.x;
    int plane = GetPlane(entity);
    vec2 UV = CalculateCubeUV(entity);
    vec3 reflect = GetColourFromTexture(UV, plane);

    float k = 1.000293f / refractionIndex;
    float vDotN = dot(viewVec, fragNormal);
    float temp = k * vDotN;
    temp -= sqrt(1.0f - (k * k)*(1.0f - (vDotN * vDotN)));
    entity = (temp * fragNormal) - (k * viewVec);
    entity = normalize(entity);
    float angC = asin(1.0f / k);
    float angle = acos(vDotN);
    if(angle >= angC)
    {
      entity = (2.0f * dot(viewVec, fragNormal) * fragNormal) - viewVec;
      entity.x = -entity.x;
    }
    plane = GetPlane(entity);
    UV = CalculateCubeUV(entity);
    vec3 refract = GetColourFromTexture(UV, plane);

    const float F = ((1.0-k) * (1.0-k)) / ((1.0+k) * (1.0+k));
    float Ratio = F + (1.0 -F) * pow((1.0 -dot(-viewVec, fragNormal)), FresnelPower);

    return mix(refract, reflect, Ratio);
  }

  int plane = GetPlane(entity);
  vec2 UV = CalculateCubeUV(entity);
  return GetColourFromTexture(UV, plane);
}