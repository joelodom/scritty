// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_UCIHANDLER_H
#define SCRITTY_UCIHANDLER_H

#include <string>
#include "UCIParser.h"
#include "Engine.h"
#include "gtest/gtest.h"

namespace scritty
{
   class UCIHandler
   {
      FRIEND_TEST(integration_tests, shall_we_play_a_game);

   public:
      bool handle_uci(const uci_tokens &tokens);
      bool handle_isready(const uci_tokens &tokens);
      bool handle_quit(const uci_tokens &tokens);
      bool handle_ucinewgame(const uci_tokens &tokens);
      bool handle_position(const uci_tokens &tokens);
      bool handle_go(const uci_tokens &tokens);

   private:
      Engine m_engine;
   };
}

#endif // #ifndef SCRITTY_UCIHANDLER_H
