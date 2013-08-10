// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "Engine.h"

namespace scritty
{
   class SearchingEngine : public Engine
   {
   public:
      SearchingEngine();
      ~SearchingEngine();

      virtual void GetBestMove(std::string *best) const; // algebraic

   private:
      static double EvaluatePosition(const Position &position); // centipawns
      Move *m_move_buffer;
   };
}
