/**
* 
* @file
*
* @brief  Main program function
*
* @author vitamin.caig@gmail.com
*
**/

//local includes
#include <apps/base/app.h>
//common includes
#include <error.h>
//std includes
#include <locale>
//text includes
#include "../text/base_text.h"

#ifdef UNICODE
std::basic_ostream<Char>& StdOut = std::wcout;
#else 
std::basic_ostream<Char>& StdOut = std::cout;
#endif

int main(int argc, char* argv[])
{
  try
  {
    std::locale::global(std::locale(""));
    std::auto_ptr<Application> app(Application::Create());
    return app->Run(argc, argv);
  }
  catch (const Error& e)
  {
    StdOut << e.ToString();
    return 1;
  }
}
