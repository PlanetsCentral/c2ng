/**
  *  \file client/tiles/planetscreenheadertile.cpp
  */

#include "client/tiles/planetscreenheadertile.hpp"
#include "game/tables/temperaturename.hpp"
#include "game/game.hpp"
#include "game/root.hpp"
#include "afl/string/format.hpp"
#include "ui/res/resid.hpp"
#include "client/proxy/objectlistener.hpp"
#include "game/map/planet.hpp"

using ui::widgets::FrameGroup;

client::tiles::PlanetScreenHeaderTile::PlanetScreenHeaderTile(ui::Root& root, client::widgets::KeymapWidget& kmw)
    : ControlScreenHeader(root, kmw),
      m_receiver(root.engine().dispatcher(), *this)
{
    // ex WPlanetScreenHeaderTile::WPlanetScreenHeaderTile
    enableButton(btnAuto, FrameGroup::NoFrame);
    enableButton(btnAdd,  FrameGroup::NoFrame);
    // enableButton(btnSend, FrameGroup::NoFrame); // FIXME: missing in PCC2!

    // FIXME: alternative personalities
    //   Planet Task: CScr
    //   Planet:      Auto + Add + Send
}

void
client::tiles::PlanetScreenHeaderTile::attach(client::proxy::ObjectObserver& oop)
{
    class Job : public util::Request<ControlScreenHeader> {
     public:
        Job(game::Session& session, game::map::Object* obj)
            : m_name(obj != 0 ? obj->getName(game::PlainName, session.translator(), session.interface()) : String_t()),
              m_subtitle(),
              m_marked(obj != 0 && obj->isMarked())
            {
                game::map::Planet* p = dynamic_cast<game::map::Planet*>(obj);
                game::Game* g = session.getGame().get();
                game::Root* r = session.getRoot().get();
                afl::string::Translator& tx = session.translator();
                if (p != 0 && g != 0 && r != 0) {
                    int temp;
                    if (p->getTemperature().get(temp)) {
                        // ex WPlanetScreenHeaderTile::getSubtitle()
                        game::UnitScoreList::Index_t index = 0;
                        int16_t level = 0, turn = 0;
                        bool levelKnown = (g->planetScores().lookup(game::ScoreId_ExpLevel, index) && p->unitScores().get(index, level, turn));
                        String_t fmt = (levelKnown
                                        ? tx.translateString("(Id #%d, %s - %d" "\xC2\xB0" "\x46, %s)")
                                        : tx.translateString("(Id #%d, %s - %d" "\xC2\xB0" "\x46)"));
                        m_subtitle = afl::string::Format(fmt.c_str(),
                                                         p->getId(),
                                                         game::tables::TemperatureName(tx)(temp),
                                                         temp,
                                                         r->hostConfiguration().getExperienceLevelName(level, session.translator()));

                        // ex WPlanetScreenHeaderTile::getPictureId()
                        m_image = ui::res::makeResourceId(ui::res::PLANET, temp, p->getId());
                    } else {
                        // Fallback
                        m_subtitle = afl::string::Format(tx.translateString("(Id #%d)").c_str(), p->getId());
                        m_image = ui::res::PLANET;
                    }
                }
            }
        void handle(ControlScreenHeader& t)
            {
                t.setText(txtHeading, m_name);
                t.setText(txtSubtitle, m_subtitle);
                t.enableButton(btnImage, m_marked ? FrameGroup::YellowFrame : FrameGroup::NoFrame);
                t.setImage(m_image);
            }
     private:
        String_t m_name;
        String_t m_subtitle;
        String_t m_image;
        bool m_marked;
    };
    class Listener : public client::proxy::ObjectListener {
     public:
        Listener(util::RequestSender<ControlScreenHeader> reply)
            : m_reply(reply)
            { }
        virtual void handle(game::Session& s, game::map::Object* obj)
            { m_reply.postNewRequest(new Job(s, obj)); }
     private:
        util::RequestSender<ControlScreenHeader> m_reply;
    };

    oop.addNewListener(new Listener(m_receiver.getSender()));
}
