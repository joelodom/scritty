// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "UCIParser.h"
#include <sstream>
#include <iterator>

using namespace scritty;

/*static*/ void UCIParser::BreakIntoTokens(
   const std::string &command, uci_tokens *tokens)
{
   /* REQUIREMENT

   * arbitrary white space between tokens is allowed
   Example: "debug on\n" and  "   debug     on  \n"
   and "\t  debug \t  \t\ton\t  \n"
   all set the debug mode of the engine on.

   */

   std::stringstream ss(command);

   std::copy(std::istream_iterator<std::string>(ss), 
      std::istream_iterator<std::string>(),
      std::back_inserter(*tokens));
}
