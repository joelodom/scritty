// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_UCIPARSER_H
#define SCRITTY_UCIPARSER_H

#include <vector>
#include "Engine.h"

typedef std::vector<std::string> uci_tokens;

namespace scritty
{
   class UCIParser
   {
   public:
      static void BreakIntoTokens(
         const std::string &command, uci_tokens *tokens);
      static bool ParseMove(const std::string &s, Move *move);
   };
}

#endif // #ifndef SCRITTY_UCIPARSER_H
