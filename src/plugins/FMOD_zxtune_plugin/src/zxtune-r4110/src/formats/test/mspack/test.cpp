/**
*
* @file
*
* @brief  MsPack test
*
* @author vitamin.caig@gmail.com
*
**/

#include "../utils.h"

int main()
{
  std::vector<std::string> tests;
  tests.push_back("slow.msp:229");
  tests.push_back("fast.msp:229");

  try
  {
    const Formats::Packed::Decoder::Ptr decoder = Formats::Packed::CreateMSPackDecoder();
    Test::TestPacked(*decoder, "etalon.bin", tests);
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }
}
