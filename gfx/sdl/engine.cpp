/**
  *  \file gfx/sdl/engine.cpp
  *  \brief Class gfx::sdl::Engine
  */

#include "config.h"
#ifdef HAVE_SDL
# include <cstdlib>
# include <SDL.h>
# ifdef HAVE_SDL_IMAGE
#  include <SDL_image.h>
# endif
# include "afl/except/fileformatexception.hpp"
# include "afl/string/format.hpp"
# include "afl/sys/mutexguard.hpp"
# include "afl/sys/time.hpp"
# include "gfx/eventconsumer.hpp"
# include "gfx/graphicsexception.hpp"
# include "gfx/sdl/engine.hpp"
# include "gfx/sdl/streaminterface.hpp"
# include "gfx/sdl/surface.hpp"
# include "gfx/windowparameters.hpp"

namespace {
    const char LOG_NAME[] = "gfx.sdl";

    const uint8_t SDL_WAKE_EVENT = SDL_USEREVENT;

    /** Convert SDL key modifier to internal key modifier.
        \param m SDL key modifier set
        \return util::Key key modifier set */
    util::Key_t convertModifier(uint32_t m)
    {
        // ex UIEvent::simplMod, sort-of
        uint32_t result = 0;
        if ((m & (KMOD_LSHIFT | KMOD_RSHIFT)) != 0) {
            result |= util::KeyMod_Shift;
        }
        if ((m & (KMOD_LCTRL | KMOD_RCTRL)) != 0) {
            result |= util::KeyMod_Ctrl;
        }
        if ((m & (KMOD_LALT | KMOD_RALT)) != 0) {
            result |= util::KeyMod_Alt;
        }
        if ((m & (KMOD_LMETA | KMOD_RMETA)) != 0) {
            result |= util::KeyMod_Meta;
        }
        return result;
    }


    /** Convert mouse buttons.
        Merges mouse and key modifiers.
        \param mouse SDL mouse button mask
        \param key modifier set
        \return mouse button set */
    gfx::EventConsumer::MouseButtons_t convertMouseButtons(uint32_t mouse, util::Key_t key)
    {
        // ex UIEvent::mergeMod

        // Convert mouse button presses
        gfx::EventConsumer::MouseButtons_t result;
        if ((mouse & SDL_BUTTON_LMASK) != 0) {
            result += gfx::EventConsumer::LeftButton;
        }
        if ((mouse & SDL_BUTTON_RMASK) != 0) {
            result += gfx::EventConsumer::RightButton;
        }
        if ((mouse & SDL_BUTTON_MMASK) != 0) {
            result += gfx::EventConsumer::MiddleButton;
        }

        if (!result.empty()) {
            // Convert key presses.
            // Do not add modifiers if this is a release, so an empty MouseButtons_t() still means mouse release even if a modifier is held.
            if ((key & util::KeyMod_Shift) != 0) {
                result += gfx::EventConsumer::ShiftKey;
            }
            if ((key & util::KeyMod_Ctrl) != 0) {
                result += gfx::EventConsumer::CtrlKey;
            }
            if ((key & util::KeyMod_Alt) != 0) {
                result += gfx::EventConsumer::AltKey;
            }
            if ((key & util::KeyMod_Meta) != 0) {
                result += gfx::EventConsumer::MetaKey;
            }
        }

        return result;
    }

