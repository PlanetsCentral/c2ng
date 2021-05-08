/**
  *  \file gfx/engine.hpp
  *  \brief Base class gfx::Engine
  */
#ifndef C2NG_GFX_ENGINE_HPP
#define C2NG_GFX_ENGINE_HPP

#include "afl/base/refcounted.hpp"
#include "afl/base/ptr.hpp"
#include "afl/io/stream.hpp"
#include "util/requestdispatcher.hpp"
#include "gfx/timer.hpp"
#include "util/key.hpp"

namespace gfx {

    class Canvas;
    class EventConsumer;
    struct WindowParameters;

    /** Base class for a graphics engine.
        This encapsulates all logic for talking to a GUI library.
        A GUI library consists of three parts:
        - a graphics output window; a single, user-visible canvas
        - a graphics file loader; produces invisible canvases, probably optimized for blitting onto the window
        - user input (event) acquisition

        In order to meaningfully work with the event loop, we also implement
        - a util::RequestDispatcher to execute tasks
        - timers */
    class Engine : public afl::base::RefCounted {
     public:
        virtual ~Engine()
            { }

        /** Create a window.

            Note that only one window can be active.
            Calling createWindow a second time will invalidate previously-created windows.

            <b>Multithreading:</b> Call this method from the GUI thread only.

            \param param Window parameters (size, color depth, title, ...)
            \return Canvas for that window; never null
            \throw GraphicsException on error */
        virtual afl::base::Ref<Canvas> createWindow(const WindowParameters& param) = 0;

        /** Load an image file.

            Uses the GUI library's file loading mechanisms to load an image file.
            The returned canvas is in a format optimized for blitting.

            <b>Multithreading:</b> Call this method from any thread (preferrably not the GUI thread to not block it).

            \param file Opened file
            \return Canvas containing the loaded image; never null
            \throw GraphicsException on error */
        virtual afl::base::Ref<Canvas> loadImage(afl::io::Stream& file) = 0;

        /** Wait for and handle an event.
            Returns when a user event, dispatcher request, or timer has been processed.
            (Note that this may process multiple events/requests in one go.)

            <b>Multithreading:</b> Call this method from the GUI thread only.

            \param consumer Event consumer
            \param relativeMouseMovement Receive relative mouse movement if enabled (default: absolute) */
        virtual void handleEvent(EventConsumer& consumer, bool relativeMouseMovement) = 0;

        /** Get current keyboard modifiers (Shift, Alt, Ctrl, Meta).
            This function should be used sparingly;
            most use-cases can be handled by observing the flags reported with key and mouse events.

            <b>Multithreading:</b> Call this method from the GUI thread only.

            \return Modifiers, as combination of KeyMod_Xxx constants */
        virtual util::Key_t getKeyboardModifierState() = 0;

        /** Get request dispatcher.
            Actions posted into the request dispatcher will be executed in the GUI thread,
            and cause handleEvent() to return.

            <b>Multithreading:</b> Call this method from any thread.

            \return Event dispatcher */
        virtual util::RequestDispatcher& dispatcher() = 0;

        /** Create a user-interface timer.
            The timer callback will execute from within handleEvent() and cause it to return.

            <b>Multithreading:</b> Call this method from the GUI thread only.

            \return newly-allocated timer; never null */
        virtual afl::base::Ref<Timer> createTimer() = 0;
    };

}

#endif
