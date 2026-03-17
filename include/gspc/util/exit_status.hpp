#pragma once


  namespace gspc::util
  {
    int wexitstatus (int);
    int wifcontinued (int);
    int wifexited (int);
    int wifsignaled (int);
    int wifstopped (int);

    int wstopsig (int);
    int wtermsig (int);
  }
