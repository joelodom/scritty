// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "UCIHandler.h"
#include "Logger.h"
#include <iostream>
#include "UCIParser.h"
#include "gtest/gtest.h"
#include "SearchingEngine.h"
#include "scritty.h"

using namespace scritty;

/* REQUIREMENT

   * The specification is independent of the operating system. For Windows,
   the engine is a normal exe file, either a console or "real" windows
   application.

*/

int main(int argc, const char *argv[])
{
   {
      srand((unsigned int)time(nullptr)); // TODO: switch to real PRNG

      Logger::LogMessage("Starting Scritty...");

      _CrtMemCheckpoint(&ScrittyTestEnvironment::s_mem_state);

      SearchingEngine engine;
      UCIHandler handler(&engine);

      /* REQUIREMENT

      * all communication is done via standard input and output with text
      commands,

      * The engine should boot and wait for input from the GUI,
      the engine should wait for the "isready" or "setoption" command to set up
      its internal parameters as the boot process
      should be as quick as possible.

      * all command strings the engine receives will end with '\n',
      also all commands the GUI receives should end with '\n',
      Note: '\n' can be 0x0d or 0x0a0d or any combination depending on your OS.
      If you use Engine and GUI in the same OS this should be no problem if you
      communicate in text mode, but be aware of this when for example
      running a Linux engine in a Windows GUI.

      */

      std::string line;
      while (std::getline(std::cin, line))
      {
         Logger::GetStream() << "Received line: " << line << std::endl;

         uci_tokens tokens;
         UCIParser::BreakIntoTokens(line, &tokens);

         if (handler.handle_quit(tokens))
            break;

         if (tokens[0] == "runtests")
         {
            if (ScrittyTestEnvironment::s_tests_were_run)
            {
               std::cout << "Google says you may only run tests once."
                  << std::endl;
            }
            else
            {
               ::testing::InitGoogleTest(&argc, (char**)argv);
               // gtest will delete new environment
               ::testing::AddGlobalTestEnvironment(new ScrittyTestEnvironment);
               int rv = RUN_ALL_TESTS();
               ScrittyTestEnvironment::s_tests_were_run = true;
               Logger::GetStream() << "Test results: " << rv << std::endl;
               std::cout << "Test results: " << rv << std::endl;
            }
         }
         else if (tokens[0] == "learn")
         {
            SearchingEngine engine;
            GeneticTournament<SearchingEngine> tournament(engine);
            tournament.Go(&engine);
            engine.PrintParameters();
         }

         // allow each command handler a chance to handle
         if (!
            (handler.handle_isready(tokens)
            || handler.handle_uci(tokens)
            || handler.handle_ucinewgame(tokens)
            || handler.handle_position(tokens)
            || handler.handle_go(tokens)
            ))
         {
            Logger::GetStream() << "Failed to process line: "
               << line << std::endl;
         }
      }

      Logger::GetStream() << "Exiting Scritty." << std::endl;
   }

   _CrtMemDumpAllObjectsSince(&ScrittyTestEnvironment::s_mem_state);
   return 0;
}

