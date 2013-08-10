// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "Engine.h"

namespace scritty
{
   class RandomEngine : public Engine
   {
      virtual void GetBestMove(std::string *best) const; // algebraic
   };
}