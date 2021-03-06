/**
  *  \file client/dialogs/buildqueuedialog.cpp
  *  \brief Build Queue Dialog
  */

#include "client/dialogs/buildqueuedialog.hpp"
#include "afl/string/format.hpp"
#include "client/downlink.hpp"
#include "game/proxy/buildqueueproxy.hpp"
#include "gfx/complex.hpp"
#include "ui/dialogs/messagebox.hpp"
#include "ui/eventloop.hpp"
#include "ui/layout/hbox.hpp"
#include "ui/layout/vbox.hpp"
#include "ui/spacer.hpp"
#include "ui/widgets/abstractlistbox.hpp"
#include "ui/widgets/framegroup.hpp"
#include "ui/widgets/standarddialogbuttons.hpp"
#include "ui/widgets/statictext.hpp"

namespace {
    using game::proxy::BuildQueueProxy;

    /*
     *  BuildQueueList - a list box displaying the build queue
     */

    const int ACTION_EMS = 25;
    const int FCODE_EMS = 5;
    const int QPOS_EMS = 5;
    const int POINTS_EMS = 10;
    const int GAP_PX = 5;
    const int PAD_PX = 2;

    class BuildQueueList : public ui::widgets::AbstractListbox {
     public:
        typedef BuildQueueProxy::Infos_t Infos_t;

        enum Column {
            QueuePositionColumn,
            BuildPointsColumn
        };
        typedef afl::bits::SmallSet<Column> Columns_t;


        BuildQueueList(ui::Root& root, afl::string::Translator& tx, Columns_t cols);

        void setContent(const Infos_t& data);
        void scrollToPlanet(game::Id_t planetId);

        virtual size_t getNumItems();
        virtual bool isItemAccessible(size_t n);
        virtual int getItemHeight(size_t n);
        virtual int getHeaderHeight();
        virtual void drawHeader(gfx::Canvas& can, gfx::Rectangle area);
        virtual void drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state);
        virtual void handlePositionChange(gfx::Rectangle& oldPosition);
        virtual ui::layout::Info getLayoutInfo() const;
        virtual bool handleKey(util::Key_t key, int prefix);

     private:
        int getItemHeight() const;

