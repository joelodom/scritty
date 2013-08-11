// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "SearchingEngine.h"

using namespace scritty;

SearchingEngine::SearchingEngine()
{
}

SearchingEngine::~SearchingEngine()
{
}

Outcome SearchingEngine::GetBestMove(std::string *best) const
{
   Move move;
   double evaluation;
   Outcome outcome = GetBestMove(m_position, 4, &move, &evaluation);
   if (outcome == OUTCOME_UNDECIDED)
      move.ToString(best);
   return outcome;
}

/*static*/ Outcome SearchingEngine::GetBestMove(const Position &position,
   size_t plies, Move *best, double *evaluation)
{
   // one ply is one half-turn in chess

   Move *move_buffer = new Move[MAX_NUMBER_OF_LEGAL_MOVES];
   size_t num_moves = ListAllLegalMoves(position, move_buffer);

   if (num_moves == 0)
   {
      delete[] move_buffer;
      return GetOutcome(position); // evaluation meaningless
   }

   Move *best_move = move_buffer;
   double best_evaluation = position.m_white_to_move ? -DBL_MAX : DBL_MAX;

   for (size_t i = 0; i < num_moves; ++i)
   {
      Position new_position = position;
      Engine::ApplyKnownLegalMoveToPosition(move_buffer[i], &new_position);

      // TODO: account for outcomes

      double new_position_evaluation;

      if (plies == 1)
         new_position_evaluation = EvaluatePosition(new_position);
      else
         GetBestMove(
         new_position, plies - 1, nullptr, &new_position_evaluation);

      if (position.m_white_to_move ? new_position_evaluation > best_evaluation
         : new_position_evaluation < best_evaluation)
      {
         best_move = move_buffer + i;
         best_evaluation = new_position_evaluation;
      }
   }

   if (best != nullptr)
      *best = *best_move;
   *evaluation = best_evaluation;

   delete[] move_buffer;

   return OUTCOME_UNDECIDED;
}

/*static*/ double SearchingEngine::EvaluatePosition(const Position &position)
{
   double evaluation = 0.0;

   for (unsigned char file = 0; file <= 7; ++file)
   {
      for (unsigned char rank = 0; rank <= 7; ++rank)
      {
         switch (position.m_board.m_squares[file][rank])
         {
         case 'P':
            evaluation += 1.0;
            break;
         case 'p':
            evaluation -= 1.0;
            break;
         case 'B':
         case 'N':
            evaluation += 3.0;
            break;
         case 'b':
         case 'n':
            evaluation -= 3.0;
            break;
         case 'R':
            evaluation += 5.0;
            break;
         case 'r':
            evaluation -= 5.0;
            break;
         case 'Q':
            evaluation += 9.0;
            break;
         case 'q':
            evaluation -= 9.0;
            break;
         }
      }
   }

   return evaluation;
}
