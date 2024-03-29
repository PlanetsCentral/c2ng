/**
  *  \file ui/widgets/button.cpp
  *  \brief Class ui::widgets::Button
  */

#include "ui/widgets/button.hpp"

ui::widgets::Button::Button(String_t text, util::Key_t key, ui::Root& root)
    : BaseButton(root, key),
      m_icon(text, gfx::FontRequest().addSize(1), root)
{
    setIcon(m_icon);
}

ui::widgets::Button::Button(const util::KeyString& ks, ui::Root& root)
    : BaseButton(root, ks.getKey()),
      m_icon(ks.getString(), gfx::FontRequest().addSize(1), root)
{
    setIcon(m_icon);
}

ui::widgets::Button::~Button()
{ }

void
ui::widgets::Button::setFont(gfx::FontRequest font)
{
    m_icon.setFont(font);
}

void
ui::widgets::Button::setTextAlign(gfx::HorizontalAlignment x, gfx::VerticalAlignment y)
{
    m_icon.setTextAlign(x, y);
    requestRedraw();
}

void
ui::widgets::Button::setText(const String_t& text)
{
    m_icon.setText(text);
    requestRedraw();
}

void
ui::widgets::Button::setCompact(bool flag)
{
    m_icon.setCompact(flag);
    requestRedraw();
}
