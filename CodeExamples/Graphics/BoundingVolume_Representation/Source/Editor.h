/*
Copyright (C) 2019DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the
prior written consent of DigiPen Institute of Technology is prohibited.
File Name: Editor.h, declaration for Editor class (user interactions)
Language: C++
Platform: Working GPU, Windows OS
Project: Assignment1_CS300
Author: Rahil Momin, r.momin, 180001717
Creation date: 3-10-19
*/
#pragma once

#include <imgui.h>

class Editor
{
public:
  ~Editor();

  static Editor* editor;

  static Editor* Instance()
  {
    if (!editor)
    {
      editor = new Editor();
    }

    return editor;
  }

  void UpdateP1();
  void UpdateP2();

  void ReloadShaders();
private:
  Editor();

  void MenuEditor(const ImVec2& screenSize);
  void LightsEditor(const ImVec2& screenSize);
  void PerLightMenu(const ImVec2& screenSize);

  void TextureMenu(const ImVec2& screenSize);
};