    /** Convert key symbol.
        We translate the keypad keys into their "gray" equivalent. We need not care for the status of NumLock,
        when that is on, SDL will translate the keys into their ASCII / Unicode value for us which is handled elsewhere.
        \param sym SDL keysym
        \param mod SDL key modifiers
        \return key */
    util::Key_t convertKey(uint32_t sym, uint32_t mod)
    {
        // ex ui/event.cc:simplSym

        /* SDL-1.2.2 on Windows does not assign Unicode numbers to numpad keys, so we'll
           do it here instead. We do not want Shift to be a temporary Numlock replacement,
           though, to accept Shift-Arrows as Shift-Arrows, not digits. */
        bool shifted = (mod & KMOD_NUM) != 0;

        switch (sym) {
         case SDLK_KP_ENTER:  return util::Key_Return;
         case SDLK_KP0:       return shifted ? '0' : util::Key_Insert;
         case SDLK_KP1:       return shifted ? '1' : util::Key_End;
         case SDLK_KP2:       return shifted ? '2' : util::Key_Down;
         case SDLK_KP3:       return shifted ? '3' : util::Key_PgDn;
         case SDLK_KP4:       return shifted ? '4' : util::Key_Left;
         case SDLK_KP5:       return shifted ? '5' : util::Key_Num5;
         case SDLK_KP6:       return shifted ? '6' : util::Key_Right;
         case SDLK_KP7:       return shifted ? '7' : util::Key_Home;
         case SDLK_KP8:       return shifted ? '8' : util::Key_Up;
         case SDLK_KP9:       return shifted ? '9' : util::Key_PgUp;
         case SDLK_KP_PERIOD: return shifted ? '.' : util::Key_Delete;
         case SDLK_F1:        return util::Key_F1;
         case SDLK_F2:        return util::Key_F2;
         case SDLK_F3:        return util::Key_F3;
         case SDLK_F4:        return util::Key_F4;
         case SDLK_F5:        return util::Key_F5;
         case SDLK_F6:        return util::Key_F6;
         case SDLK_F7:        return util::Key_F7;
         case SDLK_F8:        return util::Key_F8;
         case SDLK_F9:        return util::Key_F9;
         case SDLK_F10:       return util::Key_F10;
         case SDLK_F11:       return util::Key_F11;
         case SDLK_F12:       return util::Key_F12;
         case SDLK_F13:       return util::Key_F13;
         case SDLK_F14:       return util::Key_F14;
         case SDLK_F15:       return util::Key_F15;
         case SDLK_UP:        return util::Key_Up;
         case SDLK_DOWN:      return util::Key_Down;
         case SDLK_LEFT:      return util::Key_Left;
         case SDLK_RIGHT:     return util::Key_Right;
         case SDLK_HOME:      return util::Key_Home;
         case SDLK_END:       return util::Key_End;
         case SDLK_PAGEUP:    return util::Key_PgUp;
         case SDLK_PAGEDOWN:  return util::Key_PgDn;
         case SDLK_TAB:       return util::Key_Tab;
         case SDLK_BACKSPACE: return util::Key_Backspace;
         case SDLK_DELETE:    return util::Key_Delete;
         case SDLK_INSERT:    return util::Key_Insert;
         case SDLK_RETURN:    return util::Key_Return;
         case SDLK_ESCAPE:    return util::Key_Escape;
         case SDLK_PRINT:     return util::Key_Print;
         case SDLK_PAUSE:     return util::Key_Pause;
         case SDLK_MENU:      return util::Key_Menu;
         default:             return sym >= ' ' && sym < 127 ? sym : 0;
        }
    }


    bool isKnownIgnorableKey(uint32_t sym)
    {
        return sym == SDLK_NUMLOCK
            || sym == SDLK_CAPSLOCK
            || sym == SDLK_SCROLLOCK
            || sym == SDLK_RSHIFT
            || sym == SDLK_LSHIFT
            || sym == SDLK_RCTRL
            || sym == SDLK_LCTRL
            || sym == SDLK_RALT
            || sym == SDLK_LALT
            || sym == SDLK_RMETA
            || sym == SDLK_LMETA
            || sym == SDLK_LSUPER
            || sym == SDLK_RSUPER
            || sym == SDLK_MODE
            || sym == SDLK_COMPOSE
            || sym == 0 /* SDL does not know to map it either */;
    }

    /** QUIT handler.
        When SDL itself sees a QUIT event, it will think we handle it, and close our window.
        This does not fit our model.
        We want a QUIT event to generate an event but not disrupt the normal event stream.
        Hence, we post the event manually and lie to SDL. */
    extern "C" int quitHandler(const SDL_Event* event)
    {
        if (event->type == SDL_QUIT) {
            /* SDL_PushEvent is SDL_PeepEvents in disguise, but doesn't modify *event */
            SDL_PushEvent(const_cast<SDL_Event*>(event));
            return 0;
        }
        return 1;
    }

}

