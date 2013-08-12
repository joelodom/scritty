// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "SearchingEngine.h"
#include <iostream>

using namespace scritty;

SearchingEngine::SearchingEngine()
{
   m_parameters.push_back(std::pair<std::string, double>(
      PARAMETER_NAME_0, PARAMETER_DEFAULT_0));
   m_parameters.push_back(std::pair<std::string, double>(
      PARAMETER_NAME_1, PARAMETER_DEFAULT_1));
   m_parameters.push_back(std::pair<std::string, double>(
      PARAMETER_NAME_2, PARAMETER_DEFAULT_2));
   m_parameters.push_back(std::pair<std::string, double>(
      PARAMETER_NAME_3, PARAMETER_DEFAULT_3));
   m_parameters.push_back(std::pair<std::string, double>(
      PARAMETER_NAME_4, PARAMETER_DEFAULT_4));
}

SearchingEngine::~SearchingEngine()
{
}

Outcome SearchingEngine::GetBestMove(std::string *best) const
{
   // the move buffer for all depths is allocated once for performance
   Move *move_buffer = new Move[SEARCH_DEPTH*MAX_NUMBER_OF_LEGAL_MOVES];

   Move move;
   double evaluation = GetBestMove(
      m_position, SEARCH_DEPTH, &move, move_buffer);

   delete[] move_buffer;

   move.ToString(best);
   return OUTCOME_UNDECIDED; // don't ever give up
}

double SearchingEngine::GetBestMove(
   const Position &position, size_t plies, Move *best, Move *move_buffer) const
{
   // one ply is one half-turn in chess

   size_t num_moves = ListAllLegalMoves(position, move_buffer);

   if (num_moves == 0)
   {
      Outcome outcome = GetOutcome(position);

      if (outcome == OUTCOME_WIN_WHITE)
         return DBL_MAX;
      else if (outcome == OUTCOME_WIN_BLACK)
         return -DBL_MAX;
      else
         return 0.0;
   }

   Move *best_move = move_buffer;
   double best_evaluation = position.m_white_to_move ? -DBL_MAX : DBL_MAX;

   for (size_t i = 0; i < num_moves; ++i)
   {
      Position new_position = position;
      Engine::ApplyKnownLegalMoveToPosition(move_buffer[i], &new_position);

      // TODO: account for outcomes

      double new_position_evaluation = plies == 1
         ? new_position_evaluation = EvaluatePosition(new_position)
         : GetBestMove(new_position, plies - 1, nullptr,
         move_buffer + MAX_NUMBER_OF_LEGAL_MOVES);

      if (position.m_white_to_move ? new_position_evaluation > best_evaluation
         : new_position_evaluation < best_evaluation)
      {
         best_move = move_buffer + i;
         best_evaluation = new_position_evaluation;
      }
   }

   if (best != nullptr)
      *best = *best_move;

   return best_evaluation;
}

double SearchingEngine::EvaluatePosition(const Position &position) const
{
   double evaluation = 0.0;

   for (unsigned char file = 0; file <= 7; ++file)
   {
      for (unsigned char rank = 0; rank <= 7; ++rank)
      {
         switch (position.m_board.m_squares[file][rank])
         {
         case 'P':
            evaluation += m_parameters[0].second;
            break;
         case 'p':
            evaluation -= m_parameters[0].second;
            break;
         case 'B':
            evaluation += m_parameters[1].second;
            break;
         case 'b':
            evaluation -= m_parameters[1].second;
            break;
         case 'N':
            evaluation += m_parameters[2].second;
            break;
         case 'n':
            evaluation -= m_parameters[2].second;
            break;
         case 'R':
            evaluation += m_parameters[3].second;
            break;
         case 'r':
            evaluation -= m_parameters[3].second;
            break;
         case 'Q':
            evaluation += m_parameters[4].second;
            break;
         case 'q':
            evaluation -= m_parameters[4].second;
            break;
         }
      }
   }

   return evaluation;
}

/*virtual*/ int SearchingEngine::Compare(GeneticEngine *first,
   GeneticEngine *second) const
{
   // choose sides

   GeneticEngine *white, *black;

   if (rand() % 2 == 0)
   {
      white = first;
      black = second;
   }
   else
   {
      white = second;
      black = first;
   }

   // shake hands

   white->SetToStartPos();
   black->SetToStartPos();

   // play

   Outcome outcome;
   size_t moves = 1;

   for (;;)
   {
      std::cout << "Move " << moves << std::endl;

      // let white move
      std::string white_move;
      outcome = white->GetBestMove(&white_move);
      if (outcome != OUTCOME_UNDECIDED)
         break;

      // check for win, loose or draw
      Position position;
      white->GetPosition(&position);
      outcome = Engine::GetOutcome(position);
      if (outcome != OUTCOME_UNDECIDED)
         break;

      // let black move
      black->ApplyMove(white_move);
      std::string black_move;
      outcome = black->GetBestMove(&black_move);
      if (outcome != OUTCOME_UNDECIDED)
         break;

      // check for win, loose or draw
      black->GetPosition(&position);
      outcome = Engine::GetOutcome(position);
      if (outcome != OUTCOME_UNDECIDED)
         break;

      // possibly adjudicate a draw after too many moves
      if (moves++ == 200)
      {
         outcome = OUTCOME_DRAW;
         break;
      }

      white->ApplyMove(black_move);
   }

   // sign the score sheet

   if (outcome == OUTCOME_DRAW)
      return 0;
   if (outcome == OUTCOME_WIN_WHITE)
      return first == white ? 1 : 0;
   return second == white ? 1 : 0;
}
