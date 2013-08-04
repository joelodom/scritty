// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_UCIHANDLER_H
#define SCRITTY_UCIHANDLER_H

#include <string>
#include "UCIParser.h"

namespace scritty
{
   class UCIHandler
   {
   public:
      bool handle_uci(const uci_tokens &tokens);
      bool handle_isready(const uci_tokens &tokens);
      bool handle_quit(const uci_tokens &tokens);
   };
}

#endif // #ifndef SCRITTY_UCIHANDLER_H
