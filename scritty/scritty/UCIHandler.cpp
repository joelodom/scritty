// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "UCIHandler.h"
#include <iostream>
#include "scritty.h"
#include "Logger.h"

using namespace scritty;

bool UCIHandler::handle_uci(const uci_tokens &tokens)
{
   /* REQUIREMENT

   * uci
   tell engine to use the uci (universal chess interface),
   this will be sent once as a first command after program boot
   to tell the engine to switch to uci mode.
   After receiving the uci command the engine must identify itself with the "id"
   command and send the "option" commands to tell the GUI which engine settings
   the engine supports if any. After that the engine should send "uciok" to
   acknowledge the uci mode. If no uciok is sent within a certain time period,
   the engine task will be killed by the GUI.

   */

   if (tokens.size() > 0 && tokens[0] == "uci")
   {
      /* REQUIREMENT TODO: read and make sure I'm supporting all necessary

      * id
      * name <x>
      this must be sent after receiving the "uci" command to identify the
      engine, e.g. "id name Shredder X.Y\n"
      * author <x>
      this must be sent after receiving the "uci" command to identify the
      engine, e.g. "id author Stefan MK\n"

      */

      std::cout << "id name " << SCRITTY_NAME << std::endl;
      std::cout << "id author " << SCRITTY_AUTHOR << std::endl;

      /* REQUIREMENT

      * option
      This command tells the GUI which parameters can be changed in the engine.
      This should be sent once at engine startup after the "uci" and the "id"
      commands
      if any parameter can be changed in the engine.
      The GUI should parse this and build a dialog for the user to change the
      settings.
      Note that not every option needs to appear in this dialog as some options
      like
      "Ponder", "UCI_AnalyseMode", etc. are better handled elsewhere or are set
      automatically.
      If the user wants to change some settings, the GUI will send a "setoption"
      command to the engine.
      Note that the GUI need not send the setoption command when starting the
      engine for every option if
      it doesn't want to change the default value.
      For all allowed combinations see the examples below,
      as some combinations of this tokens don't make sense.
      One string will be sent for each parameter.
      * name <id>
      The option has the name id.
      Certain options have a fixed value for <id>, which means that the
      semantics of this option is fixed.
      Usually those options should not be displayed in the normal engine options
      window of the GUI but
      get a special treatment. "Pondering" for example should be set
      automatically when pondering is
      enabled or disabled in the GUI options. The same for "UCI_AnalyseMode"
      which should also be set
      automatically by the GUI. All those certain options have the prefix
      "UCI_" except for the
      first 6 options below. If the GUI gets an unknown Option with the prefix
      "UCI_", it should just
      ignore it and not display it in the engine's options dialog.
      * <id> = Hash, type is spin
      the value in MB for memory for hash tables can be changed,
      this should be answered with the first "setoptions" command at program
      boot
      if the engine has sent the appropriate "option name Hash" command,
      which should be supported by all engines!
      So the engine should use a very small hash first as default.
      * <id> = NalimovPath, type string
      this is the path on the hard disk to the Nalimov compressed format.
      Multiple directories can be concatenated with ";"
      * <id> = NalimovCache, type spin
      this is the size in MB for the cache for the nalimov table bases
      These last two options should also be present in the initial options
      exchange dialog
      when the engine is booted if the engine supports it
      * <id> = Ponder, type check
      this means that the engine is able to ponder.
      The GUI will send this whenever pondering is possible or not.
      Note: The engine should not start pondering on its own if this is enabled,
      this option is only
      needed because the engine might change its time management algorithm when
      pondering is allowed.
      * <id> = OwnBook, type check
      this means that the engine has its own book which is accessed by the
      engine itself.
      if this is set, the engine takes care of the opening book and the GUI will
      never
      execute a move out of its book for the engine. If this is set to false by
      the GUI,
      the engine should not access its own book.
      * <id> = MultiPV, type spin
      the engine supports multi best line or k-best mode. the default value is 1
      * <id> = UCI_ShowCurrLine, type check, should be false by default,
      the engine can show the current line it is calculating. see "info
      currline" above.
      * <id> = UCI_ShowRefutations, type check, should be false by default,
      the engine can show a move and its refutation in a line. see "info
      refutations" above.
      * <id> = UCI_LimitStrength, type check, should be false by default,
      The engine is able to limit its strength to a specific Elo number,
      This should always be implemented together with "UCI_Elo".
      * <id> = UCI_Elo, type spin
      The engine can limit its strength in Elo within this interval.
      If UCI_LimitStrength is set to false, this value should be ignored.
      If UCI_LimitStrength is set to true, the engine should play with this
      specific strength.
      This should always be implemented together with "UCI_LimitStrength".
      * <id> = UCI_AnalyseMode, type check
      The engine wants to behave differently when analysing or playing a game.
      For example when playing it can use some kind of learning.
      This is set to false if the engine is playing a game, otherwise it is true.
      * <id> = UCI_Opponent, type string
      With this command the GUI can send the name, title, elo and if the engine
      is playing a human
      or computer to the engine.
      The format of the string has to be [GM|IM|FM|WGM|WIM|none] [<elo>|none]
      [computer|human] <name>
      Examples:
      "setoption name UCI_Opponent value GM 2800 human Gary Kasparov"
      "setoption name UCI_Opponent value none none computer Shredder"
      * <id> = UCI_EngineAbout, type string
      With this command, the engine tells the GUI information about itself,
      for example a license text,
      usually it doesn't make sense that the GUI changes this text with the
      setoption command.
      Example:
      "option name UCI_EngineAbout type string default Shredder by Stefan
      Meyer-Kahlen, see www.shredderchess.com"
      * <id> = UCI_ShredderbasesPath, type string
      this is either the path to the folder on the hard disk containing the
      Shredder endgame databases or
      the path and filename of one Shredder endgame datbase.
      * <id> = UCI_SetPositionValue, type string
      the GUI can send this to the engine to tell the engine to use a certain
      value in centipawns from white's
      point of view if evaluating this specifix position. 
      The string can have the formats:
      <value> + <fen> | clear + <fen> | clearall

      * type <t>
      The option has type t.
      There are 5 different types of options the engine can send
      * check
      a checkbox that can either be true or false
      * spin
      a spin wheel that can be an integer in a certain range
      * combo
      a combo box that can have different predefined strings as a value
      * button
      a button that can be pressed to send a command to the engine
      * string
      a text field that has a string as a value,
      an empty string has the value "<empty>"
      * default <x>
      the default value of this parameter is x
      * min <x>
      the minimum value of this parameter is x
      * max <x>
      the maximum value of this parameter is x
      * var <x>
      a predefined value of this parameter is x
      Examples:
      Here are 5 strings for each of the 5 possible types of options
      "option name Nullmove type check default true\n"
      "option name Selectivity type spin default 2 min 0 max 4\n"
      "option name Style type combo default Normal var Solid var Normal var
      Risky\n"
      "option name NalimovPath type string default c:\\n"
      "option name Clear Hash type button\n"

      */

      // place options, etc. here

      /* REQUIREMENT

      * uciok
      Must be sent after the id and optional options to tell the GUI that the
      engine has sent all infos and is ready in uci mode.

      */

      std::cout << "uciok" << std::endl;

      return true;
   }

   return false;
}

