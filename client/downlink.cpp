/**
  *  \file client/downlink.cpp
  *  \brief Class client::Downlink
  */

#include "client/downlink.hpp"
#include "util/translation.hpp"

client::Downlink::Downlink(ui::Root& root)
    : WaitIndicator(root.engine().dispatcher()),
      m_root(root),
      m_indicator(root, _("!Working")),
      m_busy(false),
      m_loop(root)
{ }

client::Downlink::~Downlink()
{ }

void
client::Downlink::post(bool success)
{
    m_loop.stop(success);
}

bool
client::Downlink::wait()
{
    setBusy(true);
    bool success = (m_loop.run() != 0);
    setBusy(false);
    return success;
}

void
client::Downlink::setBusy(bool flag)
{
    if (flag != m_busy) {
        m_busy = flag;
        if (m_busy) {
            m_indicator.setExtent(gfx::Rectangle(gfx::Point(), m_indicator.getLayoutInfo().getPreferredSize()));
            m_root.moveWidgetToEdge(m_indicator, 1, 2, 10);
            m_root.add(m_indicator);
        } else {
            m_root.remove(m_indicator);
            m_indicator.replayEvents();
        }
    }
}
