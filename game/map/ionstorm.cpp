/**
  *  \file game/map/ionstorm.cpp
  *  \brief Class game::map::IonStorm
  *
  *  FIXME: as of 20200815, setters do not mark an IonStorm changed.
  *  The trivial solution, using Updater, does not work because IntegerProperty_t
  *  is not comparable.
  */

#include "game/map/ionstorm.hpp"
#include "afl/string/format.hpp"
#include "util/math.hpp"

game::map::IonStorm::IonStorm(int id)
    : CircularObject(),
      m_id(id),
      m_x(),
      m_y(),
      m_radius(),
      m_voltage(),
      m_speed(),
      m_heading(),
      m_isGrowing(false),
      m_name()
{ }

game::map::IonStorm::~IonStorm()
{ }

// Object:
String_t
game::map::IonStorm::getName(ObjectName which, afl::string::Translator& tx, InterpreterInterface& /*iface*/) const
{
    // ex GIonStorm::getName
    if (m_name.empty()) {
        return getDefaultName(tx);
    } else {
        switch (which) {
         case PlainName:
            return m_name;
         case LongName:
         case DetailedName:
            return afl::string::Format("%s: %s", getDefaultName(tx), m_name);
        }
        return String_t();
    }
}

game::Id_t
game::map::IonStorm::getId() const
{
    // ex GIonStorm::getId
    return m_id;
}

bool
game::map::IonStorm::getOwner(int& result) const
{
    // GIonStorm::getOwner
    result = 0;
    return true;
}

bool
game::map::IonStorm::getPosition(Point& result) const
{
    // ex GIonStorm::getPos
    int x, y;
    if (m_x.get(x) && m_y.get(y)) {
        result = Point(x, y);
        return true;
    } else {
        return false;
    }
}

// CircularObject:
bool
game::map::IonStorm::getRadius(int& result) const
{
    // ex GIonStorm::getRadius
    return m_radius.get(result);
}

bool
game::map::IonStorm::getRadiusSquared(int32_t& result) const
{
    // ex GIonStorm::getRadiusSquared
    int r;
    if (getRadius(r)) {
        result = util::squareInteger(r);
        return true;
    } else {
        return false;
    }
}

// IonStorm:
void
game::map::IonStorm::setName(String_t name)
{
    m_name = name;
}

void
game::map::IonStorm::setPosition(Point pos)
{
    m_x = pos.getX();
    m_y = pos.getY();
}

void
game::map::IonStorm::setRadius(int r)
{
    m_radius = r;
}

void
game::map::IonStorm::setVoltage(int voltage)
{
    m_voltage = voltage;
}

void
game::map::IonStorm::setSpeed(int speed)
{
    m_speed = speed;
}

void
game::map::IonStorm::setHeading(int heading)
{
    m_heading = heading;
}

void
game::map::IonStorm::setIsGrowing(bool flag)
{
    m_isGrowing = flag;
}

game::IntegerProperty_t
game::map::IonStorm::getClass() const
{
    // ex GIonStorm::getClass
    int volt;
    if (getVoltage().get(volt)) {
        if (volt > 200) {
            return 5;
        } else {
            return volt / 50 + 1;
        }
    } else {
        return afl::base::Nothing;
    }
}

game::IntegerProperty_t
game::map::IonStorm::getVoltage() const
{
    // ex GIonStorm::getVoltage
    return m_voltage;
}

game::IntegerProperty_t
game::map::IonStorm::getHeading() const
{
    // ex GIonStorm::getHeading
    return m_heading;
}

game::IntegerProperty_t
game::map::IonStorm::getSpeed() const
{
    // ex GIonStorm::getSpeed
    return m_speed;
}

bool
game::map::IonStorm::isGrowing() const
{
    // ex GIonStorm::isGrowing
    return m_isGrowing;
}

bool
game::map::IonStorm::isActive() const
{
    int volt;
    return getVoltage().get(volt)
        && volt > 0;
}

void
game::map::IonStorm::addMessageInformation(const game::parser::MessageInformation& info)
{
    // Allow voltage=0 to remove a storm. Otherwise, we need at minimum position, radius.
    namespace gp = game::parser;
    int32_t x, y, radius, voltage;
    if (info.getValue(gp::mi_IonVoltage, voltage)) {
        if (voltage == 0) {
            // Remove the storm
            setVoltage(voltage);
            setIsGrowing(false);
        } else {
            // Try to create the storm
            if (info.getValue(gp::mi_X, x) && info.getValue(gp::mi_Y, y) && info.getValue(gp::mi_Radius, radius)) {
                // Success
                setPosition(Point(x, y));
                setVoltage(voltage);
                setRadius(radius);

                // Try to set status. Either explicit from message, or implicit from voltage
                int status;
                if (info.getValue(gp::mi_IonStatus, status)) {
                    setIsGrowing(status != 0);
                } else {
                    setIsGrowing((voltage & 1) != 0);
                }

                // Try to set movement vector
                int speed, heading;
                if (info.getValue(gp::mi_Speed, speed) && info.getValue(gp::mi_Heading, heading)) {
                    setSpeed(speed);
                    setHeading(heading);
                }

                // Try to set the name
                info.getValue(gp::ms_Name, m_name);
            }
        }
    }
}

String_t
game::map::IonStorm::getDefaultName(afl::string::Translator& tx) const
{
    return afl::string::Format(tx.translateString("Ion storm #%d").c_str(), m_id);
}
