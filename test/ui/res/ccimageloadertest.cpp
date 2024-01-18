/**
  *  \file test/ui/res/ccimageloadertest.cpp
  *  \brief Test for ui::res::CCImageLoader
  */

#include "ui/res/ccimageloader.hpp"

#include "afl/io/constmemorystream.hpp"
#include "afl/test/testrunner.hpp"
#include "gfx/types.hpp"

namespace {
    String_t decodePixels(afl::base::Memory<gfx::ColorQuad_t> cs)
    {
        String_t result;
        while (gfx::ColorQuad_t* c = cs.eat()) {
            if (ALPHA_FROM_COLORQUAD(*c) == 0) {
                result += ' ';
            } else {
                switch (*c) {
                 case COLORQUAD_FROM_RGB(0, 0, 0):       result += 'b'; break;
                 case COLORQUAD_FROM_RGB(255, 255, 255): result += 'W'; break;
                 case COLORQUAD_FROM_RGB(64, 129, 64):   result += '~'; break;
                 case COLORQUAD_FROM_RGB(194, 194, 194): result += '.'; break;
                 case COLORQUAD_FROM_RGB(222, 222, 222): result += ':'; break;
                 case COLORQUAD_FROM_RGB(97, 97, 97):    result += '+'; break;
                 case COLORQUAD_FROM_RGB(125, 125, 125): result += '*'; break;
                 case COLORQUAD_FROM_RGB(141, 141, 141): result += '#'; break;
                 case COLORQUAD_FROM_RGB(238, 238, 238): result += '$'; break;
                 default:                                result += '?'; break;
                }
            }
        }
        return result;
    }

    String_t decodeRow(gfx::Canvas& can, int width, int row)
    {
        std::vector<gfx::Color_t> color(width);
        std::vector<gfx::ColorQuad_t> quad(width);
        can.getPixels(gfx::Point(0, row), color);
        can.decodeColors(color, quad);
        return decodePixels(quad);
    }
}

/** Load a compressed ".cd" image. */
AFL_TEST("ui.res.CCImageLoader:compressed-cd", a)
{
    // share/resource/ui/cb0.cd
    static const uint8_t IMAGE[] = {
        0xc6, 0x01, 0x00, 0x00, 0xc6, 0x01, 0xff, 0x43, 0x44, 0x10, 0x00, 0x10,
        0xff, 0xc1, 0x00, 0x37, 0xff, 0x0e, 0x00, 0x37, 0x00, 0xff, 0x0e, 0x3d,
        0x0f, 0x00, 0xff, 0x0e, 0x3d, 0x0f, 0x00, 0xff, 0x0e, 0x3d, 0x0f, 0x00,
        0xff, 0x0e, 0x3d, 0x0f, 0x00, 0xff, 0x0e, 0x3d, 0x0f, 0x00, 0xff, 0x0e,
        0x3d, 0x0f, 0x00, 0xff, 0x0e, 0x3d, 0x0f, 0x00, 0xff, 0x0e, 0x3d, 0x0f,
        0x00, 0xff, 0x0e, 0x3d, 0x0f, 0x00, 0xff, 0x0e, 0x3d, 0x0f, 0x00, 0xff,
        0x0e, 0x3d, 0x0f, 0x00, 0xff, 0x0e, 0x3d, 0x0f, 0x00, 0xff, 0x0e, 0x3d,
        0x0f, 0x00, 0xff, 0x0e, 0x3d, 0x0f, 0x37, 0xff, 0x0e, 0x0f, 0x3e, 0x00,
        0x00
    };
    afl::io::ConstMemoryStream ms(IMAGE);
    ui::res::CCImageLoader testee;

    // Load it
    afl::base::Ptr<gfx::Canvas> can(testee.loadImage(ms));
    a.checkNonNull("01. result", can.get());
    a.checkEqual("02. size x", can->getSize().getX(), 16);
    a.checkEqual("03. size y", can->getSize().getY(), 16);

    // Verify pixels
    a.checkEqual("11", decodeRow(*can, 16,  0), "*bbbbbbbbbbbbbb*");
    a.checkEqual("12", decodeRow(*can, 16,  1), "b::::::::::::::W");
    a.checkEqual("13", decodeRow(*can, 16,  2), "b::::::::::::::W");
    a.checkEqual("14", decodeRow(*can, 16,  3), "b::::::::::::::W");
    a.checkEqual("15", decodeRow(*can, 16,  4), "b::::::::::::::W");
    a.checkEqual("16", decodeRow(*can, 16,  5), "b::::::::::::::W");
    a.checkEqual("17", decodeRow(*can, 16,  6), "b::::::::::::::W");
    a.checkEqual("18", decodeRow(*can, 16,  7), "b::::::::::::::W");
    a.checkEqual("19", decodeRow(*can, 16,  8), "b::::::::::::::W");
    a.checkEqual("20", decodeRow(*can, 16,  9), "b::::::::::::::W");
    a.checkEqual("21", decodeRow(*can, 16, 10), "b::::::::::::::W");
    a.checkEqual("22", decodeRow(*can, 16, 11), "b::::::::::::::W");
    a.checkEqual("23", decodeRow(*can, 16, 12), "b::::::::::::::W");
    a.checkEqual("24", decodeRow(*can, 16, 13), "b::::::::::::::W");
    a.checkEqual("25", decodeRow(*can, 16, 14), "b::::::::::::::W");
    a.checkEqual("26", decodeRow(*can, 16, 15), "*WWWWWWWWWWWWWW$");
}

