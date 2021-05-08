/**
  *  \file ui/widgets/decimalselector.cpp
  *  \brief Class ui::widgets::DecimalSelector
  */

#include "ui/widgets/decimalselector.hpp"
#include "afl/string/format.hpp"
#include "gfx/complex.hpp"
#include "gfx/context.hpp"
#include "util/updater.hpp"

ui::widgets::DecimalSelector::DecimalSelector(Root& root, afl::string::Translator& tx, afl::base::Observable<int32_t>& value, int32_t min, int32_t max, int32_t step)
    : BaseDecimalSelector(value, min, max, step),
      m_root(root),
      m_translator(tx),
      m_flags()
{
    // ex UIDecimalSelector::UIDecimalSelector
}

ui::widgets::DecimalSelector::~DecimalSelector()
{ }

void
ui::widgets::DecimalSelector::setFlag(Flag flag, bool enable)
{
    m_flags.set(flag, enable);
}

void
ui::widgets::DecimalSelector::draw(gfx::Canvas& can)
{
    // ex UIDecimalSelector::drawContent
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    afl::base::Ref<gfx::Font> font = m_root.provider().getFont(gfx::FontRequest().addSize(1));
    ctx.useFont(*font);

    String_t post, val;

    if (getFocusState() != NoFocus) {
        post = "_";              // FIXME: replace by proper cursor
    }

    if (m_flags.contains(ShowMaximum)) {
        // FIXME: NumberFormatter
        post += afl::string::Format(m_translator(" (max. %d)"), /*numToString*/(getMax()));
    }

    if (getMode() != Zeroed) {
        if (Peer* p = getPeer()) {
            val = p->toString(*this, getValue());
        } else {
            // FIXME: NumberFormatter
            val = afl::string::Format("%d", getValue());
        }
    }

    int val_width  = font->getTextWidth(val);
    int post_width = font->getTextWidth(post);

    val_width = std::min(val_width, getExtent().getWidth());
    post_width = std::min(post_width, getExtent().getWidth() - val_width);

    int remain = getExtent().getWidth() - post_width - val_width;

    /* Left-justify:  | val_width | post_width | suffix |
       Right-justify: | prefix | val_width | post_width | */

    int pre_width, suf_width;
    if (m_flags.contains(RightJustified)) {
        pre_width = remain;
        suf_width = 0;
    } else {
        pre_width = 0;
        suf_width = remain;
    }

    // Draw it
    const int x = getExtent().getLeftX(), y = getExtent().getTopY(), h = getExtent().getHeight();
    if (getMode() == TypeErase) {
        // Focused and type-erase
        drawBackground(ctx, gfx::Rectangle(x, y, pre_width, h));
        drawBackground(ctx, gfx::Rectangle(x + pre_width + val_width, y, suf_width + post_width, h));
        drawSolidBar(ctx, gfx::Rectangle(x + pre_width, y, val_width, h), util::SkinColor::Input);
        ctx.setColor(util::SkinColor::Background);
    } else {
        // Not type-erase, thus everything on regular background
        drawBackground(ctx, getExtent());
        ctx.setColor(getFocusState() != NoFocus ? util::SkinColor::Input : util::SkinColor::Static);
    }

    outTextF(ctx, gfx::Point(x + pre_width, y), val_width, val);

    ctx.setColor(util::SkinColor::Static);
    outTextF(ctx, gfx::Point(x + pre_width + val_width, y), post_width, post);
}

ui::layout::Info
ui::widgets::DecimalSelector::getLayoutInfo() const
{
    // ex UIDecimalSelector::getLayoutInfo
    int ems = (m_flags.contains(ShowMaximum) ? 10 : 5);
    gfx::Point size = m_root.provider().getFont(gfx::FontRequest().addSize(1))->getCellSize().scaledBy(ems, 1);

    return ui::layout::Info(size, size, ui::layout::Info::GrowHorizontal);
}
