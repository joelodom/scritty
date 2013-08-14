// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_SEARCHING_ENGINE_H
#define SCRITTY_SEARCHING_ENGINE_H

#include "Engine.h"
#include "GeneticTournament.h"

#define SEARCH_DEPTH 4 // should always be even

namespace scritty
{
   class SearchingEngine : public GeneticEngine
   {
   public:
      SearchingEngine();
      ~SearchingEngine();

      virtual Outcome GetBestMove(std::string *best) const;

   virtual int Compare(GeneticEngine *first, GeneticEngine *second) const;

   private:
      double GetBestMove(
         const Position &position, size_t plies, Move *best, Move *move_buffer)
         const;
      double EvaluatePosition(const Position &position) const; // centipawns
   };
}

#endif // #ifndef SCRITTY_SEARCHING_ENGINE_H
