#include <iostream>

int main (int, char *[], char *envp[])
{
  for (char **env = envp; env && *env != nullptr; ++env)
  {
    std::cout << *env << std::endl;
  }
  return 0;
}