// Constructor.
gfx::sdl::Engine::Engine(afl::sys::LogListener& log, afl::string::Translator& tx)
    : m_log(log),
      m_translator(tx),
      m_window(),
      m_disableGrab(false),
      m_grabEnabled(false),
      m_grabDelay(1000 / 10),
      m_grabEnableTime(0),
      m_lastClickTime(0),
      m_lastClickPosition(),
      m_buttonPressed(false),
      m_doubleClickDelay(1000 / 3),
      m_runnableSemaphore(0),
      m_lastWasRunnable(false),
      m_timerQueue(),
      m_taskMutex(),
      m_taskQueue()
{
    // ex gfx/init.cc:initGraphics, ui/event.cc:initEvents
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        throw GraphicsException(afl::string::Format(m_translator("Error initializing SDL: %s").c_str(), SDL_GetError()));
    }

    SDL_SetEventFilter(quitHandler);
    SDL_EnableUNICODE(1);
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);

    SDL_EventState(SDL_ENABLE, SDL_KEYDOWN);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEBUTTONUP);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEBUTTONDOWN);
    SDL_EventState(SDL_ENABLE, SDL_MOUSEMOTION);

    m_grabEnabled = false;

    std::atexit(SDL_Quit);
}

// Destructor.
gfx::sdl::Engine::~Engine()
{
    // ex ui/event.cc:doneEvents
    SDL_SetEventFilter(0);
    SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_TIMER);
}

// Create a window.
afl::base::Ref<gfx::Canvas>
gfx::sdl::Engine::createWindow(const WindowParameters& param)
{
    int sdlFlags = 0;
    if (param.fullScreen) {
        sdlFlags |= SDL_FULLSCREEN;
    }

    if (!param.title.empty()) {
        // SDL takes UTF-8 input since 1.2.10. How convenient.
        // On earlier SDLs, there is no direct way to support non-ASCII window titles.
        SDL_WM_SetCaption(param.title.c_str(), param.title.c_str());
    }

    if (param.icon.get() != 0) {
        // Convert the icon to something manageable.
        // It must be a SDL surface, but SDL can handle pretty much any format and converts that to OS limits.
        Point iconSize = param.icon->getSize();
        SDL_Surface* iconSurface = SDL_CreateRGBSurface(SDL_SWSURFACE, iconSize.getX(), iconSize.getY(), 32,
                                                        0x000000FF, 0x0000FF00, 0x00FF0000, 0xFF000000);
        if (iconSurface) {
            Surface iconCopy(iconSurface, true);
            iconCopy.blit(Point(0, 0), *param.icon, Rectangle(Point(0, 0), iconSize));
            iconCopy.ensureUnlocked();
            SDL_WM_SetIcon(iconSurface, 0);
        }
    }

    SDL_Surface* sfc = SDL_SetVideoMode(param.size.getX(), param.size.getY(), param.bitsPerPixel, sdlFlags);
    if (!sfc) {
        throw GraphicsException(afl::string::Format(m_translator("Error setting video mode: %s").c_str(), SDL_GetError()));
    }
    m_window = new Surface(sfc, false);
    m_disableGrab = param.disableGrab;

    // Log it
    char driverName[100];
    if (SDL_VideoDriverName(driverName, sizeof driverName)) {
        if (const SDL_VideoInfo* vi = SDL_GetVideoInfo()) {
            String_t flags;
            if (vi->wm_available) {  // do not translate the flag strings
                flags += ", wm";
            }
            if (vi->hw_available) {
                flags += ", hw";
            }
            if (vi->blit_hw) {
                flags += ", hw->hw";
            }
            if (vi->blit_sw) {
                flags += ", sw->hw";
            }
            if (vi->blit_fill) {
                flags += ", fill";
            }
            m_log.write(m_log.Info, LOG_NAME, afl::string::Format(m_translator("Video driver: %s (%dk%s)").c_str(), driverName, vi->video_mem, flags));
        }
    }

    return *m_window;
}

