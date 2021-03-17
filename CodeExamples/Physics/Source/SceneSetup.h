#pragma once
#include "SceneManager.h"

class SceneSetup : public Scene
{
public:
  void Load();
  void Init();
  void Update(float dt);
  void DeInit();
  void UnLoad();
private:
};