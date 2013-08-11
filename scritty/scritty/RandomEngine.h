// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_RANDOM_ENGINE_H
#define SCRITTY_RANDOM_ENGINE_H

#include "Engine.h"

namespace scritty
{
   class RandomEngine : public Engine
   {
   public:
      virtual Outcome GetBestMove(std::string *best) const;
   };
}

#endif // #ifndef SCRITTY_RANDOM_ENGINE_H