        ui::Root& m_root;
        afl::string::Translator& m_translator;
        Infos_t m_data;
        Columns_t m_columns;
    };

    class BuildQueueKeyHandler : public ui::InvisibleWidget {
     public:
        BuildQueueKeyHandler(BuildQueueProxy& proxy, BuildQueueList& list);
        bool handleKey(util::Key_t key, int prefix);

     private:
        BuildQueueProxy& m_proxy;
        BuildQueueList& m_list;
    };

    class BuildQueueDialog {
     public:
        typedef BuildQueueProxy::Infos_t Infos_t;

        BuildQueueDialog(ui::Root& root, afl::string::Translator& tx, BuildQueueProxy& proxy, BuildQueueList::Columns_t cols)
            : m_root(root),
              m_list(root, tx, cols),
              m_loop(root),
              m_translator(tx),
              m_proxy(proxy)
            {
                proxy.sig_update.add(this, &BuildQueueDialog::setContent);
            }

        void setContent(const Infos_t& data)
            { m_list.setContent(data); }

        void scrollToPlanet(game::Id_t planetId)
            { m_list.scrollToPlanet(planetId); }

        void run()
            {
                afl::base::Deleter del;
                ui::Window win(m_translator("Manage Build Queue"), m_root.provider(), m_root.colorScheme(), ui::BLUE_WINDOW, ui::layout::VBox::instance5);
                win.add(ui::widgets::FrameGroup::wrapWidget(del, m_root.colorScheme(), ui::LoweredFrame, m_list));

                ui::Widget& keys = del.addNew(new BuildQueueKeyHandler(m_proxy, m_list));
                win.add(keys);

                ui::Group& g = del.addNew(new ui::Group(ui::layout::HBox::instance5));
                ui::widgets::Button& btnFaster = del.addNew(new ui::widgets::Button("+", '+', m_root));
                btnFaster.dispatchKeyTo(keys);
                g.add(btnFaster);
                g.add(del.addNew(new ui::widgets::StaticText(m_translator("Build earlier"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));

                ui::widgets::Button& btnSlower = del.addNew(new ui::widgets::Button("-", '-', m_root));
                btnSlower.dispatchKeyTo(keys);
                g.add(btnSlower);
                g.add(del.addNew(new ui::widgets::StaticText(m_translator("Build later"), util::SkinColor::Static, gfx::FontRequest(), m_root.provider())));
                g.add(del.addNew(new ui::Spacer()));

                win.add(g);

                ui::widgets::StandardDialogButtons& btns = del.addNew(new ui::widgets::StandardDialogButtons(m_root));
                btns.addStop(m_loop);
                win.add(btns);


                win.pack();
                m_root.centerWidget(win);
                m_root.add(win);
                if (m_loop.run()) {
                    m_proxy.commit();
                }
            }

     private:
        ui::Root& m_root;
        BuildQueueList m_list;
        ui::EventLoop m_loop;
        afl::string::Translator& m_translator;
        BuildQueueProxy& m_proxy;
    };

}

/*
 *  BuildQueueList
 */

inline
BuildQueueList::BuildQueueList(ui::Root& root, afl::string::Translator& tx, Columns_t cols)
    : m_root(root),
      m_translator(tx),
      m_data(),
      m_columns(cols)
{ }

void
BuildQueueList::setContent(const Infos_t& data)
{
    // Remember current Id
    game::Id_t currentId = 0;
    size_t currentItem = getCurrentItem();
    if (currentItem < m_data.size()) {
        currentId = m_data[currentItem].planetId;
    }

    // Update
    m_data = data;
    requestRedraw();
    handleModelChange();

    // Select current Id
    if (currentId != 0) {
        scrollToPlanet(currentId);
    }
}

void
BuildQueueList::scrollToPlanet(game::Id_t planetId)
{
    for (size_t i = 0, n = m_data.size(); i < n; ++i) {
        if (m_data[i].planetId == planetId) {
            setCurrentItem(i);
            break;
        }
    }
}

size_t
BuildQueueList::getNumItems()
{
    return m_data.size();
}

bool
BuildQueueList::isItemAccessible(size_t /*n*/)
{
    return true;
}

int
BuildQueueList::getItemHeight(size_t /*n*/)
{
    return getItemHeight();
}

int
BuildQueueList::getHeaderHeight()
{
    return m_root.provider().getFont(gfx::FontRequest())->getLineHeight();
}

void
BuildQueueList::drawHeader(gfx::Canvas& can, gfx::Rectangle area)
{
    afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont(gfx::FontRequest());
    const int em = normalFont->getEmWidth();

    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    ctx.setColor(util::SkinColor::Static);
    ctx.useFont(*normalFont);

    drawHLine(ctx, area.getLeftX(), area.getBottomY()-1, area.getRightX()-1);

    area.consumeX(GAP_PX);
    outTextF(ctx, area.splitX(ACTION_EMS * em + GAP_PX), m_translator("Build Order"));
    outTextF(ctx, area.splitX(FCODE_EMS * em + GAP_PX), m_translator("FCode"));
    if (m_columns.contains(QueuePositionColumn)) {
        outTextF(ctx, area.splitX(QPOS_EMS * em + GAP_PX), m_translator("Q-Pos"));
    }
    if (m_columns.contains(BuildPointsColumn)) {
        outTextF(ctx, area, m_translator("Build Points"));
    }
}

void
BuildQueueList::drawItem(gfx::Canvas& can, gfx::Rectangle area, size_t item, ItemState state)
{
    // Prepare
    afl::base::Deleter del;
    gfx::Context<util::SkinColor::Color> ctx(can, getColorScheme());
    prepareColorListItem(ctx, area, state, m_root.colorScheme(), del);

    // Draw
    afl::base::Ref<gfx::Font> normalFont = m_root.provider().getFont(gfx::FontRequest());
    afl::base::Ref<gfx::Font> boldFont = m_root.provider().getFont(gfx::FontRequest().addWeight(1));
    afl::base::Ref<gfx::Font> smallFont = m_root.provider().getFont("-");
    if (item < m_data.size()) {
        const BuildQueueProxy::Info_t& e = m_data[item];
        const int em = normalFont->getEmWidth();

        area.consumeX(GAP_PX);
        area.consumeY(PAD_PX);

        // Name
        gfx::Rectangle nameArea = area.splitX(ACTION_EMS * em);
        ctx.setTextAlign(0, 0);
        ctx.useFont(*normalFont);
        ctx.setColor(util::SkinColor::Static);
        outTextF(ctx, nameArea.splitY(normalFont->getLineHeight()), e.actionName);
        ctx.useFont(*smallFont);
        ctx.setColor(util::SkinColor::Faded);
        outTextF(ctx, nameArea, afl::string::Format("(%s, #%d)", e.planetName, e.planetId));
        area.consumeX(GAP_PX);

        // Friendly code
        ctx.useFont(e.hasPriority ? *boldFont : *normalFont);
        ctx.setColor(e.conflict ? util::SkinColor::Red : util::SkinColor::Static);
        outTextF(ctx, area.splitX(FCODE_EMS * em), e.friendlyCode);
        area.consumeX(GAP_PX);

        // Queue position
        if (m_columns.contains(QueuePositionColumn)) {
            gfx::Rectangle queueArea = area.splitX(QPOS_EMS * em);
            if (e.queuePosition != 0) {
                ctx.useFont(*normalFont);
                ctx.setColor(util::SkinColor::Static);
                ctx.setTextAlign(2, 0);
                outTextF(ctx, queueArea, afl::string::Format("%d", e.queuePosition));
            }
            area.consumeX(GAP_PX);
        }

        // Points
        if (m_columns.contains(BuildPointsColumn)) {
            int space = POINTS_EMS * em;
            int half = space/2 - 3;
            gfx::Rectangle needArea = area.splitX(half);
            gfx::Rectangle haveArea = area.splitX(space - half);

            int32_t reqd, avail;
            if (e.pointsRequired.get(reqd)) {
                ctx.useFont(*normalFont);
                if (e.pointsAvailable.get(avail)) {
                    ctx.setColor(util::SkinColor::Static);
                    ctx.setTextAlign(0, 0);
                    outTextF(ctx, haveArea, afl::string::Format(" / %d", avail));

                    ctx.setColor(e.hasPriority ? reqd > avail ? util::SkinColor::Red : util::SkinColor::Static : util::SkinColor::Faded);
                } else {
                    ctx.setColor(util::SkinColor::Static);
                }
                ctx.setTextAlign(2, 0);
                outTextF(ctx, needArea, afl::string::Format("%d", reqd));
            }
        }
    }
}

void
BuildQueueList::handlePositionChange(gfx::Rectangle& oldPosition)
{
    defaultHandlePositionChange(oldPosition);
}

ui::layout::Info
BuildQueueList::getLayoutInfo() const
{
    int em = m_root.provider().getFont(gfx::FontRequest())->getEmWidth();
    int extraSize = 0;
    if (m_columns.contains(QueuePositionColumn)) {
        extraSize += em * QPOS_EMS + GAP_PX;
    }
    if (m_columns.contains(BuildPointsColumn)) {
        extraSize += em * POINTS_EMS + GAP_PX;
    }

    gfx::Point size(em * (ACTION_EMS + FCODE_EMS) + 3*GAP_PX + extraSize, getItemHeight() * 15);
    return ui::layout::Info(size, size, ui::layout::Info::GrowBoth);
}

bool
BuildQueueList::handleKey(util::Key_t key, int prefix)
{
    return defaultHandleKey(key, prefix);
}

int
BuildQueueList::getItemHeight() const
{
    return m_root.provider().getFont(gfx::FontRequest())->getLineHeight()
        + m_root.provider().getFont("-")->getLineHeight()
        + 2*PAD_PX;
}

/*
 *  BuildQueueKeyHandler - special keys for the dialog
 */

inline
BuildQueueKeyHandler::BuildQueueKeyHandler(BuildQueueProxy& proxy, BuildQueueList& list)
    : m_proxy(proxy),
      m_list(list)
{ }

bool
BuildQueueKeyHandler::handleKey(util::Key_t key, int prefix)
{
    if (key == '-' || key == util::Key_Down + util::KeyMod_Shift) {
        m_proxy.decreasePriority(m_list.getCurrentItem());
        return true;
    } else if (key == '+' || key == util::Key_Up + util::KeyMod_Shift) {
        m_proxy.increasePriority(m_list.getCurrentItem());
        return true;
    } else if (key >= '0' && key <= '9') {
        m_proxy.setPriority(m_list.getCurrentItem(), key - '0');
        return true;
    } else {
        return defaultHandleKey(key, prefix);
    }
}


/*
 *  Main entry point
 */

void
client::dialogs::doBuildQueueDialog(game::Id_t baseId,
                                    ui::Root& root,
                                    util::RequestSender<game::Session> gameSender,
                                    afl::string::Translator& tx)
{
    // Set up proxy
    BuildQueueProxy proxy(gameSender, root.engine().dispatcher());
    BuildQueueProxy::Infos_t infos;
    Downlink link(root);
    proxy.getStatus(link, infos);
    if (infos.empty()) {
        ui::dialogs::MessageBox(tx("You have no active ship build orders."),
                                tx("Manage Build Queue"),
                                root).doOkDialog();
        return;
    }

    // Column configuration
    BuildQueueList::Columns_t cols;
    for (size_t i = 0, n = infos.size(); i < n; ++i) {
        if (infos[i].queuePosition != 0) {
            cols += BuildQueueList::QueuePositionColumn;
        }
        if (infos[i].pointsRequired.isValid()) {
            cols += BuildQueueList::BuildPointsColumn;
        }
    }

    // Set up dialog
    BuildQueueDialog dlg(root, tx, proxy, cols);
    dlg.setContent(infos);
    dlg.scrollToPlanet(baseId);
    dlg.run();
}
