#include "test_functions.hpp"

BOOST_AFIO_AUTO_TEST_CASE(async_io_statfs, "Tests statfs", 20)
{
  using namespace BOOST_AFIO_V1_NAMESPACE;
  auto dispatcher = make_async_file_io_dispatcher();
  std::cout << "\n\nTesting statfs:\n";
  auto mkdir(dispatcher->dir(async_path_op_req("testdir", file_flags::Create)));
  auto mkfile(dispatcher->file(async_path_op_req::relative(mkdir, "foo", file_flags::Create | file_flags::ReadWrite)));
  auto statfs_(dispatcher->statfs(mkfile, fs_metadata_flags::All));
  auto delfile(dispatcher->rmfile(statfs_.second));
  auto closefile = dispatcher->close(delfile);
  BOOST_CHECK_NO_THROW(when_all({mkdir, mkfile, statfs_.second, closefile, delfile}).get());
  auto deldir(dispatcher->rmdir(mkdir));
  BOOST_CHECK_NO_THROW(when_all(deldir).wait());  // virus checkers sometimes make this spuriously fail

  auto statfs(statfs_.first.get());
#define PRINT_FIELD(field, ...)                                                                                                                                                                                                                                                                                                \
  std::cout << "  f_flags." #field ": ";                                                                                                                                                                                                                                                                                       \
  std::cout << statfs.f_flags.field __VA_ARGS__ << std::endl
  PRINT_FIELD(rdonly);
  PRINT_FIELD(noexec);
  PRINT_FIELD(nosuid);
  PRINT_FIELD(acls);
  PRINT_FIELD(xattr);
  PRINT_FIELD(compression);
  PRINT_FIELD(extents);
  PRINT_FIELD(filecompression);
#undef PRINT_FIELD
#define PRINT_FIELD(field, ...)                                                                                                                                                                                                                                                                                                \
  std::cout << "  f_" #field ": ";                                                                                                                                                                                                                                                                                             \
  std::cout << statfs.f_##field __VA_ARGS__ << std::endl
  PRINT_FIELD(bsize);
  PRINT_FIELD(iosize);
  PRINT_FIELD(blocks, << " (" << (statfs.f_blocks * statfs.f_bsize / 1024.0 / 1024.0 / 1024.0) << " Gb)");
  PRINT_FIELD(bfree, << " (" << (statfs.f_bfree * statfs.f_bsize / 1024.0 / 1024.0 / 1024.0) << " Gb)");
  PRINT_FIELD(bavail, << " (" << (statfs.f_bavail * statfs.f_bsize / 1024.0 / 1024.0 / 1024.0) << " Gb)");
  PRINT_FIELD(files);
  PRINT_FIELD(ffree);
  PRINT_FIELD(namemax);
#ifndef WIN32
  PRINT_FIELD(owner);
#endif
  PRINT_FIELD(fsid, [0] << statfs.f_fsid[1]);
  PRINT_FIELD(fstypename);
  PRINT_FIELD(mntfromname);
  PRINT_FIELD(mntonname);
#undef PRINT_FIELD
}
