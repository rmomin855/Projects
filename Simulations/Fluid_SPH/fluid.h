#pragma once
#include <vector>
#include <glm/gtx/norm.hpp>
#include <glm/glm.hpp>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace glm;

class FluidParticle
{
public:
  FluidParticle() :fp_density(), fp_pressure(), fp_velocity(), fp_Force() {}
  float fp_density;
  float fp_pressure;
  vec2 fp_velocity;
  vec2 fp_Force;
};

class Fluid
{
public:
  Fluid(const std::vector<vec2>& particlePositions, float visconsity, float mass = 1.0f, float size = 1.0f, float restDensity = 1000.0f);

  void Update(float dt);

  void CalculateDensityAndPressure();
  void CalculateForces();
  void Integrate(float dt);

  friend class VapPhysicsEngine;

  std::vector<FluidParticle> f_particles;
  std::vector<vec2> f_positions;

  vec2 f_spawnPosition;
  size_t f_numberOfParticles;
  float f_visconsity;
  float f_size;
  float f_mass;
  float f_restDensity;

  float f_sizeSQ;

  float mullerPoly;
  float spikyGrad;
  float vicsGrad;
};

Fluid::Fluid(const std::vector<vec2>& particlePositions, float visconsity, float mass, float size, float restDensity) :
  f_size(size), f_visconsity(visconsity), f_mass(mass), f_restDensity(restDensity)
{
  f_positions = particlePositions;
  f_particles.resize(particlePositions.size(), FluidParticle());

  float PI = float(M_PI);
  mullerPoly = 315.f / (65.f * PI * pow(f_size, 9.f));
  spikyGrad = -45.f / (PI * pow(f_size, 6.f));
  vicsGrad = 45.f / (PI * pow(f_size, 6.f));

  f_sizeSQ = f_size * f_size;
  f_numberOfParticles = particlePositions.size();
}

void Fluid::Update(float dt)
{
  CalculateDensityAndPressure();
  CalculateForces();
  Integrate(dt);
}

void Fluid::CalculateDensityAndPressure()
{
  for (size_t i = 0; i < f_numberOfParticles; i++)
  {
    f_particles[i].fp_density = 0.0f;

    for (size_t j = 0; j < f_numberOfParticles; j++)
    {
      vec2 distVec = f_positions[j] - f_positions[i];
      float lenSQ = length2(distVec);

      if (lenSQ < f_sizeSQ)
      {
        f_particles[i].fp_density += f_mass * mullerPoly * pow(f_sizeSQ - lenSQ, 3.0f);
      }
    }

    f_particles[i].fp_pressure = 8.31446f * (f_particles[i].fp_density - f_restDensity);
    f_particles[i].fp_Force = vec2(0.0f);
  }
}

void Fluid::CalculateForces()
{
  for (size_t i = 0; i < f_numberOfParticles; i++)
  {
    vec2 pressureForce = vec2();
    vec2 visconcityForce = vec2();

    for (size_t j = 0; j < f_numberOfParticles; j++)
    {
      if (i == j)
      {
        continue;
      }

      vec2 distVec = f_positions[j] - f_positions[i];
      float lenSQ = length2(distVec);

      if (lenSQ && lenSQ < f_sizeSQ)
      {
        float len = length(distVec);
        distVec /= len;

        float pressure = f_particles[i].fp_pressure + f_particles[j].fp_pressure;

        float force = f_mass * pressure / (2.0f * f_particles[j].fp_density) * spikyGrad * pow(f_size - len, 2.0f);
        pressureForce += -distVec * force;

        float visconcity = f_visconsity * f_mass / f_particles[j].fp_density * vicsGrad * (f_size - len);
        visconcityForce += (f_particles[j].fp_velocity - f_particles[i].fp_velocity) * visconcity;
      }
    }

    f_particles[i].fp_Force = pressureForce + visconcityForce;
  }
}

void Fluid::Integrate(float dt)
{
  for (size_t i = 0; i < f_numberOfParticles; i++)
  {
    f_particles[i].fp_Force += vec2(0.0f, -150.05) * f_particles[i].fp_density;

    f_particles[i].fp_velocity += dt * f_particles[i].fp_Force / f_particles[i].fp_density;
    f_particles[i].fp_velocity += dt * -0.1f * f_particles[i].fp_velocity;

    f_positions[i] += dt * f_particles[i].fp_velocity;
  }
}