// Load an image file.
afl::base::Ref<gfx::Canvas>
gfx::sdl::Engine::loadImage(afl::io::Stream& file)
{
    StreamInterface iface(file);

    SDL_Surface* sfc;
#if HAVE_SDL_IMAGE
    sfc = IMG_Load_RW(&iface, 0);
#else
    sfc = SDL_LoadBMP_RW(&iface, 0);
#endif
    if (!sfc) {
        throw afl::except::FileFormatException(file, SDL_GetError());
    }
    return *new Surface(sfc, true);
}

// Wait for and handle an event.
void
gfx::sdl::Engine::handleEvent(EventConsumer& consumer, bool relativeMouseMovement)
{
    // ex ui/event.cc:getEvent, sort-of (no queue, no idle)

    // Flush output
    if (Surface* sfc = m_window.get()) {
        sfc->ensureUnlocked();
    }

    // Update mouse grab
    setMouseGrab(relativeMouseMovement);

    // Performance hack.
    // SDL runs at 100 Hz. This means that a task that posts Runnables in lock-step mode
    // (i.e. a new Runnable is posted after the previous one confirmed)
    // will get a throughput of at best 100 operations per second.
    // Building a scripted dialog will need several dozen actions, leading to noticeable delays.
    // If we know the last event was a Runnable, we therefore anticipate that the next one will be one, too.
    // Sleeping a little will give the other side time to prepare their event.
    // Posting a Runnable will post the semaphore, interrupting the wait.
    // This improves throughput good enough such that constructing a dialog such as
    // "CCUI.Ship.SetExtendedMission" no longer leads to a noticeable delay.
    if (m_lastWasRunnable) {
        m_runnableSemaphore.wait(5);
        m_lastWasRunnable = false;
    }

    // Wait for event to arrive
    SDL_Event ev;
    while (1) {
        afl::sys::Timeout_t t = m_timerQueue.getNextTimeout();
        if (t == afl::sys::INFINITE_TIMEOUT) {
            // Easy case: wait for event
            SDL_WaitEvent(&ev);
            if (convertEvent(ev, consumer, relativeMouseMovement)) {
                break;
            }
        } else {
            // Not-so-easy case: wait until we have an event.
            // SDL-1.2 has no SDL_WaitEventTimeout, so we do a delay loop.
            // This is the same thing that SDL does internally, i.e. SDL doesn't have internal super powers.
            uint32_t start = afl::sys::Time::getTickCounter();
            uint32_t elapsed = 0;
            bool eventStatus = false;
            while (!eventStatus && elapsed < t) {
                eventStatus = (SDL_PollEvent(&ev) == 1);
                if (!eventStatus) {
                    SDL_Delay(10);
                }
                elapsed = afl::sys::Time::getTickCounter() - start;
            }

            // Evaluate result
            bool timerResult = m_timerQueue.handleElapsedTime(elapsed);
            bool eventResult = eventStatus && convertEvent(ev, consumer, relativeMouseMovement);
            if (timerResult || eventResult) {
                break;
            }
        }
    }
}

// Get current keyboard modifiers (Shift, Alt, Ctrl, Meta).
util::Key_t
gfx::sdl::Engine::getKeyboardModifierState()
{
    return convertModifier(SDL_GetModState());
}

// Get request dispatcher.
util::RequestDispatcher&
gfx::sdl::Engine::dispatcher()
{
    return *this;
}

// Create a user-interface timer.
afl::base::Ref<gfx::Timer>
gfx::sdl::Engine::createTimer()
{
    return m_timerQueue.createTimer();
}

/*
 *  Privates
 */

/** Set mouse mode.
    \param enable true: grab mouse pointer and start reporting infinite movement; false: normal mouse behaviour */
