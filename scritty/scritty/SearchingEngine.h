// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_SEARCHING_ENGINE_H
#define SCRITTY_SEARCHING_ENGINE_H

#include "Engine.h"
#include "GeneticTournament.h"
#include <Windows.h>

#define FIRST_PASS_SEARCH_DEPTH 3
#define MAX_SEARCH_DEPTH 4

namespace scritty
{
   class SearchingEngine : public GeneticEngine
   {
   public:
      SearchingEngine();
      ~SearchingEngine() { delete[] m_parameters; }

      SearchingEngine *Clone() const;

      virtual Outcome GetBestMove(std::string *best) const;

      virtual int Compare(GeneticEngine *first, GeneticEngine *second) const;

   private:
      SearchingEngine(const SearchingEngine &); // copy disallowed

      double GetBestMove(const Position &position, const Move *suggestion,
         size_t current_depth, double alpha, double beta, bool maximize,
         Move **best, Move *move_buffer) const;
      double EvaluatePosition(const Position &position) const; // centipawns

      mutable size_t m_nodes_searched;
      mutable ULONGLONG m_start_tick_count;
   };
}

#endif // #ifndef SCRITTY_SEARCHING_ENGINE_H
