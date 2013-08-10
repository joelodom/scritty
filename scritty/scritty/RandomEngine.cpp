// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "RandomEngine.h"

using namespace scritty;

void RandomEngine::GetBestMove(std::string *best) const
{
   // possibly the smartest chess algorithm of all time

   Move move;

   do
   {
      move.start_file = rand() % 8;
      move.start_rank = rand() % 8;
      move.end_file = rand() % 8;
      move.end_rank = rand() % 8;
   } while (!IsMoveLegal(m_position, move));

   // handle promotion

   char piece = m_position.m_board.m_squares[move.start_file][move.start_rank];

   if (move.end_rank == 7 && piece == 'P')
      move.promotion_piece = 'Q';
   else if (move.end_rank == 0 && piece == 'p')
      move.promotion_piece = 'q';
   else
      move.promotion_piece = NO_PIECE;

   move.ToString(best);
}
