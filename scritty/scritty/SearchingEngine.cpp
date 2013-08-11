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
   // the move buffer for all depths is allocated once for performance
   Move *move_buffer = new Move[SEARCH_DEPTH*MAX_NUMBER_OF_LEGAL_MOVES];

   Move move;
   double evaluation = GetBestMove(
      m_position, SEARCH_DEPTH, &move, move_buffer);

   delete[] move_buffer;

   if (move.promotion_piece == DRAW_PIECE)
      return OUTCOME_DRAW;
   else if (evaluation == DBL_MAX)
      return OUTCOME_WIN_WHITE;
   else if (evaluation == -DBL_MAX)
      return OUTCOME_WIN_BLACK;

   move.ToString(best);
   return OUTCOME_UNDECIDED;
}

/*static*/ double SearchingEngine::GetBestMove(
   const Position &position, size_t plies, Move *best, Move *move_buffer)
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
      {
         if (best != nullptr)
            best->promotion_piece = DRAW_PIECE;
         return 0.0;
      }
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
