#include "SceneManager.h"
#include "SceneSetup.h"

SceneManager* SceneManager::sm_singleton = nullptr;

SceneManager::SceneManager() : sm_currentScene(Scenes::Invalid), sm_nextScene(Scenes::FirstScene), sm_prevScene(Scenes::Invalid),
sm_isChanging(true), sm_isRestarting(false)
{
  sm_scenes[Scenes::SceneSetup] = new SceneSetup();
}

SceneManager::~SceneManager()
{
  if (sm_scenes[sm_currentScene])
  {
    sm_scenes[sm_currentScene]->DeInit();
    sm_scenes[sm_currentScene]->UnLoad();
  }

  auto current = sm_scenes.begin();
  auto end = sm_scenes.end();

  while (current != end)
  {
    if (current->second)
    {
      delete current->second;
    }
    current++;
  }
}

void SceneManager::SetNextScene(Scenes scene)
{
  sm_nextScene = scene;

  if (sm_currentScene == sm_nextScene)
  {
    sm_isRestarting = true;
  }
  else
  {
    sm_isChanging = true;
  }
  isDirty = true;
}

void SceneManager::ChangeStates()
{
  if (sm_scenes[sm_currentScene])
  {
    sm_scenes[sm_currentScene]->DeInit();
    sm_scenes[sm_currentScene]->UnLoad();
  }

  sm_currentScene = sm_nextScene;

  if (sm_scenes[sm_currentScene])
  {
    sm_scenes[sm_currentScene]->Load();
    sm_scenes[sm_currentScene]->Init();
  }

  sm_isChanging = false;
}

void SceneManager::ResetState()
{
  if (sm_scenes[sm_currentScene])
  {
    sm_scenes[sm_currentScene]->DeInit();
    sm_scenes[sm_currentScene]->Init();
  }

  sm_isRestarting = false;
}

void SceneManager::Update(float dt)
{
  if (sm_isChanging)
  {
    ChangeStates();
  }
  else if (sm_isRestarting)
  {
    ResetState();
  }

  sm_scenes[sm_currentScene]->Update(dt);
}