void
gfx::sdl::Engine::setMouseGrab(bool enable)
{
    // ex ui/event.cc:setMouseMode
    if (!m_disableGrab && (enable != m_grabEnabled)) {
        m_grabEnabled = enable;
        if (enable) {
            SDL_ShowCursor(0);
            SDL_WM_GrabInput(SDL_GRAB_ON);
            m_grabEnableTime = afl::sys::Time::getTickCounter();
        } else {
            SDL_ShowCursor(1);
            SDL_WM_GrabInput(SDL_GRAB_OFF);
        }
    }
}

/** Convert and dispatch SDL event.
    \param se       SDL event
    \param consumer Event consumer to receive the event
    \param infinite true iff we are in "infinite movement" mode and want relative mouse movement to be reported.
    \retval true Event was dispatched
    \retval false Event not understood */
bool
gfx::sdl::Engine::convertEvent(const SDL_Event& se, gfx::EventConsumer& consumer, bool infinite)
{
    // ex ui/event.cc:convertEvent
    m_lastWasRunnable = false;
    switch(se.type) {
     case SDL_KEYDOWN: {
        uint32_t u = se.key.keysym.unicode;
        uint32_t sdlMod = se.key.keysym.mod;

        // Windows reports AltGr keys with RALT and LCTRL.
        // Thus, if it's printable, remove the Alt/Ctrl.
        if (u != 0 && (sdlMod & KMOD_RALT) != 0) {
            sdlMod &= ~KMOD_RALT;
            sdlMod &= ~KMOD_LCTRL;
        }
        util::Key_t key = convertModifier(sdlMod);
        if (u >= 32 && u != 127 && u < util::Key_FirstSpecial) {
            // It is a printable character. Use as-is.
            key |= u;

            // Discount shift on these keys unless they're from the number pad.
            // These are the only ones that can generate shifted printable.
            if (se.key.keysym.sym < SDLK_KP0 || se.key.keysym.sym > SDLK_KP_EQUALS) {
                key &= ~util::KeyMod_Shift;
            }
        } else {
            // It is a special key.
            key |= convertKey(se.key.keysym.sym, se.key.keysym.mod);
        }

        if ((key & util::Key_Mask) != 0) {
            consumer.handleKey(key, 0);
            return true;
        } else {
            if (!isKnownIgnorableKey(se.key.keysym.sym)) {
                m_log.write(m_log.Trace, LOG_NAME, afl::string::Format(m_translator("Key not mapped: 0x%x").c_str(), int(se.key.keysym.sym)));
            }
            return false;
        }
     }

     case SDL_MOUSEMOTION:
        /* In infinite mode, refuse movement events
           - when mouse grab is disabled (reported coordinates are wrong)
           - mouse grab has been active for too little time, i.e. the reported
           coordinates may be an unexpected jump */
        if (infinite) {
            if (m_disableGrab || (afl::sys::Time::getTickCounter() - m_grabEnableTime) < m_grabDelay) {
                return false;
            } else {
                Point pt(se.motion.xrel, se.motion.yrel);
                /* If there are more motion events, consume those as well, and merge
                   them with the current event. This avoids that events pile up and motion
                   "lags" when we cannot process events fast enough. This happens with the
                   starcharts at high resolution. */
                SDL_Event ae;
                while (SDL_PeepEvents(&ae, 1, SDL_PEEKEVENT, ~0U) > 0 && ae.type == SDL_MOUSEMOTION
                       && SDL_PeepEvents(&ae, 1, SDL_GETEVENT, ~0U) > 0 && ae.type == SDL_MOUSEMOTION)
                {
                    pt += Point(ae.motion.xrel, ae.motion.yrel);
                }
                consumer.handleMouse(pt, convertMouseButtons(se.motion.state, convertModifier(SDL_GetModState())));
                return true;
            }
        } else {
            Point pt(se.motion.x, se.motion.y);

            // If there are more motion events, consume those as well.
            SDL_Event ae;
            while (SDL_PeepEvents(&ae, 1, SDL_PEEKEVENT, ~0U) > 0 && ae.type == SDL_MOUSEMOTION
                   && SDL_PeepEvents(&ae, 1, SDL_GETEVENT, ~0U) > 0 && ae.type == SDL_MOUSEMOTION)
            {
                pt = Point(ae.motion.x, ae.motion.y);
            }
            consumer.handleMouse(pt, convertMouseButtons(se.motion.state, convertModifier(SDL_GetModState())));
            return true;
        }

     case SDL_MOUSEBUTTONDOWN:
            /* SDL 1.2.1 has no mouse wheel API, but the Windows backend
               reports a mouse wheel as buttons 4 and 5 (the fbcon backend
               detects an imaginary Z axis, but that doesn't yet fit into the
               SDL_MouseMotionEvent. */
        if (se.button.button == 4) {
            consumer.handleKey(util::Key_WheelUp | convertModifier(SDL_GetModState()), 0);
            return true;
        } else if (se.button.button == 5) {
            consumer.handleKey(util::Key_WheelDown | convertModifier(SDL_GetModState()), 0);
            return true;
        } else {
            /* Reportedly, touchpads generate very quick sequences of
               button-down and button-up, so we never see a button pressed
               in SDL_GetMouseState(). However, since this is a press, we
               know that the respective button must be down, so force it
               to be included. */
            return handleMouse(consumer, se.button, infinite, 1 << (se.button.button-1));
        }

     case SDL_MOUSEBUTTONUP:
        return handleMouse(consumer, se.button, infinite, 0);

     case SDL_WAKE_EVENT:
        m_lastWasRunnable = true;
        processTaskQueue();
        return true;

     case SDL_QUIT:
        consumer.handleKey(util::Key_Quit, 0);
        return true;

     default:
        return false;
    }
}