/** Test an uncompressed ".gfx" image. */
AFL_TEST("ui.res.CCImageLoader:compressed-gfx", a)
{
    // share/resource/ui/radio0.gfx
    static const uint8_t IMAGE[] = {
        0x00, 0x08, 0x14, 0x00, 0x14, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x20, 0x20, 0x20,
        0x20, 0x20, 0x20, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x20, 0x20, 0x18, 0x1d, 0x1d, 0x1d, 0x1d, 0x17, 0x20,
        0x20, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0x20, 0x17,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x20, 0x01, 0x17, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0x20, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x01, 0xff, 0xff, 0xff, 0xff, 0xff,
        0x20, 0x17, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0x20, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x0f,
        0xff, 0xff, 0xff, 0x20, 0x18, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x0f, 0xff, 0xff, 0x20,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x0f, 0xff, 0xff, 0x20, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x0f, 0xff, 0xff, 0x20, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x0f, 0xff, 0xff, 0x20,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x0f, 0xff, 0xff, 0x20, 0x18, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x0f, 0xff, 0xff, 0xff, 0x20, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x0f, 0xff, 0xff, 0xff, 0xff,
        0x20, 0x17, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0x20, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x0f, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0x17, 0x20, 0x01, 0x1d, 0x1d, 0x1d, 0x1d,
        0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x0f, 0x1d, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0x17, 0x01, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x1d, 0x0f,
        0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
        0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
    };

    afl::io::ConstMemoryStream ms(IMAGE);
    ui::res::CCImageLoader testee;

    // Load it
    afl::base::Ptr<gfx::Canvas> can(testee.loadImage(ms));
    a.checkNonNull("01. result", can.get());
    a.checkEqual("02. size x", can->getSize().getX(), 20);
    a.checkEqual("03. size y", can->getSize().getY(), 20);

    // Verify pixels
    a.checkEqual("11", decodeRow(*can, 20,  0), "                    ");
    a.checkEqual("12", decodeRow(*can, 20,  1), "       bbbbbb       ");
    a.checkEqual("13", decodeRow(*can, 20,  2), "     bb#::::*bb     ");
    a.checkEqual("14", decodeRow(*can, 20,  3), "   *b*::::::::b+*   ");
    a.checkEqual("15", decodeRow(*can, 20,  4), "   b::::::::::::+   ");
    a.checkEqual("16", decodeRow(*can, 20,  5), "  b*::::::::::::WW  ");
    a.checkEqual("17", decodeRow(*can, 20,  6), "  b::::::::::::::W  ");
    a.checkEqual("18", decodeRow(*can, 20,  7), " b#:::::::::::::::W ");
    a.checkEqual("19", decodeRow(*can, 20,  8), " b::::::::::::::::W ");
    a.checkEqual("20", decodeRow(*can, 20,  9), " b::::::::::::::::W ");
    a.checkEqual("21", decodeRow(*can, 20, 10), " b::::::::::::::::W ");
    a.checkEqual("22", decodeRow(*can, 20, 11), " b::::::::::::::::W ");
    a.checkEqual("23", decodeRow(*can, 20, 12), " b#:::::::::::::::W ");
    a.checkEqual("24", decodeRow(*can, 20, 13), "  b::::::::::::::W  ");
    a.checkEqual("25", decodeRow(*can, 20, 14), "  b*:::::::::::::W  ");
    a.checkEqual("26", decodeRow(*can, 20, 15), "   b::::::::::::W   ");
    a.checkEqual("27", decodeRow(*can, 20, 16), "   *b+:::::::::W:   ");
    a.checkEqual("28", decodeRow(*can, 20, 17), "     *+::::::WW     ");
    a.checkEqual("29", decodeRow(*can, 20, 18), "       WWWWWW       ");
    a.checkEqual("30", decodeRow(*can, 20, 19), "                    ");
}

