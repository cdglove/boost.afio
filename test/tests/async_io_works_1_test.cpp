#include "test_functions.hpp"

BOOST_AFIO_AUTO_TEST_CASE(async_io_works_1, "Tests that the async i/o implementation works", 60)
{
  using namespace BOOST_AFIO_V1_NAMESPACE;
  namespace asio = BOOST_AFIO_V1_NAMESPACE::asio;
  auto dispatcher = make_async_file_io_dispatcher(process_threadpool(), file_flags::None);
  std::cout << "\n\n1000 file opens, writes 1 byte, closes, and deletes:\n";
  _1000_open_write_close_deletes(dispatcher, 1);
}