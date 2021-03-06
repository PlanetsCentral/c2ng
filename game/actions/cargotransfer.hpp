/**
  *  \file game/actions/cargotransfer.hpp
  *  \brief Class game::actions::CargoTransfer
  */
#ifndef C2NG_GAME_ACTIONS_CARGOTRANSFER_HPP
#define C2NG_GAME_ACTIONS_CARGOTRANSFER_HPP

#include "afl/base/signal.hpp"
#include "afl/container/ptrvector.hpp"
#include "game/cargocontainer.hpp"
#include "game/cargospec.hpp"
#include "game/spec/shiplist.hpp"

namespace game { namespace actions {

    /** Cargo transfer.

        This class provides everything necessary to transfer cargo between objects.
        It holds (and controls life of) a number of CargoContainer objects.

        To use,
        - create a CargoTransfer
        - add participants using addNew()
        - use move() to move stuff around
        - commit() if the transaction is valid

        CargoTransfer will not itself allow to create an invalid transaction (exceeding unit limits).
        However, a cargo transfer can become invalid by the underlying objects changing.
        An additional validitity criterion is that all temporary containers must be empty at the end of a transaction. */
    class CargoTransfer {
     public:
        /** Default constructor.
            Makes an empty CargoTransfer. */
        CargoTransfer();

        /** Destructor.
            Destroying the CargoTransfer object without committing it cancels the transfer. */
        ~CargoTransfer();

        /** Add new participant.
            \param container New participant. Will become owned by CargoTransfer. Null is ignored. */
        void addNew(CargoContainer* container);

        /** Get participant by index.
            \param index Index [0,getNumContainers())
            \return container; null if index is out of range */
        CargoContainer* get(size_t index);

        /** Get number of participants.
            \return number of participants */
        size_t getNumContainers() const;

        /** Set overload permission.
            With overload enabled, units accept more than the rules allow.
            \param enable New value */
        void setOverload(bool enable);

        /** Check overload mode.
            \return true if overload enabled */
        bool isOverload() const;

        /** Move cargo.
            \param type    Element type to move
            \param amount  Amount to move (kt, clans, mc, units)
            \param from    Index of source unit
            \param to      Index of target unit
            \param partial If true, allow partial transfer. If false, only allow complete transfer.
            \param sellSupplies If enabled, convert supplies to mc
            \return Amount moved; 0 or amount if partial=false.

            Special behaviour:
            - if from or source are out of range or identical, the call is a no-op and returns 0.
            - if either participant does not support the requested type, the call is a no-op and returns 0.
            - if sellSupplies is true, and type is Element::Supplies, and a CargoContainer::SupplySale
              takes part in the transfer, this will sell supplies.
            - if amount is negative, the direction is reversed. */
        int32_t move(Element::Type type, int32_t amount, size_t from, size_t to, bool partial, bool sellSupplies);

        /** Move cargo specified by a CargoSpec.
            \param amount       [in/out] On input, cargo to move. On output, cargo not moved. If amount.isZero(), everything was moved.
            \param shipList     [in] Ship list (needed to determine number of torpedo types)
            \param from         [in] Index of source unit
            \param to           [in] Index of target unit
            \param sellSupplies [in] If enabled, convert supplies to mc */
        void move(CargoSpec& amount, const game::spec::ShipList& shipList, size_t from, size_t to, bool sellSupplies);

        /** Unload operation.
            This is a shortcut to transfer all industry resources (T/D/M/$/S/Clans), corresponding to the user interface "U" function.
            \param sellSupplies If enabled, convert supplies to mc
            \return true if function succeeded (see isUnloadAllowed()).
            Note that only reports whether the structural requirement was fulfilled, it does NOT report whether cargo was moved successfully. */
        bool unload(bool sellSupplies);

        /** Check whether unload is allowed.
            Unload requires
            - exactly one CargoContainer::UnloadTarget
            - at least one CargoContainer::UnloadSource
            \return true if unload() will succeed */
        bool isUnloadAllowed() const;

        /** Check whether supply sale is allowed.
            Supply sale requires at least one unit with CargoContainer::SupplySale present.
            \return true if condition fulfilled */
        bool isSupplySaleAllowed() const;

        /** Get permitted element types.
            An element type is permitted in the transfer if it is supported on ALL participants, and a nonzero amount is present.

            \param shipList     [in] Ship list (needed to determine number of torpedo types)

            \return set of all element types */
        ElementTypes_t getElementTypes(const game::spec::ShipList& shipList) const;

        /** Check validity of transaction.
            The transaction is valid if
            - all participants are valid
            - all CargoContainer::Temporary are empty
            \return true if condition fulfilled */
        bool isValid() const;

        /** Commit.
            \pre isValid()
            \throw Exception if condition not fulfilled */
        void commit();

        /** Change signal.
            Called whenever anything in any container changes. */
        afl::base::Signal<void()> sig_change;

     private:
        afl::container::PtrVector<CargoContainer> m_units;
        bool m_overload;
    };

} }

#endif
