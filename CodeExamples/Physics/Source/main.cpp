#include "System.h"

int main()
{
  System* system = System::Instance(1300, 900);
  system->Init();

  while (system->Running())
  {
    system->Update(1.0f / 60.0f);
  }

  system->cleanup();
  System::DeleteInstance();
  return 0;
}