/**
  *  \file u/t_util_plugin_installer.cpp
  *  \brief Test for util::plugin::Installer
  */

#include "util/plugin/installer.hpp"

#include "t_util_plugin.hpp"
#include "afl/io/internalfilesystem.hpp"
#include "afl/string/nulltranslator.hpp"
#include "afl/sys/log.hpp"
#include "util/plugin/manager.hpp"

namespace {
    // explmap.c2z plugin
    //   -rw-r--r--  3.0 unx      116 tx defN 15-Apr-10 22:21 explmap.c2p
    //   -rw-r--r--  3.0 unx     1437 tx defN 15-Apr-10 22:21 explmap.q
    const uint8_t FILE_CONTENT[] = {
        0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0xba, 0xb2, 0x8a, 0x46, 0x1e, 0x46,
        0xc1, 0x59, 0x63, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x1c, 0x00, 0x65, 0x78,
        0x70, 0x6c, 0x6d, 0x61, 0x70, 0x2e, 0x63, 0x32, 0x70, 0x55, 0x54, 0x09, 0x00, 0x03, 0xdf, 0x30,
        0x28, 0x55, 0x8f, 0x16, 0x74, 0x56, 0x75, 0x78, 0x0b, 0x00, 0x01, 0x04, 0xe8, 0x03, 0x00, 0x00,
        0x04, 0xe8, 0x03, 0x00, 0x00, 0x1d, 0x8d, 0x31, 0x0e, 0xc2, 0x30, 0x10, 0x04, 0x7b, 0xbf, 0x62,
        0x5f, 0xc0, 0x0f, 0xe8, 0x80, 0x0e, 0x1a, 0xf2, 0x81, 0x93, 0xb5, 0xc2, 0x27, 0x9c, 0xf3, 0x71,
        0x76, 0x14, 0x9e, 0x8f, 0x43, 0x3d, 0x33, 0x9a, 0x87, 0xac, 0xc4, 0x19, 0xd7, 0xaf, 0xd7, 0x16,
        0xc4, 0x5d, 0x1c, 0x4b, 0x48, 0x7e, 0x33, 0xd2, 0x85, 0x3d, 0x87, 0xfa, 0xd0, 0x66, 0xd3, 0x58,
        0x8a, 0x76, 0x78, 0xdd, 0x5e, 0x6a, 0x08, 0x7a, 0x8b, 0xd1, 0x41, 0xc9, 0x05, 0x63, 0x0b, 0xc3,
        0x5e, 0x64, 0xc0, 0xb8, 0x4f, 0x43, 0x8c, 0x13, 0x8d, 0x42, 0xa4, 0xe7, 0xbf, 0xbf, 0x69, 0x3d,
        0x16, 0x9c, 0x8b, 0x55, 0xfc, 0xf4, 0x49, 0x3f, 0x50, 0x4b, 0x03, 0x04, 0x14, 0x00, 0x00, 0x00,
        0x08, 0x00, 0xba, 0xb2, 0x8a, 0x46, 0xd4, 0x92, 0x31, 0xda, 0xe1, 0x02, 0x00, 0x00, 0x9d, 0x05,
        0x00, 0x00, 0x09, 0x00, 0x1c, 0x00, 0x65, 0x78, 0x70, 0x6c, 0x6d, 0x61, 0x70, 0x2e, 0x71, 0x55,
        0x54, 0x09, 0x00, 0x03, 0xdf, 0x30, 0x28, 0x55, 0x8f, 0x16, 0x74, 0x56, 0x75, 0x78, 0x0b, 0x00,
        0x01, 0x04, 0xe8, 0x03, 0x00, 0x00, 0x04, 0xe8, 0x03, 0x00, 0x00, 0x75, 0x54, 0x61, 0x6b, 0xdb,
        0x30, 0x10, 0xfd, 0xbc, 0x40, 0xfe, 0xc3, 0xcd, 0xd0, 0x92, 0xd0, 0xd4, 0x4d, 0x0a, 0xdd, 0x46,
        0xa0, 0x83, 0x2e, 0xcb, 0x20, 0x63, 0x4d, 0x03, 0x2d, 0x74, 0x63, 0xec, 0x83, 0x62, 0x5f, 0x12,
        0x51, 0x45, 0xf2, 0x24, 0xb9, 0x8e, 0xf7, 0xeb, 0x77, 0x27, 0xd9, 0x8d, 0x19, 0x5d, 0xbe, 0xd8,
        0xd6, 0xe9, 0xde, 0xbd, 0x7b, 0xf7, 0x2e, 0x27, 0xfd, 0xde, 0x09, 0xc0, 0xfc, 0x50, 0x28, 0x63,
        0x11, 0xf6, 0xa2, 0x00, 0x6f, 0x45, 0xf6, 0x84, 0x96, 0xce, 0x43, 0xe8, 0x61, 0x27, 0x1d, 0xb8,
        0xcc, 0xca, 0xc2, 0xc7, 0x90, 0x03, 0x8d, 0x95, 0xaa, 0xe9, 0x4c, 0x68, 0x8d, 0x39, 0x14, 0x4a,
        0x68, 0xf4, 0x0e, 0x2a, 0xe9, 0x77, 0xe0, 0x77, 0x08, 0x22, 0xcf, 0xcf, 0x8d, 0x6e, 0x31, 0x6f,
        0x45, 0x91, 0x06, 0xa0, 0x47, 0x8e, 0x1f, 0x0f, 0x39, 0xaf, 0x46, 0xeb, 0xc0, 0x62, 0x86, 0xf2,
        0x19, 0x01, 0x45, 0x46, 0xf9, 0xa5, 0xd5, 0x20, 0xb8, 0x02, 0x7c, 0xff, 0xc1, 0xc8, 0x69, 0x2e,
        0x3c, 0x38, 0x6f, 0xac, 0xd4, 0x5b, 0x30, 0x2a, 0x07, 0xa1, 0xf3, 0x10, 0x6e, 0xca, 0x46, 0xec,
        0xe5, 0xf1, 0x00, 0x4c, 0x96, 0x95, 0x16, 0xd6, 0xc2, 0x11, 0x37, 0xa2, 0xc1, 0x8c, 0x22, 0x55,
        0x0b, 0x0e, 0xbd, 0x27, 0x1c, 0x07, 0xb2, 0x4b, 0x2f, 0x40, 0xd2, 0x0d, 0x58, 0x23, 0x75, 0x28,
        0xb3, 0xa7, 0x1a, 0xbc, 0x81, 0x8d, 0x51, 0xca, 0x54, 0x69, 0xa3, 0x01, 0xb6, 0x12, 0x3c, 0xa3,
        0x95, 0x1b, 0x89, 0x8e, 0xe2, 0x36, 0x52, 0x8e, 0x75, 0x41, 0x6e, 0x88, 0xb7, 0x2b, 0x30, 0x93,
        0x42, 0x41, 0x61, 0x4d, 0x81, 0xd6, 0xd7, 0xe7, 0x5e, 0x6c, 0x61, 0x63, 0xcd, 0x9e, 0x62, 0x94,
        0xb0, 0x27, 0x0e, 0xa1, 0x43, 0x3c, 0x48, 0xd7, 0x72, 0x5f, 0x6c, 0x40, 0x1b, 0x0f, 0x03, 0xcf,
        0x42, 0xef, 0x51, 0x68, 0x17, 0x38, 0x37, 0xb0, 0x95, 0x70, 0x90, 0xa3, 0xc7, 0xcc, 0x53, 0x3b,
        0x32, 0xb6, 0x43, 0xfd, 0x59, 0xd4, 0x3e, 0x40, 0x0d, 0x09, 0xab, 0x50, 0x3c, 0x37, 0x92, 0xdd,
        0x51, 0x99, 0xbd, 0xb0, 0x34, 0x3d, 0x18, 0x6c, 0x2d, 0xa2, 0xa6, 0xb6, 0x32, 0x5f, 0xba, 0x61,
        0x28, 0x44, 0x4d, 0x05, 0x31, 0xbc, 0xb0, 0x7c, 0x9f, 0x1b, 0x20, 0x6d, 0x15, 0x0a, 0xe7, 0x49,
        0x28, 0x0c, 0x70, 0x91, 0xd2, 0x4d, 0x9e, 0x4b, 0x2f, 0x8d, 0x16, 0x4a, 0xd5, 0x23, 0xc6, 0x44,
        0xe7, 0xc4, 0x16, 0x61, 0x6d, 0x0e, 0x34, 0x66, 0xa5, 0x88, 0x08, 0x77, 0xd3, 0xf2, 0xa4, 0x31,
        0xc2, 0xce, 0x54, 0x54, 0x5a, 0xd7, 0xdd, 0xd9, 0xc0, 0x4e, 0xd0, 0x5c, 0xd7, 0xcc, 0xa3, 0x31,
        0x4b, 0xda, 0xba, 0x6a, 0xf2, 0xe1, 0x62, 0x69, 0x9e, 0x2f, 0x2e, 0xc7, 0xe3, 0x77, 0x53, 0x78,
        0x20, 0x7d, 0x1e, 0x4b, 0xb4, 0x7f, 0x82, 0xeb, 0x28, 0x38, 0xbe, 0xb8, 0x29, 0x2c, 0x05, 0x27,
        0x57, 0x53, 0xf0, 0x15, 0x8a, 0x27, 0xea, 0x9d, 0xe9, 0x4a, 0x9d, 0xa9, 0xd2, 0x11, 0x31, 0x56,
        0x62, 0x35, 0x9b, 0x5d, 0x52, 0xa5, 0x72, 0x4b, 0xef, 0x4a, 0xae, 0xad, 0xb0, 0x75, 0x40, 0xef,
        0xf7, 0x56, 0x64, 0x16, 0x0f, 0xc9, 0xcf, 0x46, 0x99, 0xf4, 0x77, 0x17, 0xf1, 0x57, 0xc2, 0x57,
        0x66, 0x16, 0x85, 0xc7, 0x55, 0xe0, 0xb9, 0x6a, 0x86, 0xd5, 0x2a, 0x99, 0x3a, 0x62, 0xcc, 0x97,
        0xee, 0xcb, 0x75, 0x7b, 0xd6, 0xef, 0x01, 0x7c, 0x33, 0x19, 0x8d, 0x96, 0xfb, 0xbb, 0x86, 0xf1,
        0x08, 0x3c, 0x3d, 0x1e, 0x48, 0xb2, 0xb3, 0xc9, 0x31, 0xc8, 0x06, 0xbd, 0x86, 0x99, 0x29, 0xb5,
        0x8f, 0xd8, 0x6e, 0xb0, 0xa4, 0xd9, 0x2e, 0xdc, 0x7c, 0x5f, 0xf8, 0x7a, 0xd0, 0x2d, 0x30, 0x1c,
        0x72, 0x1a, 0x0d, 0x9f, 0x73, 0xc8, 0x60, 0x9a, 0x3f, 0x01, 0x4e, 0x60, 0x11, 0x65, 0x13, 0x8a,
        0x28, 0xe6, 0xb4, 0x66, 0x41, 0x3e, 0xb3, 0xc7, 0x17, 0xc7, 0xc3, 0xe3, 0x8e, 0xe6, 0xb6, 0xe0,
        0x08, 0x39, 0xa7, 0x02, 0x19, 0x36, 0x32, 0x8d, 0xf9, 0x5f, 0x8c, 0x9d, 0xb3, 0x2b, 0x63, 0x79,
        0xf8, 0x6c, 0xe2, 0x71, 0xa8, 0xf4, 0x2a, 0x8d, 0x4e, 0x6d, 0xfe, 0xd1, 0x26, 0xdd, 0x46, 0x07,
        0x45, 0x84, 0x54, 0x99, 0x2c, 0x3d, 0x8c, 0x3a, 0x1f, 0xe4, 0x88, 0xf7, 0x23, 0xb8, 0x1a, 0x41,
        0x92, 0x8c, 0x82, 0x0e, 0xfd, 0xde, 0x9b, 0x2e, 0x22, 0x4c, 0xaf, 0x61, 0x72, 0x04, 0x64, 0xbd,
        0xe8, 0x84, 0x1f, 0x67, 0xc7, 0xf3, 0xb9, 0xce, 0x17, 0x9b, 0xf8, 0xb1, 0xc4, 0x83, 0x8f, 0x6f,
        0xbc, 0x09, 0x74, 0xed, 0x23, 0x8c, 0xd9, 0x59, 0x2f, 0xa4, 0x6e, 0xa3, 0xfb, 0x3e, 0x91, 0xf9,
        0x92, 0x3b, 0x5a, 0xed, 0x66, 0x9b, 0x1d, 0x14, 0xb4, 0xab, 0xe4, 0x8c, 0xb2, 0x80, 0x04, 0x4e,
        0x43, 0xea, 0x29, 0xbd, 0x75, 0xff, 0x1c, 0x88, 0x61, 0xd2, 0x2c, 0x3a, 0x5b, 0xfe, 0x3e, 0x66,
        0x4e, 0xc3, 0xff, 0x45, 0x33, 0xa0, 0xb7, 0x49, 0xac, 0xf3, 0xc2, 0x68, 0xae, 0x1c, 0xfe, 0x33,
        0x0b, 0xde, 0xcf, 0xd0, 0x1a, 0x19, 0x9c, 0xd6, 0x54, 0x6f, 0x53, 0xf8, 0x5a, 0x3a, 0x3e, 0xf3,
        0xcd, 0xbe, 0xb9, 0xff, 0xc9, 0x0f, 0xaf, 0x4a, 0xd3, 0x14, 0xa3, 0x07, 0x79, 0x8c, 0xad, 0x76,
        0xa7, 0xc9, 0x41, 0x22, 0xe7, 0x84, 0x79, 0xeb, 0xb8, 0x7e, 0xef, 0x2f, 0x50, 0x4b, 0x01, 0x02,
        0x1e, 0x03, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0xba, 0xb2, 0x8a, 0x46, 0x1e, 0x46, 0xc1, 0x59,
        0x63, 0x00, 0x00, 0x00, 0x74, 0x00, 0x00, 0x00, 0x0b, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x01, 0x00, 0x00, 0x00, 0xa4, 0x81, 0x00, 0x00, 0x00, 0x00, 0x65, 0x78, 0x70, 0x6c, 0x6d, 0x61,
        0x70, 0x2e, 0x63, 0x32, 0x70, 0x55, 0x54, 0x05, 0x00, 0x03, 0xdf, 0x30, 0x28, 0x55, 0x75, 0x78,
        0x0b, 0x00, 0x01, 0x04, 0xe8, 0x03, 0x00, 0x00, 0x04, 0xe8, 0x03, 0x00, 0x00, 0x50, 0x4b, 0x01,
        0x02, 0x1e, 0x03, 0x14, 0x00, 0x00, 0x00, 0x08, 0x00, 0xba, 0xb2, 0x8a, 0x46, 0xd4, 0x92, 0x31,
        0xda, 0xe1, 0x02, 0x00, 0x00, 0x9d, 0x05, 0x00, 0x00, 0x09, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x01, 0x00, 0x00, 0x00, 0xa4, 0x81, 0xa8, 0x00, 0x00, 0x00, 0x65, 0x78, 0x70, 0x6c, 0x6d,
        0x61, 0x70, 0x2e, 0x71, 0x55, 0x54, 0x05, 0x00, 0x03, 0xdf, 0x30, 0x28, 0x55, 0x75, 0x78, 0x0b,
        0x00, 0x01, 0x04, 0xe8, 0x03, 0x00, 0x00, 0x04, 0xe8, 0x03, 0x00, 0x00, 0x50, 0x4b, 0x05, 0x06,
        0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0xa0, 0x00, 0x00, 0x00, 0xcc, 0x03, 0x00, 0x00,
        0x00, 0x00
    };
}