bool
gfx::sdl::Engine::handleMouse(EventConsumer& consumer, const SDL_MouseButtonEvent& be, bool infinite, uint32_t addButton)
{
    // No button events when we're in infinite mode but shouldn't be.
    if (infinite && m_disableGrab) {
        return false;
    } else {
        // Convert event
        Point pt = infinite ? Point() : Point(be.x, be.y);
        EventConsumer::MouseButtons_t btn = convertMouseButtons(SDL_GetMouseState(0, 0) | addButton, convertModifier(SDL_GetModState()));

        if (btn.empty() && m_buttonPressed) {
            // Mouse was released
            uint32_t time = afl::sys::Time::getTickCounter();
            if (m_lastClickTime > 0
                && time - m_lastClickTime < m_doubleClickDelay
                && std::abs(m_lastClickPosition.getX() - pt.getX()) < 5 && std::abs(m_lastClickPosition.getY() - pt.getY()) < 5)
            {
                btn += EventConsumer::DoubleClick;
                m_lastClickTime = 0;
            } else {
                m_lastClickTime = time;
            }
            m_lastClickPosition = pt;
        }
        m_buttonPressed = !(btn - EventConsumer::DoubleClick).empty();
        consumer.handleMouse(pt, btn);
        return true;
    }
}

void
gfx::sdl::Engine::postNewRunnable(afl::base::Runnable* p)
{
    // Make sure to post only non-null runnables.
    // Each Runnable will be accompanied by a single SDL_WAKE_EVENT.
    if (p != 0) {
        afl::sys::MutexGuard g(m_taskMutex);
        m_taskQueue.pushBackNew(p);

        SDL_Event event;
        event.type = SDL_WAKE_EVENT;
        event.user.code = 0;
        event.user.data1 = 0;
        event.user.data2 = 0;
        SDL_PushEvent(&event);
        m_runnableSemaphore.post();
    }
}

void
gfx::sdl::Engine::processTaskQueue()
{
    // Process precisely one Runnable.
    // Processing the Runnable may spawn a new event loop.
    // If we were using the "swap whole list and process everything" optimisation here,
    // events posted before the spawn would be held up.
    m_runnableSemaphore.wait(0);
    std::auto_ptr<afl::base::Runnable> t;
    {
        afl::sys::MutexGuard g(m_taskMutex);
        t.reset(m_taskQueue.extractFront());
    }

    if (t.get() != 0) {
        t->run();
    }
}

#else
int g_dummyToMakeGfxSdlEngineNotEmpty;
#endif
