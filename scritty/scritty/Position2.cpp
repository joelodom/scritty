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