/** Test installation.
    Many more usecases are tested by PluginManagerProxy. */
void
TestUtilPluginInstaller::testInstallZip()
{
    // Environment
    afl::string::NullTranslator tx;
    afl::sys::Log log;
    afl::io::InternalFileSystem fs;
    fs.createDirectory("/plugins");
    fs.createDirectory("/source");
    fs.openFile("/source/file.c2z", afl::io::FileSystem::Create)
        ->fullWrite(FILE_CONTENT);

    // Test object
    util::plugin::Manager mgr(tx, log);
    afl::base::Ref<afl::io::Directory> dir = fs.openDirectory("/plugins");
    util::plugin::Installer testee(mgr, fs, *dir);

    // Perform installation sequence
    // - prepare
    util::plugin::Plugin* plug = testee.prepareInstall("/source/file.c2z");
    TS_ASSERT(plug != 0);

    // - examine
    String_t tmp;
    TS_ASSERT_EQUALS(testee.checkInstallAmbiguity(tmp), util::plugin::Installer::NoPlugin);
    TS_ASSERT_EQUALS(testee.checkInstallPreconditions().isValid(), false);

    // - install
    testee.doInstall();

    // Verify installation
    // Name is taken from the *.c2p file inside the zip
    TS_ASSERT(mgr.getPluginById("EXPLMAP") != 0);
    TS_ASSERT_EQUALS(fs.openFile("/plugins/explmap/explmap.q", afl::io::FileSystem::OpenRead)->getSize(), 1437U);
    TS_ASSERT_EQUALS(fs.openFile("/plugins/explmap.c2p",       afl::io::FileSystem::OpenRead)->getSize(), 116U);
}

