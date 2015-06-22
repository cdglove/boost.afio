#include "afio_pch.hpp"

int main(void)
{
  using namespace boost::afio;
  auto dispatcher = boost::afio::make_async_file_io_dispatcher();

  auto mkdir(dispatcher->dir(async_path_op_req("testdir", file_flags::Create)));
  try
  {
    for(size_t n = 1; n < 256; n++)
    {
      char cs[2] = {(char) n, 0};
      path p(cs);
      try
      {
        auto mkfile(dispatcher->file(
        async_path_op_req::relative(mkdir, p, boost::afio::file_flags::Create)));
        mkfile.get();
        auto rmfile(dispatcher->close(dispatcher->rmfile(mkfile)));
        std::cout << "Character " << n << " (" << p
                  << ") is permitted on this operating system." << std::endl;
      }
      catch(...)
      {
        std::cout << "Character " << n << " (" << p
                  << ") failed on this operating system." << std::endl;
      }
    }
  }
  catch(...)
  {
    std::cerr << boost::current_exception_diagnostic_information(true) << std::endl;
    throw;
  }
  auto rmdir(dispatcher->close(dispatcher->rmdir(mkdir)));
  rmdir.get();
}
