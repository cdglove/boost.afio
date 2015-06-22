#include "test_functions.hpp"

BOOST_AFIO_AUTO_TEST_CASE(api_error_check, "Tests that every API returns errors as it is supposed to", 20)
{
  using namespace BOOST_AFIO_V1_NAMESPACE;
  auto dispatcher = make_async_file_io_dispatcher();
#define BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(call, errcode)                                                                                                                                                                                                                                                                      \
  try                                                                                                                                                                                                                                                                                                                          \
  {                                                                                                                                                                                                                                                                                                                            \
    BOOST_TEST_MESSAGE("Testing " #call);                                                                                                                                                                                                                                                                                      \
    call;                                                                                                                                                                                                                                                                                                                      \
    BOOST_FAIL("Exception not thrown by " #call);                                                                                                                                                                                                                                                                              \
  }                                                                                                                                                                                                                                                                                                                            \
  catch(const system_error &e)                                                                                                                                                                                                                                                                                                 \
  {                                                                                                                                                                                                                                                                                                                            \
    BOOST_CHECK(e.code().value() == errcode);                                                                                                                                                                                                                                                                                  \
    std::cout << "\nsystem_error message from " #call " was: " << e.what() << std::endl;                                                                                                                                                                                                                                       \
  }                                                                                                                                                                                                                                                                                                                            \
  catch(...)                                                                                                                                                                                                                                                                                                                   \
  {                                                                                                                                                                                                                                                                                                                            \
    BOOST_FAIL("Exception thrown by " #call " was not a system_error");                                                                                                                                                                                                                                                        \
  }
#ifdef WIN32
#define BOOST_AFIO_FILE_NOT_FOUND_ERRCODE ERROR_FILE_NOT_FOUND
#define BOOST_AFIO_BAD_FD ERROR_INVALID_HANDLE
#else
#define BOOST_AFIO_FILE_NOT_FOUND_ERRCODE ENOENT
#define BOOST_AFIO_BAD_FD EBADF
#endif

  // Create a bad handle
  async_io_op op = dispatcher->file(async_path_op_req("testfile", file_flags::Create | file_flags::ReadWrite));
  auto h = op.get();
  dispatcher->close(dispatcher->rmfile(op)).get();
  char buffer[32];

  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->dir("should not exist this").get(), BOOST_AFIO_FILE_NOT_FOUND_ERRCODE);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->file("should not exist this").get(), BOOST_AFIO_FILE_NOT_FOUND_ERRCODE);
  // No point checking sync(), he won't call if no bytes have been written
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->zero(op, {{0, 0}}).get(), BOOST_AFIO_BAD_FD);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->read(make_async_data_op_req(op, buffer, 32)).get(), BOOST_AFIO_BAD_FD);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->write(make_async_data_op_req(op, buffer, 32)).get(), BOOST_AFIO_BAD_FD);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->truncate(op, 32).get(), BOOST_AFIO_BAD_FD);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->enumerate(op).first.get(), BOOST_AFIO_BAD_FD);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->enumerate(op).second.get(), BOOST_AFIO_BAD_FD);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->extents(op).first.get(), BOOST_AFIO_BAD_FD);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->extents(op).second.get(), BOOST_AFIO_BAD_FD);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->statfs(op, fs_metadata_flags::All).first.get(), BOOST_AFIO_BAD_FD);
  BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE(dispatcher->statfs(op, fs_metadata_flags::All).second.get(), BOOST_AFIO_BAD_FD);

#undef BOOST_AFIO_CHECK_SYSTEM_ERROR_CODE
}
