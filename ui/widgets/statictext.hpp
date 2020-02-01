/**
  *  \file ui/widgets/statictext.hpp
  */
#ifndef C2NG_UI_WIDGETS_STATICTEXT_HPP
#define C2NG_UI_WIDGETS_STATICTEXT_HPP

#include "ui/simplewidget.hpp"
#include "util/skincolor.hpp"
#include "gfx/fontrequest.hpp"
#include "gfx/resourceprovider.hpp"
#include "afl/base/optional.hpp"

namespace ui { namespace widgets {

    class StaticText : public SimpleWidget {
     public:
        StaticText(const String_t& text, util::SkinColor::Color color, gfx::FontRequest font, gfx::ResourceProvider& provider, int align = 0);
        StaticText(const char* text, util::SkinColor::Color color, gfx::FontRequest font, gfx::ResourceProvider& provider, int align = 0);

        StaticText& setText(const String_t& text);
        StaticText& setIsFlexible(bool flex);
        StaticText& setColor(util::SkinColor::Color color);
        StaticText& setForcedWidth(int width);

        // SimpleWidget:
        virtual void draw(gfx::Canvas& can);
        virtual void handleStateChange(State st, bool enable);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);
        virtual bool handleMouse(gfx::Point pt, MouseButtons_t pressedButtons);

     private:
        String_t m_text;
        util::SkinColor::Color m_color;
        gfx::FontRequest m_font;
        gfx::ResourceProvider& m_provider;
        afl::base::Optional<int> m_forcedWidth;
        int m_align;
        bool m_isFlexible;
    };

} }

#endif
