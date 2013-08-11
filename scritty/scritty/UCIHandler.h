// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_UCIHANDLER_H
#define SCRITTY_UCIHANDLER_H

#include <string>
#include "Engine.h"
#include "UCIParser.h"

namespace scritty
{
   class UCIHandler
   {
   public:
      UCIHandler(Engine* engine) : m_engine(engine)
      {
      }

      bool handle_uci(const uci_tokens &tokens);
      bool handle_isready(const uci_tokens &tokens);
      bool handle_quit(const uci_tokens &tokens);
      bool handle_ucinewgame(const uci_tokens &tokens);
      bool handle_position(const uci_tokens &tokens);
      bool handle_go(const uci_tokens &tokens);

   private:
      Engine* m_engine;
   };
}

#endif // #ifndef SCRITTY_UCIHANDLER_H
