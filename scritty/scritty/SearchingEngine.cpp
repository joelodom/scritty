// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "SearchingEngine.h"

using namespace scritty;

SearchingEngine::SearchingEngine()
{
   m_parameters.push_back(PARAMETER_DEFAULT_0);
   m_parameter_names.push_back(PARAMETER_NAME_0);

   m_parameters.push_back(PARAMETER_DEFAULT_1);
   m_parameter_names.push_back(PARAMETER_NAME_1);

   m_parameters.push_back(PARAMETER_DEFAULT_2);
   m_parameter_names.push_back(PARAMETER_NAME_2);

   m_parameters.push_back(PARAMETER_DEFAULT_3);
   m_parameter_names.push_back(PARAMETER_NAME_3);

   m_parameters.push_back(PARAMETER_DEFAULT_4);
   m_parameter_names.push_back(PARAMETER_NAME_4);
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
            evaluation += m_parameters[0];
            break;
         case 'p':
            evaluation -= m_parameters[0];
            break;
         case 'B':
            evaluation += m_parameters[1];
            break;
         case 'b':
            evaluation -= m_parameters[1];
            break;
         case 'N':
            evaluation += m_parameters[2];
            break;
         case 'n':
            evaluation -= m_parameters[2];
            break;
         case 'R':
            evaluation += m_parameters[3];
            break;
         case 'r':
            evaluation -= m_parameters[3];
            break;
         case 'Q':
            evaluation += m_parameters[4];
            break;
         case 'q':
            evaluation -= m_parameters[4];
            break;
         }
      }
   }

   return evaluation;
}
