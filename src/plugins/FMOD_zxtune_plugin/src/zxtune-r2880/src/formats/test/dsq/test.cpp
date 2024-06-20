/**
*
* @file
*
* @brief  DataSqueezer test
*
* @author vitamin.caig@gmail.com
*
**/

#include "../utils.h"

int main()
{
  try
  {
    std::vector<std::string> tests;
    tests.push_back("4kfixed.bin");
    tests.push_back("win512.bin");
    tests.push_back("win1024.bin");
    tests.push_back("win2048.bin");
    //tests.push_back("win4096.bin");
    tests.push_back("win8192.bin");
    //tests.push_back("win16384.bin");
    tests.push_back("win32768.bin");
    tests.push_back("60005f00.bin");
    tests.push_back("60006100.bin");

    const Formats::Packed::Decoder::Ptr decoder = Formats::Packed::CreateDataSquieezerDecoder();
    Test::TestPacked(*decoder, "etalon.bin", tests);
  }
  catch (const std::exception& e)
  {
    std::cout << e.what() << std::endl;
    return 1;
  }
}
