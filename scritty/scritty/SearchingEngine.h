// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_SEARCHING_ENGINE_H
#define SCRITTY_SEARCHING_ENGINE_H

#include "Engine.h"

#define SEARCH_DEPTH 4 // should always be even
#define DRAW_PIECE 'd' // kind of a hack

namespace scritty
{
   class SearchingEngine : public Engine
   {
   public:
      SearchingEngine();
      ~SearchingEngine();

      virtual Outcome GetBestMove(std::string *best) const;

   private:
      static double GetBestMove(
         const Position &position, size_t plies, Move *best, Move *move_buffer);
      static double EvaluatePosition(const Position &position); // centipawns
   };
}

#endif // #ifndef SCRITTY_SEARCHING_ENGINE_H