/* REQUIREMENTS TO IMPLEMENT:

* the engine must always be able to process input from stdin, even while thinking.

* The engine will always be in forced mode which means it should never start calculating
  or pondering without receiving a "go" command first.

* by default all the opening book handling is done by the GUI,
  but there is an option for the engine to use its own book ("OwnBook" option, see below)

* if the engine or the GUI receives an unknown command or token it should just ignore it and try to
  parse the rest of the string in this line.
  Examples: "joho debug on\n" should switch the debug mode on given that joho is not defined,
            "debug joho on\n" will be undefined however.

* if the engine receives a command which is not supposed to come, for example "stop" when the engine is
  not calculating, it should also just ignore it.

GUI to engine:
--------------

These are all the command the engine gets from the interface.

* debug [ on | off ]
	switch the debug mode of the engine on and off.
	In debug mode the engine should send additional infos to the GUI, e.g. with the "info string" command,
	to help debugging, e.g. the commands that the engine has received etc.
	This mode should be switched off by default and this command can be sent
	any time, also when the engine is thinking.

* setoption name <id> [value <x>]
	this is sent to the engine when the user wants to change the internal parameters
	of the engine. For the "button" type no value is needed.
	One string will be sent for each parameter and this will only be sent when the engine is waiting.
	The name and value of the option in <id> should not be case sensitive and can inlude spaces.
	The substrings "value" and "name" should be avoided in <id> and <x> to allow unambiguous parsing,
	for example do not use <name> = "draw value".
	Here are some strings for the example below:
	   "setoption name Nullmove value true\n"
      "setoption name Selectivity value 3\n"
	   "setoption name Style value Risky\n"
	   "setoption name Clear Hash\n"
	   "setoption name NalimovPath value c:\chess\tb\4;c:\chess\tb\5\n"

* register
	this is the command to try to register an engine or to tell the engine that registration
	will be done later. This command should always be sent if the engine	has sent "registration error"
	at program startup.
	The following tokens are allowed:
	* later
	   the user doesn't want to register the engine now.
	* name <x>
	   the engine should be registered with the name <x>
	* code <y>
	   the engine should be registered with the code <y>
	Example:
	   "register later"
	   "register name Stefan MK code 4359874324"

* stop
	stop calculating as soon as possible,
	don't forget the "bestmove" and possibly the "ponder" token when finishing the search

* ponderhit
	the user has played the expected move. This will be sent if the engine was told to ponder on the same move
	the user has played. The engine should continue searching but switch from pondering to normal search.

Engine to GUI:
--------------

* copyprotection
	this is needed for copyprotected engines. After the uciok command the engine can tell the GUI,
	that it will check the copy protection now. This is done by "copyprotection checking".
	If the check is ok the engine should send "copyprotection ok", otherwise "copyprotection error".
	If there is an error the engine should not function properly but should not quit alone.
	If the engine reports "copyprotection error" the GUI should not use this engine
	and display an error message instead!
	The code in the engine can look like this
      TellGUI("copyprotection checking\n");
	   // ... check the copy protection here ...
	   if(ok)
	      TellGUI("copyprotection ok\n");
      else
         TellGUI("copyprotection error\n");

* registration
	this is needed for engines that need a username and/or a code to function with all features.
	Analog to the "copyprotection" command the engine can send "registration checking"
	after the uciok command followed by either "registration ok" or "registration error".
	Also after every attempt to register the engine it should answer with "registration checking"
	and then either "registration ok" or "registration error".
	In contrast to the "copyprotection" command, the GUI can use the engine after the engine has
	reported an error, but should inform the user that the engine is not properly registered
	and might not use all its features.
	In addition the GUI should offer to open a dialog to
	enable registration of the engine. To try to register an engine the GUI can send
	the "register" command.
	The GUI has to always answer with the "register" command	if the engine sends "registration error"
	at engine startup (this can also be done with "register later")
	and tell the user somehow that the engine is not registered.
	This way the engine knows that the GUI can deal with the registration procedure and the user
	will be informed that the engine is not properly registered.

* info
	the engine wants to send information to the GUI. This should be done whenever
   one of the info has changed.
	The engine can send only selected infos or multiple infos with one info
   command,
	e.g. "info currmove e2e4 currmovenumber 1" or
	     "info depth 12 nodes 123456 nps 100000".
	Also all infos belonging to the pv should be sent together
	e.g. "info depth 2 score cp 214 time 1242 nodes 2124 nps 34928 pv e2e4
   e7e5 g1f3"
	I suggest to start sending "currmove", "currmovenumber", "currline" and
   "refutation" only after one second
	to avoid too much traffic.
	Additional info:
	* depth <x>
		search depth in plies
	* seldepth <x>
		selective search depth in plies,
		if the engine sends seldepth there must also be a "depth" present in the
      same string.
	* time <x>
		the time searched in ms, this should be sent together with the pv.
	* nodes <x>
		x nodes searched, the engine should send this info regularly
	* pv <move1> ... <movei>
		the best line found
	* multipv <num>
		this for the multi pv mode.
		for the best move/pv add "multipv 1" in the string when you send the pv.
		in k-best mode always send all k variants in k strings together.
	* score
		* cp <x>
			the score from the engine's point of view in centipawns.
		* mate <y>
			mate in y moves, not plies.
			If the engine is getting mated use negative values for y.
		* lowerbound
	      the score is just a lower bound.
		* upperbound
		   the score is just an upper bound.
	* currmove <move>
		currently searching this move
	* currmovenumber <x>
		currently searching move number x, for the first move x should be 1 not 0.
	* hashfull <x>
		the hash is x permill full, the engine should send this info regularly
	* nps <x>
		x nodes per second searched, the engine should send this info regularly
	* tbhits <x>
		x positions where found in the endgame table bases
	* sbhits <x>
		x positions where found in the shredder endgame databases
	* cpuload <x>
		the cpu usage of the engine is x permill.
	* string <str>
		any string str which will be displayed be the engine,
		if there is a string command the rest of the line will be interpreted as
      <str>.
	* refutation <move1> <move2> ... <movei>
	   move <move1> is refuted by the line <move2> ... <movei>, i can be any
      number >= 1.
	   Example: after move d1h5 is searched, the engine can send
	   "info refutation d1h5 g6h5"
	   if g6h5 is the best answer after d1h5 or if g6h5 refutes the move d1h5.
	   if there is no refutation for d1h5 found, the engine should just send
	   "info refutation d1h5"
		The engine should only send this if the option "UCI_ShowRefutations" is
      set to true.
	* currline <cpunr> <move1> ... <movei>
	   this is the current line the engine is calculating. <cpunr> is the number
      of the cpu if
	   the engine is running on more than one cpu. <cpunr> = 1,2,3....
	   if the engine is just using one cpu, <cpunr> can be omitted.
	   If <cpunr> is greater than 1, always send all k lines in k strings
      together.
		The engine should only send this if the option "UCI_ShowCurrLine" is set
      to true.

Examples:
---------

This is how the communication when the engine boots can look like:

GUI     engine

// tell the engine to switch to UCI mode
uci

// engine identify
      id name Shredder
		id author Stefan MK

// engine sends the options it can change
// the engine can change the hash size from 1 to 128 MB
		option name Hash type spin default 1 min 1 max 128

// the engine supports Nalimov endgame tablebases
		option name NalimovPath type string default <empty>
		option name NalimovCache type spin default 1 min 1 max 32

// the engine can switch off Nullmove and set the playing style
	   option name Nullmove type check default true
  		option name Style type combo default Normal var Solid var Normal var Risky

// the engine has sent all parameters and is ready
		uciok

// Note: here the GUI can already send a "quit" command if it just wants to find out
//       details about the engine, so the engine should not initialize its internal
//       parameters before here.
// now the GUI sets some values in the engine
// set hash to 32 MB
setoption name Hash value 32

// init tbs
setoption name NalimovCache value 1
setoption name NalimovPath value d:\tb;c\tb

// waiting for the engine to finish initializing
// this command and the answer is required here!
isready

// engine has finished setting up the internal values
		readyok

// now we are ready to go

// if the GUI is supporting it, tell the engine that is is
// searching on a game that it hasn't searched on before
ucinewgame

// if the engine supports the "UCI_AnalyseMode" option and the next search is supposed to
// be an analysis, the GUI should set "UCI_AnalyseMode" to true if it is currently
// set to false with this engine
setoption name UCI_AnalyseMode value true

// tell the engine to search infinite from the start position after 1.e4 e5
position startpos moves e2e4 e7e5
go infinite

// the engine starts sending infos about the search to the GUI
// (only some examples are given)


		info depth 1 seldepth 0
		info score cp 13  depth 1 nodes 13 time 15 pv f1b5
		info depth 2 seldepth 2
		info nps 15937
		info score cp 14  depth 2 nodes 255 time 15 pv f1c4 f8c5
		info depth 2 seldepth 7 nodes 255
		info depth 3 seldepth 7
		info nps 26437
		info score cp 20  depth 3 nodes 423 time 15 pv f1c4 g8f6 b1c3
		info nps 41562
		....


// here the user has seen enough and asks to stop the searching
stop

// the engine has finished searching and is sending the bestmove command
// which is needed for every "go" command sent to tell the GUI
// that the engine is ready again
		bestmove g1f3 ponder d8f6



Chess960
========

UCI could easily be extended to support Chess960 (also known as Fischer Random Chess).

The engine has to tell the GUI that it is capable of playing Chess960 and the GUI has to tell
the engine that is should play according to the Chess960 rules.
This is done by the special engine option UCI_Chess960. If the engine knows about Chess960
it should send the command 'option name UCI_Chess960 type check default false'
to the GUI at program startup.
Whenever a Chess960 game is played, the GUI should set this engine option to 'true'.

Castling is different in Chess960 and the white king move when castling short is not always e1g1.
A king move could both be the castling king move or just a normal king move.
This is why castling moves are sent in the form king "takes" his own rook.
Example: e1h1 for the white short castle move in the normal chess start position.

In EPD and FEN position strings specifying the castle rights with w and q is not enough as
there could be more than one rook on the right or left side of the king.
This is why the castle rights are specified with the letter of the castle rook's line.
Upper case letters for white's and lower case letters for black's castling rights.
Example: The normal chess position would be:
rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w AHah -

*/

/* PARAMETERS TO MAKE SEARCHABLE VIA GENETIC ALGORITHM:

 - Piece values based on square and game phase
 - Square values (ability to attack square) based on square and game phase
 - Game phase changes based on count of captured pieces (probably cuts at 3, 10)
 - Value of pins, maybe based on positional factors
 - Value of a check (as it may lead to mate)

*/
