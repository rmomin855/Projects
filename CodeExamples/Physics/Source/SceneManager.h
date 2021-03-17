#ifndef SCENEMANAGER
#define SCENEMANAGER

#include <unordered_map>
#include <memory>

enum class Scenes
{
  Invalid = -1,
  SceneSetup,

  FirstScene = SceneSetup,
};

class Scene
{
public:
  virtual void Load() {};
  virtual void Init() {};
  virtual void Update(float dt) {};
  virtual void DeInit() {};
  virtual void UnLoad() {};
};

class SceneManager
{
public:
  static SceneManager* Instance()
  {
    if (!sm_singleton)
    {
      sm_singleton = new SceneManager();
    }

    return sm_singleton;
  }
  static void DeleteInstance()
  {
    delete sm_singleton;
    sm_singleton = nullptr;
  }

  void SetNextScene(Scenes scene);


  void ChangeStates();
  void ResetState();

  void Update(float dt);

  Scene* GetCurrent() { return sm_scenes[sm_currentScene]; }

  bool isDirty = false;
  bool sm_isChanging;

private:
  static SceneManager* sm_singleton;
  SceneManager();
  ~SceneManager();

  std::unordered_map<Scenes, Scene*> sm_scenes;

  Scenes sm_currentScene;
  Scenes sm_nextScene;
  Scenes sm_prevScene;
  bool sm_isRestarting;
};

#endif // !1
