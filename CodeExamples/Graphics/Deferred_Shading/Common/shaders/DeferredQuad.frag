/*
Copyright (C) 2020 DigiPen Institute of Technology.
take values from g-buffer and us it for phong model
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS350
Author: Rahil Momin, r.momin, 180001717
Creation date: 31-1-20
*/

#version 460 core

out vec3 fragColor;

uniform sampler2D texDiffuse;
uniform sampler2D texSpecular;
uniform sampler2D posTex;
uniform sampler2D normTex;
uniform sampler2D uvTex;

uniform int target;

uniform vec3 cameraPos;
uniform float near;
uniform float far;
uniform int numLights;
uniform vec3 attunations;
uniform vec3 Emissive;
uniform vec3 Fog;
uniform int textured;

in vec2 quadCoord;

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

vec3 GetPointLight(Light light, vec3 lightVec, vec3 fragNormal, vec2 uv);
vec3 GetDirectionalLight(Light light, vec3 lightVec, vec3 fragNormal, vec2 uv);
vec3 GetSpotLight(Light light, vec3 lightVec, vec3 fragNormal, vec2 uv);
vec3 GetColourFromLight(Light light, vec3 viewVec, vec3 fragNormal, vec2 uv, vec3 position);

void main()
{
  vec3 position = texture(posTex, quadCoord).xyz;
  vec3 normal = texture(normTex, quadCoord).xyz;
  vec3 uv = texture(uvTex, quadCoord).xyz;

  if(dot(normal, normal) == 0.0f)
  {
    fragColor = normal;
    return;
  }

  //FSQ target position
  if(target == 1)
  {
    fragColor = position;
    return;
  }

  //FSQ target normal
  if(target == 2)
  {
    fragColor = normal;
    return;
  }

  //FSQ target uv
  if(target == 3)
  {
    fragColor = uv;
    return;
  }

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
    colour += GetColourFromLight(lights[i], viewVec, normal, uv.xy, position);
  }

  //local
  colour += emissive;

  //final
  float s = (zfar - viewLen) / (zfar - znear);
  colour = (s * colour) + ((1.0f - s) * fog);

  fragColor = colour;
}

vec3 GetColourFromLight(Light light, vec3 viewVec, vec3 fragNormal, vec2 uv, vec3 position)
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
    diffuseColour = GetPointLight(light, lightVec, fragNormal, uv);
  }
  //directional
  else if(light.LightType == 1)
  {
    diffuseColour = GetDirectionalLight(light, lightVec, fragNormal, uv);
  }
  //spot
  else if(light.LightType == 2)
  {
    diffuseColour = GetSpotLight(light, lightVec, fragNormal, uv);
  }

  //specular
  float specularShine = pow(max(dot(reflecVec, viewVec), 0.0f), light.shininess);
  vec3 specularColour = light.specularColour * light.specularStrength * specularShine;

  if(textured == 1)
  {
    specularColour = light.specularColour * texture(texSpecular, uv).xyz * specularShine;
  }

  //local
  float c0 = attunations.x;
  float c1 = attunations.y;
  float c2 = attunations.z;
  float attunation = min(1.0f / ((c0 + (c1 * lenLight) + 
                                (c2 * lenLight * lenLight))), 1.0f);

  return (ambientColour + diffuseColour + specularColour) * attunation;
}

vec3 GetPointLight(Light light, vec3 lightVec, vec3 fragNormal, vec2 uv)
{
  float diffuse = max(dot(lightVec, fragNormal), 0.0f);
  vec3 diffuseColour = light.diffuseColour * diffuse * light.diffuseStrength;

  if(textured == 1)
  {
    diffuseColour = light.diffuseColour * texture(texDiffuse, uv).xyz * diffuse;
  }

  return diffuseColour;
}

vec3 GetDirectionalLight(Light light, vec3 lightVec, vec3 fragNormal, vec2 uv)
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
    diffuseColour = light.diffuseColour * texture(texDiffuse, uv).xyz * diffuse * direction;
  }

  return diffuseColour;
}

vec3 GetSpotLight(Light light, vec3 lightVec, vec3 fragNormal, vec2 uv)
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
      diffuseColour = light.diffuseColour * texture(texDiffuse, uv).xyz * pow(smoothstep(light.halfFieldOfView, light.innerAngle, angleBetween), light.fallOff) * direction;
    }
  }

  return diffuseColour;
}
