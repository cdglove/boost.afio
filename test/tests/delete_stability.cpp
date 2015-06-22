#include "test_functions.hpp"

BOOST_AFIO_AUTO_TEST_CASE(delete_stability, "Tests that deleting files and directories always succeeds", 120)
{
  using namespace BOOST_AFIO_V1_NAMESPACE;
  static BOOST_CONSTEXPR_OR_CONST size_t ITERATIONS = 1000;
  static BOOST_CONSTEXPR_OR_CONST size_t ITEMS = 10;
  // Oh Windows, oh Windows, how strange you are ...
  for(size_t n = 0; n < 10; n++)
  {
    try
    {
      if(filesystem::exists("testdir"))
        filesystem::remove_all("testdir");
      break;
    }
    catch(...)
    {
      this_thread::sleep_for(chrono::milliseconds(10));
    }
  }

  try
  {
    // HoldParentOpen is actually ineffectual as renames zap the parent container, but it tests more code.
    auto dispatcher = make_async_file_io_dispatcher(process_threadpool(), file_flags::HoldParentOpen);
    auto testdir = dispatcher->dir(async_path_op_req("testdir", file_flags::Create));

    // Monte Carlo creating or deleting lots of directories containing a few files of 4Kb
    ranctx ctx;
    raninit(&ctx, 1);
    for(size_t iter = 0; iter < ITERATIONS; iter++)
    {
      size_t idx = ranval(&ctx) % ITEMS;
      if(filesystem::exists("testdir/" + to_string(idx)))
      {
        auto dirh = dispatcher->dir(async_path_op_req::relative(testdir, to_string(idx), file_flags::ReadWrite));
        std::vector<async_path_op_req> reqs = {async_path_op_req::relative(dirh, "a", file_flags::ReadWrite), async_path_op_req::relative(dirh, "b", file_flags::ReadWrite), async_path_op_req::relative(dirh, "c", file_flags::ReadWrite)};
        auto files = dispatcher->file(reqs);
        // Go synchronous for these
        for(auto &i : files)
          i->unlink();
        dirh->unlink();
      }
      else
      {
        static char buffer[4096];
        auto dirh = dispatcher->dir(async_path_op_req::relative(testdir, to_string(idx), file_flags::Create));
        std::vector<async_path_op_req> reqs = {async_path_op_req::relative(dirh, "a", file_flags::Create | file_flags::ReadWrite), async_path_op_req::relative(dirh, "b", file_flags::Create | file_flags::ReadWrite), async_path_op_req::relative(dirh, "c", file_flags::Create | file_flags::ReadWrite)};
        auto files = dispatcher->file(reqs);
        auto resized = dispatcher->truncate(files, {sizeof(buffer), sizeof(buffer), sizeof(buffer)});
        std::vector<async_data_op_req<char>> reqs2;
        for(auto &i : resized)
          reqs2.push_back(make_async_data_op_req(i, buffer, 0));
        auto written = dispatcher->write(reqs2);
        dirh.get();
        when_all(files.begin(), files.end()).get();
        when_all(resized.begin(), resized.end()).get();
        when_all(written.begin(), written.end()).get();
      }
    }
    BOOST_CHECK(true);

    filesystem::remove_all("testdir");
  }
  catch(...)
  {
    BOOST_CHECK(false);
    filesystem::remove_all("testdir");
    throw;
  }
  //]
}
