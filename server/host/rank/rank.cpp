/**
  *  \file server/host/rank/rank.cpp
  *  \brief Utilities for Player Ranking
  */

#include "server/host/rank/rank.hpp"

// Initialize ranks.
void
server::host::rank::initRanks(Rank_t& ranks, int32_t value)
{
    // ex planetscentral/host/rank.h:initRanks
    for (int i = 0; i < Game::NUM_PLAYERS; ++i) {
        ranks[i] = value;
    }
}

// Compact ranks.
void
server::host::rank::compactRanks(Rank_t& dest, const Rank_t& rank, const Rank_t& score, game::PlayerSet_t players)
{
    // ex planetscentral/host/rank.h:compactRanks
    int place = 1;
    while (1) {
        /* Locate best-possible entry */
        game::PlayerSet_t thisSlots;   // players on this rank
        int32_t    thisRank = 0;       // current value from rank
        int32_t    thisScore = 0;      // current value from score
        for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
            if (players.contains(i)) {
                if (thisSlots.empty()
                    || rank[i-1] < thisRank
                    || (rank[i-1] == thisRank && score[i-1] > thisScore))
                {
                    thisSlots = game::PlayerSet_t(i);
                    thisRank  = rank[i-1];
                    thisScore = score[i-1];
                } else if (rank[i-1] == thisRank && score[i-1] == thisScore) {
                    thisSlots += i;
                }
            }
        }

        /* Termination condition */
        if (thisSlots.empty()) {
            break;
        }

        /* Mark it */
        players -= thisSlots;
        for (int i = 1; i <= Game::NUM_PLAYERS; ++i) {
            if (thisSlots.contains(i)) {
                dest[i-1] = place;
            }
        }
        ++place;
    }
}