/** Test a compressed ".cc" file. */
AFL_TEST("ui.res.CCImageLoader:compressed-cc", a)
{
    // PCC1: cc/res/cb-cond.cc
    static const uint8_t IMAGE[] = {
        0x95, 0x00, 0x00, 0x00, 0x95, 0x00, 0xfe, 0x43, 0x43, 0x10, 0x00, 0x10,
        0xfe, 0x18, 0x00, 0x20, 0xfe, 0x06, 0x22, 0xf2, 0x20, 0xfe, 0x06, 0x22,
        0xf2, 0x20, 0x22, 0x22, 0x55, 0x55, 0x25, 0x22, 0xf2, 0x20, 0x22, 0x52,
        0xfe, 0x03, 0x55, 0x22, 0xf2, 0x20, 0x22, 0x55, 0x25, 0x22, 0x55, 0x25,
        0xf2, 0x20, 0x22, 0x55, 0x22, 0x22, 0x52, 0x22, 0xf2, 0x20, 0x22, 0x55,
        0xfe, 0x04, 0x22, 0xf2, 0x20, 0x22, 0x55, 0xfe, 0x04, 0x22, 0xf2, 0x20,
        0x22, 0x55, 0x22, 0x22, 0x52, 0x22, 0xf2, 0x20, 0x22, 0x55, 0x25, 0x22,
        0x55, 0x25, 0xf2, 0x20, 0x22, 0x52, 0xfe, 0x03, 0x55, 0x22, 0xf2, 0x20,
        0x22, 0x22, 0x55, 0x55, 0x25, 0x22, 0xf2, 0x20, 0xfe, 0x06, 0x22, 0xf2,
        0x20, 0xfe, 0x06, 0x22, 0xf2, 0xf0, 0xfe, 0x07, 0xff, 0x00, 0x00
    };

    afl::io::ConstMemoryStream ms(IMAGE);
    ui::res::CCImageLoader testee;

    // Load it
    afl::base::Ptr<gfx::Canvas> can(testee.loadImage(ms));
    a.checkNonNull("01. result", can.get());
    a.checkEqual("02. size x", can->getSize().getX(), 16);
    a.checkEqual("03. size y", can->getSize().getY(), 16);

    // Verify pixels
    a.checkEqual("11", decodeRow(*can, 16,  0), "bbbbbbbbbbbbbbbb");
    a.checkEqual("12", decodeRow(*can, 16,  1), "b..............W");
    a.checkEqual("13", decodeRow(*can, 16,  2), "b..............W");
    a.checkEqual("14", decodeRow(*can, 16,  3), "b.....~~~~~....W");
    a.checkEqual("15", decodeRow(*can, 16,  4), "b....~~~~~~~...W");
    a.checkEqual("16", decodeRow(*can, 16,  5), "b...~~~...~~~..W");
    a.checkEqual("17", decodeRow(*can, 16,  6), "b...~~.....~...W");
    a.checkEqual("18", decodeRow(*can, 16,  7), "b...~~.........W");
    a.checkEqual("19", decodeRow(*can, 16,  8), "b...~~.........W");
    a.checkEqual("20", decodeRow(*can, 16,  9), "b...~~.....~...W");
    a.checkEqual("21", decodeRow(*can, 16, 10), "b...~~~...~~~..W");
    a.checkEqual("22", decodeRow(*can, 16, 11), "b....~~~~~~~...W");
    a.checkEqual("23", decodeRow(*can, 16, 12), "b.....~~~~~....W");
    a.checkEqual("24", decodeRow(*can, 16, 13), "b..............W");
    a.checkEqual("25", decodeRow(*can, 16, 14), "b..............W");
    a.checkEqual("26", decodeRow(*can, 16, 15), "bWWWWWWWWWWWWWWW");
}