bool UCIHandler::handle_isready(const uci_tokens &tokens)
{
   /* REQUIREMENT

   * isready
   this is used to synchronize the engine with the GUI. When the GUI has sent a
   command or multiple commands that can take some time to complete,
   this command can be used to wait for the engine to be ready again or
   to ping the engine to find out if it is still alive.
   E.g. this should be sent after setting the path to the tablebases as this can
   take some time.  This command is also required once before the engine is
   asked to do any search to wait for the engine to finish initializing.
   This command must always be answered with "readyok" and can be sent also when
   the engine is calculating in which case the engine should also immediately
   answer with "readyok" without stopping the search.

   */

   if (tokens.size() > 0 && tokens[0] == "isready")
   {
      /* REQUIREMENT

      * readyok
      This must be sent when the engine has received an "isready" command and
      has processed all input and is ready to accept new commands now.
      It is usually sent after a command that can take some time to be able to
      wait for the engine, but it can be used anytime, even when the engine is
      searching, and must always be answered with "isready".

      */

      std::cout << "readyok" << std::endl;

      return true;
   }

   return false;
}

bool UCIHandler::handle_quit(const uci_tokens &tokens)
{
   /* REQUIREMENT

   * quit
   quit the program as soon as possible

   */

   return tokens.size() > 0 && tokens[0] == "quit";
}

bool UCIHandler::handle_ucinewgame(const uci_tokens &tokens)
{
   /* REQUIREMENT

   * ucinewgame
   this is sent to the engine when the next search (started with "position"
   and "go") will be from
   a different game. This can be a new game the engine should play or a new game
   it should analyse but
   also the next position from a testsuite with positions only.
   If the GUI hasn't sent a "ucinewgame" before the first "position" command,
   the engine shouldn't
   expect any further ucinewgame commands as the GUI is probably not supporting
   the ucinewgame command.
   So the engine should not rely on this command even though all new GUIs should
   support it.
   As the engine's reaction to "ucinewgame" can take some time the GUI should
   always send "isready"
   after "ucinewgame" to wait for the engine to finish its operation.

   */

   if (tokens.size() < 1 || tokens[0] != "ucinewgame")
      return false;

   std::cout << "info string Scritty will defeat you!" << std::endl;

   m_engine.SetToStartPos();
   return true;
}

