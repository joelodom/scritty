// Scritty is Copyright (c) 2013-2015 by Joel Odom, Marietta, GA, All Rights Reserved

#include "Position.h"

using namespace scritty;

void Position2::SetToStartPos()
{
   // boards are represented as 64-bit integers where the first byte is the first rank and so on

   m_white_pawns = 0x00ff000000000000;
   m_black_pawns = 0x000000000000ff00;

   m_white_bishops = 0x2400000000000000;
   m_black_bishops = 0x0000000000000024;

   m_white_knights = 0x4200000000000000;
   m_black_knights = 0x0000000000000042;

   m_white_rooks = 0x8100000000000000;
   m_black_rooks = 0x0000000000000081;

   m_white_queens = 0x1000000000000000;
   m_black_queens = 0x0000000000000010;

   m_white_kings = 0x0800000000000000;
   m_black_kings = 0x0000000000000008;

   m_white_to_move = true;

   m_white_may_castle_short = true;
   m_white_may_castle_long = true;
   m_black_may_castle_short = true;
   m_black_may_castle_long = true;

   m_en_passant_allowed_on = NO_EN_PASSANT;
}

char Position2::GetPieceAt(unsigned char file, unsigned char rank) const
{
  unsigned __int64 mask = RankAndFileToMask(file, rank);

  if (m_white_pawns & mask)
     return 'P';
  if (m_black_pawns & mask)
     return 'p';

  if (m_white_bishops & mask)
     return 'B';
  if (m_black_bishops & mask)
     return 'b';

  if (m_white_knights & mask)
     return 'N';
  if (m_black_knights & mask)
     return 'n';

  if (m_white_rooks & mask)
     return 'R';
  if (m_black_rooks & mask)
     return 'r';

  if (m_white_queens & mask)
     return 'Q';
  if (m_black_queens & mask)
     return 'q';

  if (m_white_kings & mask)
     return 'K';
  if (m_black_kings & mask)
     return 'k';

  return NO_PIECE;
}

void Position2::ApplyKnownLegalMove(const Move &move)
{
   unsigned __int64 start_mask = RankAndFileToMask(move.start_file, move.start_rank);
   unsigned __int64 end_mask = RankAndFileToMask(move.end_file, move.end_rank);

   // remove all pieces from end mask (to account for capture)

   unsigned __int64 not_end_mask = ~end_mask;

   m_white_pawns &= not_end_mask;
   m_black_pawns &= not_end_mask;

   m_white_bishops &= not_end_mask;
   m_black_bishops &= not_end_mask;

   m_white_knights &= not_end_mask;
   m_black_knights &= not_end_mask;

   m_white_rooks &= not_end_mask;
   m_black_rooks &= not_end_mask;

   m_white_queens &= not_end_mask;
   m_black_queens &= not_end_mask;

   // place moved piece on end mask and remove from start mask

   if (m_white_pawns & start_mask)
   {
      if (end_mask & 0x00000000000000ff)
      {
         // must promote (in some UCI output, promotion pieces are not cased as expected)
         if (move.promotion_piece == 'Q' || move.promotion_piece == 'q')
         {
            m_white_queens |= end_mask;
         }
         else if (move.promotion_piece == 'R' || move.promotion_piece == 'r')
         {
            m_white_rooks |= end_mask;
         }
         else if (move.promotion_piece == 'N' || move.promotion_piece == 'n')
         {
            m_white_knights |= end_mask;
         }
         else // bishop
         {
            m_white_bishops |= end_mask;
         }
      }
      else
      {
         m_white_pawns |= end_mask;
      }

      m_white_pawns ^= start_mask;
   }
   else if (m_black_pawns & start_mask)
   {
      if (end_mask & 0xff00000000000000)
      {
         // must promote (in some UCI output, promotion pieces are not cased as expected)
         if (move.promotion_piece == 'q' || move.promotion_piece == 'Q')
         {
            m_black_queens |= end_mask;
         }
         else if (move.promotion_piece == 'r' || move.promotion_piece == 'R')
         {
            m_black_rooks |= end_mask;
         }
         else if (move.promotion_piece == 'n' || move.promotion_piece == 'N')
         {
            m_black_knights |= end_mask;
         }
         else // bishop
         {
            m_black_bishops |= end_mask;
         }
      }
      else
      {
         m_black_pawns |= end_mask;
      }

      m_black_pawns ^= start_mask;
   }
   else if (m_white_bishops & start_mask)
   {
      m_white_bishops |= end_mask;
      m_white_bishops ^= start_mask;
   }
   else if (m_black_bishops & start_mask)
   {
      m_black_bishops |= end_mask;
      m_black_bishops ^= start_mask;
   }
   else if (m_white_knights & start_mask)
   {
      m_white_knights |= end_mask;
      m_white_knights ^= start_mask;
   }
   else if (m_black_knights & start_mask)
   {
      m_black_knights |= end_mask;
      m_black_knights ^= start_mask;
   }
   else if (m_white_rooks & start_mask)
   {
      m_white_rooks |= end_mask;
      m_white_rooks ^= start_mask;
   }
   else if (m_black_rooks & start_mask)
   {
      m_black_rooks |= end_mask;
      m_black_rooks ^= start_mask;
   }
   else if (m_white_queens & start_mask)
   {
      m_white_queens |= end_mask;
      m_white_queens ^= start_mask;
   }
   else if (m_black_queens & start_mask)
   {
      m_black_queens |= end_mask;
      m_black_queens ^= start_mask;
   }
   else if (m_white_kings & start_mask)
   {
      m_white_kings |= end_mask;
      m_white_kings ^= start_mask;
   }
   else // black king
   {
      m_black_kings |= end_mask;
      m_black_kings ^= start_mask;
   }

   // switch sides
   m_white_to_move = !m_white_to_move;
}
