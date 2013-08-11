// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_SEARCHING_ENGINE_H
#define SCRITTY_SEARCHING_ENGINE_H

#include "Engine.h"

namespace scritty
{
   class SearchingEngine : public Engine
   {
   public:
      SearchingEngine();
      ~SearchingEngine();

      virtual Outcome GetBestMove(std::string *best) const;

   private:
      static Outcome GetBestMove(const Position &position,
         size_t plies, Move *best, double *evaluation);
      static double EvaluatePosition(const Position &position); // centipawns
   };
}

#endif // #ifndef SCRITTY_SEARCHING_ENGINE_H