bool UCIHandler::handle_position(const uci_tokens &tokens)
{
   /* REQUIREMENT

   * Before the engine is asked to search on a position, there will always be a
   position command
   to tell the engine about the current position.

   * position [fen <fenstring> | startpos ]  moves <move1> .... <movei>
	   set up the position described in fenstring on the internal board and
	   play the moves on the internal chess board.
	   if the game was played  from the start position the string "startpos" will
      be sent
	   Note: no "new" command is needed. However, if this position is from a
      different game than
	   the last position sent to the engine, the GUI should have sent a
      "ucinewgame" inbetween.

   */

   if (tokens.size() < 2 || tokens[0] != "position")
      return false;

   if (tokens[1] == "startpos")
   {
      m_engine.SetToStartPos();
   }
   else if (tokens[1] == "fen")
   {
      // see http://en.wikipedia.org/wiki/Forsyth%E2%80%93Edwards_Notation
      Logger::LogMessage("TODO fen under construction");
      return false;
   }
   else
   {
      Logger::LogMessage("Bad position command.  Expected startpos or fen.");
      return false;
   }

   if (tokens.size() > 3)
   {
      if (tokens[2] != "moves")
      {
         Logger::LogMessage("Bad position command.  Expected moves.");
         return false;
      }

      /* REQUIREMENT

      Move format:
      ------------

      The move format is in long algebraic notation.
      A nullmove from the Engine to the GUI should be sent as 0000.
      Examples:  e2e4, e7e5, e1g1 (white short castling), e7e8q (for promotion)

      */

      for (auto it = tokens.begin() + 3; it != tokens.end(); ++it)
      {
         if (!m_engine.ApplyMove(*it))
         {
            Logger::LogMessage("ApplyMove failed.");
            return false;
         }
      }
   }

   return true;
}

bool UCIHandler::handle_go(const uci_tokens &tokens)
{
   /* REQUIREMENT

   * go
   start calculating on the current position set up with the "position" command.
   There are a number of commands that can follow this command, all will be sent
   in the same string.
   If one command is not sent its value should be interpreted as it would not
   influence the search.
   * searchmoves <move1> .... <movei>
   restrict search to this moves only
   Example: After "position startpos" and "go infinite searchmoves e2e4 d2d4"
   the engine should only search the two moves e2e4 and d2d4 in the initial
   position.
   * ponder
   start searching in pondering mode.
   Do not exit the search in ponder mode, even if it's mate!
   This means that the last move sent in in the position string is the ponder
   move.
   The engine can do what it wants to do, but after a "ponderhit" command
   it should execute the suggested move to ponder on. This means that the ponder
   move sent by
   the GUI can be interpreted as a recommendation about which move to ponder.
   However, if the
   engine decides to ponder on a different move, it should not display any
   mainlines as they are
   likely to be misinterpreted by the GUI because the GUI expects the engine to
   ponder
   on the suggested move.
   * wtime <x>
   white has x msec left on the clock
   * btime <x>
   black has x msec left on the clock
   * winc <x>
   white increment per move in mseconds if x > 0
   * binc <x>
   black increment per move in mseconds if x > 0
   * movestogo <x>
   there are x moves to the next time control,
   this will only be sent if x > 0,
   if you don't get this and get the wtime and btime it's sudden death
   * depth <x>
   search x plies only.
   * nodes <x>
   search x nodes only,
   * mate <x>
   search for a mate in x moves
   * movetime <x>
   search exactly x mseconds
   * infinite
   search until the "stop" command. Do not exit the search without being told so
   in this mode!

   */

   if (tokens.size() < 2 || tokens[0] != "go")
      return false;

   std::cout << "info string Scritty is thinking..." << std::endl;

   if (tokens[1] == "movetime")
   {
      /* REQUIREMENT

      * bestmove <move1> [ ponder <move2> ]
      the engine has stopped searching and found the move <move> best in this
      position.
      the engine can send the move it likes to ponder on. The engine must not
      start pondering automatically.
      this command must always be sent if the engine stops searching, also in
      pondering mode if there is a
      "stop" command, so for every "go" command a "bestmove" command is needed!
      Directly before that the engine should send a final "info" command with
      the final search information,
      the the GUI has the complete statistics about the last search.

      */

      std::string move = "e7e5";

      if (!m_engine.ApplyMove(move))
      {
         Logger::GetStream() << "Failed to apply own move: "
            << move << std::endl;
         return false;
      }

      std::cout << "bestmove " << move << std::endl;

      return true;
   }

   // TODO: time controls & other go options

   return false;
}
