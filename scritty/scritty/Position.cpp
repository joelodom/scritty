// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "Position.h"
#include "scritty.h"

using namespace scritty;

void Position::SetToStartPos()
{
   memcpy (m_squares,
      "RP\0\0\0\0pr"
      "NP\0\0\0\0pn"
      "BP\0\0\0\0pb"
      "QP\0\0\0\0pq"
      "KP\0\0\0\0pk"
      "BP\0\0\0\0pb"
      "NP\0\0\0\0pn"
      "RP\0\0\0\0pr", sizeof(m_squares));

   m_white_to_move = true;

   m_white_may_castle_short = true;
   m_white_may_castle_long = true;
   m_black_may_castle_short = true;
   m_black_may_castle_long = true;

   m_en_passant_allowed_on = NO_EN_PASSANT;

   *m_chain_length = 0;

   m_hash = POSITION_HASH_MODULUS;
}

void Position::RollBackOneMove()
{
   // there must be a move to roll back!

   Position *previous = m_chain + *m_chain_length - 1;

   m_white_to_move = previous->m_white_to_move;
   m_white_may_castle_short = previous->m_white_may_castle_short;
   m_white_may_castle_long = previous->m_white_may_castle_long;
   m_black_may_castle_short = previous->m_black_may_castle_short;
   m_black_may_castle_long = previous->m_black_may_castle_long;
   m_en_passant_allowed_on = previous->m_en_passant_allowed_on;

   memcpy(m_squares, previous->m_squares, sizeof(previous->m_squares));

   --(*m_chain_length);

   m_hash = POSITION_HASH_MODULUS;
}

void Position::ApplyKnownLegalMove(const Move &move)
{
   // save position (for various draw rules)
   SCRITTY_ASSERT(*m_chain_length < MAX_POSITION_CHAIN_LEN);
   m_chain[(*m_chain_length)++] = *this; // copy to chain

   // move the piece
   m_squares[move.end_file][move.end_rank]
   = m_squares[move.start_file][move.start_rank];
   m_squares[move.start_file][move.start_rank] = NO_PIECE;

   // handle castle rules

   char piece = m_squares[move.end_file][move.end_rank];

   if (move.start_rank == 0)
   {
      if (move.start_file == 0)
      {
         m_white_may_castle_long = false;
      }
      else if (move.start_file == 7)
      {
         m_white_may_castle_short = false;
      }
      else if (move.start_file == 4)
      {
         // possibly move rook
         if (piece == 'K')
         {
            if (move.end_file == 2)
            {
               m_squares[0][0] = NO_PIECE;
               m_squares[3][0] = 'R';
            }
            else if (move.end_file == 6)
            {
               m_squares[7][0] = NO_PIECE;
               m_squares[5][0] = 'R';
            }

            m_white_may_castle_long = false;
            m_white_may_castle_short = false;
         }
      }
   }
   else if (move.start_rank == 7)
   {
      if (move.start_file == 0)
      {
         m_black_may_castle_long = false;
      }
      else if (move.start_file == 7)
      {
         m_black_may_castle_short = false;
      }
      else if (move.start_file == 4)
      {
         // possibly move rook
         if (piece == 'k')
         {
            if (move.end_file == 2)
            {
               m_squares[0][7] = NO_PIECE;
               m_squares[3][7] = 'r';
            }
            else if (move.end_file == 6)
            {
               m_squares[7][7] = NO_PIECE;
               m_squares[5][7] = 'r';
            }

            m_black_may_castle_long = false;
            m_black_may_castle_short = false;
         }
      }
   }

   // handle en passant rules

   if (move.start_rank == 4 && piece == 'P'
      && (move.end_file == move.start_file - 1
      || move.end_file == move.start_file + 1)
      && m_en_passant_allowed_on == move.end_file)
   {
      m_squares[move.end_file][4] = NO_PIECE;
   }
   else if (move.start_rank == 3 && piece == 'p'
      && (move.end_file == move.start_file - 1
      || move.end_file == move.start_file + 1)
      && m_en_passant_allowed_on == move.end_file)
   {
      m_squares[move.end_file][3] = NO_PIECE;
   }

   if (move.start_rank == 1 && move.end_rank == 3 && piece == 'P')
   {
      m_en_passant_allowed_on = move.start_file;
   }
   else if (move.start_rank == 6 && move.end_rank == 4 && piece == 'p')
   {
      m_en_passant_allowed_on = move.start_file;
   }
   else
   {
      m_en_passant_allowed_on = NO_EN_PASSANT;
   }

   // handle promotion
   if ((move.end_rank == 7 && piece == 'P')
      || (move.end_rank == 0 && piece == 'p'))
   {
      // in some UCI output, promotion pieces are not cased as expected
      m_squares[move.end_file][move.end_rank]
      = m_white_to_move
         ? ::toupper(move.promotion_piece) : ::tolower(move.promotion_piece);
   }

   // switch sides
   m_white_to_move = !m_white_to_move;

   // reset hash
   m_hash = POSITION_HASH_MODULUS;
}

bool Position::IsRookMoveLegal(const Move &move) const
{
   if (move.start_file == move.end_file)
   {
      if (move.end_rank > move.start_rank)
      {
         for (unsigned char rank = move.start_rank + 1;
            rank <= move.end_rank; ++rank)
         {
            if (m_squares[move.start_file][rank] != NO_PIECE)
              return IsOpponentsPiece(
              m_squares[move.start_file][move.start_rank],
              m_squares[move.end_file][rank])
              && rank == move.end_rank;
         }
      }
      else
      {
         for (unsigned char rank = move.start_rank - 1;
            (char)rank >= move.end_rank; --rank)
         {
            if (m_squares[move.start_file][rank] != NO_PIECE)
              return IsOpponentsPiece(
              m_squares[move.start_file][move.start_rank],
              m_squares[move.end_file][rank])
              && rank == move.end_rank;
         }
      }

      return true; // reached end of board
   }

   if (move.start_rank == move.end_rank)
   {
      if (move.end_file > move.start_file)
      {
         for (unsigned char file = move.start_file + 1;
            file <= move.end_file; ++file)
         {
            if (m_squares[file][move.start_rank] != NO_PIECE)
              return IsOpponentsPiece(
              m_squares[move.start_file][move.start_rank],
              m_squares[file][move.end_rank])
              && file == move.end_file;
         }
      }
      else
      {
         for (unsigned char file = move.start_file - 1;
            (char)file >= move.end_file; --file)
         {
            if (m_squares[file][move.start_rank] != NO_PIECE)
              return IsOpponentsPiece(
              m_squares[move.start_file][move.start_rank],
              m_squares[file][move.end_rank])
              && file == move.end_file;
         }
      }

      return true; // reached end of board
   }

   return false; // moved off rank or file
}

bool Position::IsBishopMoveLegal(const Move &move) const
{
   char file_increment = move.end_file > move.start_file ? 1 : -1;
   char rank_increment = move.end_rank > move.start_rank ? 1 : -1;

   char file = move.start_file + file_increment;
   char rank = move.start_rank + rank_increment;

   while (file != move.end_file || rank != move.end_rank)
   {
      if (file > 7 || file < 0 || rank > 7 || rank < 0)
         return false;
      if (m_squares[file][rank] != NO_PIECE)
         return false;
      file += file_increment;
      rank += rank_increment;
   }

   // we are at the end file and rank, so motion was legal and not blocked

   return m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(m_squares[move.start_file][move.start_rank],
      m_squares[file][rank]);
}

/*static*/ inline bool Position::IsOpponentsPiece(char mine, char theirs)
{
   return (mine < 'Z' && theirs > 'a') || (mine > 'a' && theirs < 'Z');
}

bool Position::operator==(const Position &other) const
{
   // used to compare for threefold repetition and for position table lookup

   if (m_white_to_move != other.m_white_to_move
      || m_white_may_castle_short != other.m_white_may_castle_short
      || m_white_may_castle_long != other.m_white_may_castle_long
      || m_black_may_castle_short != other.m_black_may_castle_short
      || m_black_may_castle_long != other.m_black_may_castle_long
      || m_en_passant_allowed_on != other.m_en_passant_allowed_on)
      return false;

   return memcmp(m_squares, other.m_squares, sizeof(m_squares)) == 0;
}

bool Position::MayClaimDraw() const
{
   // alse returns true for situations where either side may claim a draw

   // Threefold repetition - if an identical position has just occurred three
   // times with the same player to move, or will occur after the player on turn
   // makes his move, the player on move may claim a draw (to the arbiter). In
   // such a case the draw is not automatic - a player must claim it if he wants
   // the draw. When the position will occur for the third time after the
   // player's intended next move, he writes the move on his scoresheet but does
   // not make the move on the board and claims the draw. Article 9.2 states
   // that a position is considered identical to another if the same player is
   // on move, the same types of pieces of the same colors occupy the same
   // squares, and the same moves are available to each player; in particular,
   // each player has the same castling and en passant capturing rights. (A
   // player may lose his right to castle; and an en passant capture is
   // available only at the first opportunity.) If the claim is not made on the
   // move in which the repetition occurs, the player forfeits the right to make
   // the claim. Of course, the opportunity may present itself again.

   size_t identical_count = 0;
   for (size_t i = 0; i < *m_chain_length; ++i)
   {
      if (*this == m_chain[i])
      {
         if (identical_count == 1)
            return true; // either side may claim a draw
         ++identical_count;
      }
   }

   // The fifty-move rule - if in the previous fifty moves by each side,
   // no pawn has moved and no capture has been made, a draw may be claimed by
   // either player. Here again, the draw is not automatic and must be claimed
   // if the player wants the draw. If the player whose turn it is to move has
   // made only 49 such moves, he may write his next move on the scoresheet and
   // claim a draw. As with the threefold repetition, the right to claim the
   // draw is forfeited if it is not used on that move, but the opportunity may
   // occur again.

   // TODO P2

   // Impossibility of checkmate - if a position arises in which neither player
   // could possibly give checkmate by a series of legal moves, the game is a
   // draw. This is usually because there is insufficient material left, but it
   // is possible in other positions too. Combinations with insufficient
   // material to checkmate are:
   //   king versus king
   //   king and bishop versus king
   //   king and knight versus king
   //   king and bishop versus king and bishop with the bishops on the same
   //     colour. (Any number of additional bishops of either color on the same
   //     color of square due to underpromotion do not affect the situation.)

   // TODO P2 should not go in "May Claim Draw" but should be in "Is A Draw"

   return false;
}

bool Position::IsKnightMoveLegal(const Move &move) const
{
   // check that it is a valid motion

   if ((move.end_rank == move.start_rank + 2
      && move.end_file == move.start_file - 1)

      || (move.end_rank == move.start_rank + 2
      && move.end_file == move.start_file + 1)

      || (move.end_rank == move.start_rank + 1
      && move.end_file == move.start_file + 2)

      || (move.end_rank == move.start_rank - 1
      && move.end_file == move.start_file + 2)

      || (move.end_rank == move.start_rank - 2
      && move.end_file == move.start_file + 1)

      || (move.end_rank == move.start_rank - 2
      && move.end_file == move.start_file - 1)

      || (move.end_rank == move.start_rank - 1
      && move.end_file == move.start_file - 2)

      || (move.end_rank == move.start_rank + 1
      && move.end_file == move.start_file - 2))
   {
      // check that it is a valid empty square or capture
      return m_squares[move.end_file][move.end_rank]
      == NO_PIECE || IsOpponentsPiece(
         m_squares[move.start_file][move.start_rank],
         m_squares[move.end_file][move.end_rank]);
   }

   return false;
}

bool Position::IsAttackingSquare(
   bool white, unsigned char file, unsigned char rank) const
{
   Move move;
   move.end_file = file;
   move.end_rank = rank;
   move.promotion_piece = NO_PIECE;

   for (move.start_rank = 0; move.start_rank <= 7; ++move.start_rank)
   {
      for (move.start_file = 0; move.start_file <= 7; ++move.start_file)
      {
         char piece = m_squares[move.start_file][move.start_rank];

         if (piece != NO_PIECE
            && ((white && piece < 'Z') || (!white && piece > 'a')))
         {
            // notice that we don't check to see if king is in check
            if (IsMoveLegal(move, white, false))
               return true;
         }
      }
   }

   return false;
}

size_t Position::PopulateQueenEndpoints(unsigned char start_file,
   unsigned char start_rank, unsigned char *endpoints) const
{
   size_t endpoints_index = PopulateBishopEndpoints(
      start_file, start_rank, endpoints);
   endpoints_index += PopulateRookEndpoints(
      start_file, start_rank, endpoints + endpoints_index);
   return endpoints_index;
}

size_t Position::PopulateKingEndpoints(unsigned char start_file,
   unsigned char start_rank, unsigned char *endpoints) const
{
   // DOES NOT THINK ABOUT CASTLE

   size_t endpoints_index = 0;

   for (char i = -1; i <= 1; ++i)
      for (char j = -1; j <= 1; ++j)
      {
         if (i == 0 && j == 0)
            continue;

         unsigned char file = start_file + i;
         if (file > 7)
            continue;

         unsigned char rank = start_rank + j;
         if (rank > 7)
            continue;

         endpoints[endpoints_index++] = file;
         endpoints[endpoints_index++] = rank;
         endpoints[endpoints_index++] = NO_PIECE;
      }

   return endpoints_index;
}

size_t Position::PopulateBishopEndpoints(unsigned char start_file,
   unsigned char start_rank, unsigned char *endpoints) const
{
   size_t endpoints_index = 0;

   char file = start_file + 1;
   char rank = start_rank + 1;

   char start_piece = m_squares[start_file][start_rank];

   while (file <= 7 && rank <= 7)
   {
      if (m_squares[file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file++;
         endpoints[endpoints_index++] = rank++;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece, m_squares[file][rank]))
         {
            endpoints[endpoints_index++] = file++;
            endpoints[endpoints_index++] = rank++;
            endpoints[endpoints_index++] = NO_PIECE;
         }

         break;
      }
   }

   file = start_file + 1;
   rank = start_rank - 1;

   while (file <= 7 && rank >= 0)
   {
      if (m_squares[file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file++;
         endpoints[endpoints_index++] = rank--;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece, m_squares[file][rank]))
         {
            endpoints[endpoints_index++] = file++;
            endpoints[endpoints_index++] = rank--;
            endpoints[endpoints_index++] = NO_PIECE;
         }

         break;
      }
   }

   file = start_file - 1;
   rank = start_rank - 1;

   while (file >= 0 && rank >= 0)
   {
      if (m_squares[file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file--;
         endpoints[endpoints_index++] = rank--;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece, m_squares[file][rank]))
         {
            endpoints[endpoints_index++] = file--;
            endpoints[endpoints_index++] = rank--;
            endpoints[endpoints_index++] = NO_PIECE;
         }

         break;
      }
   }

   file = start_file - 1;
   rank = start_rank + 1;

   while (file >= 0 && rank <= 7)
   {
      if (m_squares[file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file--;
         endpoints[endpoints_index++] = rank++;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece, m_squares[file][rank]))
         {
            endpoints[endpoints_index++] = file--;
            endpoints[endpoints_index++] = rank++;
            endpoints[endpoints_index++] = NO_PIECE;
         }

         break;
      }
   }

   return endpoints_index;
}

size_t Position::PopulateRookEndpoints(unsigned char start_file,
   unsigned char start_rank, unsigned char *endpoints) const
{
   size_t endpoints_index = 0;

   char start_piece = m_squares[start_file][start_rank];

   for (char rank = start_rank + 1; rank <= 7; ++rank)
   {
      if (m_squares[start_file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = start_file;
         endpoints[endpoints_index++] = rank;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece, m_squares[start_file][rank]))
         {
            endpoints[endpoints_index++] = start_file;
            endpoints[endpoints_index++] = rank;
            endpoints[endpoints_index++] = NO_PIECE;
         }

         break;
      }
   }

   for (char rank = start_rank - 1; rank >= 0; --rank)
   {
      if (m_squares[start_file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = start_file;
         endpoints[endpoints_index++] = rank;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece, m_squares[start_file][rank]))
         {
            endpoints[endpoints_index++] = start_file;
            endpoints[endpoints_index++] = rank;
            endpoints[endpoints_index++] = NO_PIECE;
         }

         break;
      }
   }

   for (char file = start_file + 1; file <= 7; ++file)
   {
      if (m_squares[file][start_rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file;
         endpoints[endpoints_index++] = start_rank;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece, m_squares[file][start_rank]))
         {
            endpoints[endpoints_index++] = file;
            endpoints[endpoints_index++] = start_rank;
            endpoints[endpoints_index++] = NO_PIECE;
         }

         break;
      }
   }

   for (char file = start_file - 1; file >= 0; --file)
   {
      if (m_squares[file][start_rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file;
         endpoints[endpoints_index++] = start_rank;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece, m_squares[file][start_rank]))
         {
            endpoints[endpoints_index++] = file;
            endpoints[endpoints_index++] = start_rank;
            endpoints[endpoints_index++] = NO_PIECE;
         }

         break;
      }
   }

   return endpoints_index;
}

size_t Position::PopulateKnightEndpoints(unsigned char start_file,
   unsigned char start_rank, unsigned char *endpoints) const
{
   // okay for some endpoints to be off board

   size_t endpoints_index = 0;

   char start_piece = m_squares[start_file][start_rank];

   unsigned char file = start_file + 1;
   unsigned char rank = start_rank + 2;

   if (file <= 7 && rank <= 7 && (m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, m_squares[file][rank])))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file + 2;
   rank = start_rank + 1;

   if (file <= 7 && rank <= 7 && (m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, m_squares[file][rank])))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file + 2;
   rank = start_rank - 1;

   if (file <= 7 && rank <= 7 && (m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, m_squares[file][rank])))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file + 1;
   rank = start_rank - 2;

   if (file <= 7 && rank <= 7 && (m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, m_squares[file][rank])))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file - 1;
   rank = start_rank - 2;

   if (file <= 7 && rank <= 7 && (m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, m_squares[file][rank])))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file - 2;
   rank = start_rank - 1;

   if (file <= 7 && rank <= 7 && (m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, m_squares[file][rank])))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file - 2;
   rank = start_rank + 1;

   if (file <= 7 && rank <= 7 && (m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, m_squares[file][rank])))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file - 1;
   rank = start_rank + 2;

   if (file <= 7 && rank <= 7 && (m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, m_squares[file][rank])))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   return endpoints_index;
}

bool Position::IsMoveLegal(const Move &move) const
{
   return IsMoveLegal(move, m_white_to_move, true);
}

bool Position::IsMoveLegal(
   const Move &move, bool white, bool check_king /*= true*/) const
{
   // is the new position on the board?

   if (move.end_file > 7 || move.end_rank > 7)
      return false;

   // is this the correct player to move and is there a piece there?

   const char piece = m_squares[move.start_file][move.start_rank];

   if ((piece == NO_PIECE)
      || (piece < 'Z' && !white)
      || (piece > 'a' && white))
      return false;

   // did the piece actually move?

   if (move.start_file == move.end_file && move.start_rank == move.end_rank)
      return false;

   // is this a valid motion for the particular piece type?

   switch (piece)
   {
   case 'P':
      // PGN Extract always makes promotion pieces upper case, but UCI example
      // shows one as lower case, so it shouldn't matter.  Also note that FIDE
      // rules require a pawn to be exchanged for another piece.  Keeping a
      // pawn on the last rank is not allowed.

      if (move.end_file == move.start_file - 1
         || move.end_file == move.start_file + 1)
      {
         // capture

         if (move.end_rank != move.start_rank + 1)
            return false;

         if (m_squares[move.end_file][move.end_rank] == NO_PIECE
            || !IsOpponentsPiece(piece,
            m_squares[move.end_file][move.end_rank]))
         {
            // handle en passant
            if (move.start_rank != 4
               || move.end_file != m_en_passant_allowed_on)
               return false;
         }

         break;
      }

      if (move.start_rank == 1)
      {
         if (move.end_rank - move.start_rank > 2)
            return false;
         if (m_squares[move.end_file][move.start_rank + 1] != NO_PIECE)
            return false; // blocked
      }
      else
      {
         if (move.end_rank < move.start_rank
            || move.end_rank - move.start_rank > 1)
            return false;
      }

      if (move.end_rank < move.start_rank
         || move.start_file != move.end_file
         || m_squares[move.end_file][move.end_rank] != NO_PIECE)
         return false;

      // handle promotion
      if (move.end_rank == 7 && move.promotion_piece == NO_PIECE)
         return false;

      break;
   case 'p':
      if (move.end_file == move.start_file - 1
         || move.end_file == move.start_file + 1)
      {
         // capture

         if (move.end_rank != move.start_rank - 1)
            return false;

         if (m_squares[move.end_file][move.end_rank] == NO_PIECE
            || !IsOpponentsPiece(piece,
            m_squares[move.end_file][move.end_rank]))
         {
            // handle en passant
            if (move.start_rank != 3
               || move.end_file != m_en_passant_allowed_on)
               return false;
         }

         break;
      }

      if (move.start_rank == 6)
      {
         if (move.start_rank - move.end_rank > 2)
            return false;
         if (m_squares[move.end_file][move.start_rank - 1] != NO_PIECE)
            return false; // blocked
      }
      else
      {
         if (move.end_rank > move.start_rank
            || move.start_rank - move.end_rank > 1)
            return false;
      }

      if (move.end_rank > move.start_rank
         || move.start_file != move.end_file
         || m_squares[move.end_file][move.end_rank] != NO_PIECE)
         return false;

      // handle promotion
      if (move.end_rank == 0 && move.promotion_piece == NO_PIECE)
         return false;

      break;
   case 'R':
   case 'r':
      if (!IsRookMoveLegal(move))
         return false;
      break;
   case 'B':
   case 'b':
      if (!IsBishopMoveLegal(move))
         return false;
      break;
   case 'Q':
   case 'q':
      if (!(IsRookMoveLegal(move) || IsBishopMoveLegal(move)))
         return false;
      break;
   case 'N':
   case 'n':
      if (!IsKnightMoveLegal(move))
         return false;
      break;
   case 'K':
      if (move.start_file == 4 && move.start_rank == 0
         && move.end_file == 6 && move.end_rank == 0) // castle short
      {
         if (!m_white_may_castle_short)
            return false;

         // check that there are no pieces at f1 or g1

         if (m_squares[5][0] != NO_PIECE|| m_squares[6][0] != NO_PIECE)
            return false;

         // check that black does not attack e1 or f1

         if (IsAttackingSquare(!m_white_to_move, 4, 0)
            || IsAttackingSquare(!m_white_to_move, 5, 0))
            return false;

         // check that the rook is has not been captured
         if (m_squares[7][0] != 'R')
            return false;
      }
      else if (move.start_file == 4 && move.start_rank == 0
         && move.end_file == 2 && move.end_rank == 0) // castle long
      {
         if (!m_white_may_castle_long)
            return false;

         // check that there are no pieces at b1, c1 or d1

         if (m_squares[1][0] != NO_PIECE || m_squares[2][0] != NO_PIECE
            || m_squares[3][0] != NO_PIECE)
            return false;

         // check that black does not attack d1 or e1

         if (IsAttackingSquare(!m_white_to_move, 3, 0)
            || IsAttackingSquare(!m_white_to_move, 4, 0))
            return false;

         // check that the rook is has not been captured
         if (m_squares[0][0] != 'R')
            return false;
      }
      else // normal move
      {
         char files_moved = (char)move.end_file - (char)move.start_file;
         if (files_moved < 0)
            files_moved = -files_moved;
         char ranks_moved = (char)move.end_rank - (char)move.start_rank;
         if (ranks_moved < 0)
            ranks_moved = -ranks_moved;
         if (files_moved > 1 || ranks_moved > 1)
            return false;
         if (m_squares[move.end_file][move.end_rank]
         != NO_PIECE && !IsOpponentsPiece(piece,
            m_squares[move.end_file][move.end_rank]))
            return false;
      }
      break;
   case 'k':
      if (move.start_file == 4 && move.start_rank == 7
         && move.end_file == 6 && move.end_rank == 7) // castle short
      {
         if (!m_black_may_castle_short)
            return false;

         // check that there are no pieces at f8 or g8

         if (m_squares[5][7] != NO_PIECE
            || m_squares[6][7] != NO_PIECE)
            return false;

         // check that white does not attack e8 or f8

         if (IsAttackingSquare(!m_white_to_move, 4, 7)
            || IsAttackingSquare(!m_white_to_move, 5, 7))
            return false;

         // check that the rook is has not been captured
         if (m_squares[7][7] != 'r')
            return false;
      }
      else if (move.start_file == 4 && move.start_rank == 7
         && move.end_file == 2 && move.end_rank == 7) // castle long
      {
         if (!m_black_may_castle_long)
            return false;

         // check that there are no pieces at b8, c8 or d8

         if (m_squares[1][7] != NO_PIECE || m_squares[2][7] != NO_PIECE
            || m_squares[3][7] != NO_PIECE)
            return false;

         // check that white does not attack d8 or e8

         if (IsAttackingSquare(!m_white_to_move, 3, 7)
            || IsAttackingSquare(!m_white_to_move, 4, 7))
            return false;

         // check that the rook is has not been captured
         if (m_squares[0][7] != 'r')
            return false;
      }
      else // normal move
      {
         char files_moved = (char)move.end_file - (char)move.start_file;
         if (files_moved < 0)
            files_moved = -files_moved;
         char ranks_moved = (char)move.end_rank - (char)move.start_rank;
         if (ranks_moved < 0)
            ranks_moved = -ranks_moved;
         if (files_moved > 1 || ranks_moved > 1)
            return false;
         if (m_squares[move.end_file][move.end_rank]
         != NO_PIECE && !IsOpponentsPiece(piece,
            m_squares[move.end_file][move.end_rank]))
            return false;
      }
      break;
   default:
      return false; // ???
   }

   // is my king in check after the move?

   if (check_king)
   {
      Position& must_roll_back = const_cast<Position&>(*this);
      must_roll_back.ApplyKnownLegalMove(move);
      must_roll_back.m_white_to_move = !must_roll_back.m_white_to_move;
      bool is_check = must_roll_back.IsCheck(m_white_to_move);
      must_roll_back.RollBackOneMove();
      if (is_check)
         return false;
   }

   return true;
}

bool Position::IsCheck(bool white) const
{
   char which_king = white ? 'K' : 'k';

   for (unsigned char rank = 0; rank <= 7; ++rank)
   {
      for (unsigned char file = 0; file <= 7; ++file)
      {
         if (m_squares[file][rank] == which_king)
            return IsAttackingSquare(!m_white_to_move, file, rank);
      }
   }

   return false; // should never get here if king is on board
}

/*static*/ size_t Position::s_table_hits = 0;
/*static*/ size_t Position::s_table_misses = 0;

size_t Position::ListAllLegalMoves(Move *buf /*= nullptr*/) const
{
   // pass in null buffer to test if there are any legal moves
   // (returns 0 if no legal moves)

   size_t count = 0;

   // first check the position table
   SCRITTY_ASSERT(m_position_table != nullptr);
   if (m_position_table->Lookup(*this, buf, &count))
   {
      SCRITTY_ASSERT(++s_table_hits > 0);
      return count;
   }

   SCRITTY_ASSERT(++s_table_misses > 0);

   const size_t MAGIC_NUM = 100;
   unsigned char endpoints[8*8*4 + 1]; // f1, r1, promotion1, f2, ..., MAGIC_NUM
   Move move;

   for (move.start_file = 0; move.start_file <= 7; ++move.start_file)
   {
      for (move.start_rank = 0; move.start_rank <= 7; ++move.start_rank)
      {
         size_t endpoints_index = 0;

         char piece = m_squares[move.start_file][move.start_rank];

         if ((piece == NO_PIECE) || (piece > 'a' && m_white_to_move)
            || (piece < 'Z' && !m_white_to_move))
            continue;

         switch (piece)
         {

         case 'P':

            if (move.start_rank < 6)
            {
               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank + 1;
               endpoints[endpoints_index++] = NO_PIECE;

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = NO_PIECE;
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = NO_PIECE;
               }

               if (move.start_rank == 1)
               {
                  endpoints[endpoints_index++] = move.start_file;
                  endpoints[endpoints_index++] = move.start_rank + 2;
                  endpoints[endpoints_index++] = NO_PIECE;
               }
            }
            else
            {
               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank + 1;
               endpoints[endpoints_index++] = 'B';

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = 'B';
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = 'B';
               }

               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank + 1;
               endpoints[endpoints_index++] = 'N';

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = 'N';
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = 'N';
               }

               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank + 1;
               endpoints[endpoints_index++] = 'R';

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = 'R';
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = 'R';
               }

               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank + 1;
               endpoints[endpoints_index++] = 'Q';

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = 'Q';
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank + 1;
                  endpoints[endpoints_index++] = 'Q';
               }
            }

            break;

         case 'p':

            if (move.start_rank > 1)
            {
               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank - 1;
               endpoints[endpoints_index++] = NO_PIECE;

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = NO_PIECE;
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = NO_PIECE;
               }

               if (move.start_rank == 6)
               {
                  endpoints[endpoints_index++] = move.start_file;
                  endpoints[endpoints_index++] = move.start_rank - 2;
                  endpoints[endpoints_index++] = NO_PIECE;
               }
            }
            else
            {
               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank - 1;
               endpoints[endpoints_index++] = 'b';

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = 'b';
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = 'b';
               }

               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank - 1;
               endpoints[endpoints_index++] = 'n';

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = 'n';
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = 'n';
               }

               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank - 1;
               endpoints[endpoints_index++] = 'r';

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = 'r';
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = 'r';
               }

               endpoints[endpoints_index++] = move.start_file;
               endpoints[endpoints_index++] = move.start_rank - 1;
               endpoints[endpoints_index++] = 'q';

               if (move.start_file > 0)
               {
                  endpoints[endpoints_index++] = move.start_file - 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = 'q';
               }

               if (move.start_file < 7)
               {
                  endpoints[endpoints_index++] = move.start_file + 1;
                  endpoints[endpoints_index++] = move.start_rank - 1;
                  endpoints[endpoints_index++] = 'q';
               }
            }

            break;

         case 'B':
         case 'b':

            endpoints_index = PopulateBishopEndpoints(
               move.start_file, move.start_rank, endpoints);
            break;

         case 'N':
         case 'n':

            endpoints_index = PopulateKnightEndpoints(
               move.start_file, move.start_rank, endpoints);
            break;

         case 'R':
         case 'r':

            endpoints_index = PopulateRookEndpoints(
               move.start_file, move.start_rank, endpoints);
            break;

         case 'Q':
         case 'q':

            endpoints_index = PopulateQueenEndpoints(
               move.start_file, move.start_rank, endpoints);
            break;

         case 'K':
         case 'k':

            endpoints_index = PopulateKingEndpoints(
               move.start_file, move.start_rank, endpoints);

            // add castle possibilities

            endpoints[endpoints_index++] = move.start_file - 2;
            endpoints[endpoints_index++] = move.start_rank;
            endpoints[endpoints_index++] = NO_PIECE;

            endpoints[endpoints_index++] = move.start_file - 2;
            endpoints[endpoints_index++] = move.start_rank;
            endpoints[endpoints_index++] = NO_PIECE;

            break;

         default:
            continue; // ???
         }

         endpoints[endpoints_index] = MAGIC_NUM;

         for (endpoints_index = 0; endpoints[endpoints_index] != MAGIC_NUM; )
         {
            move.end_file = endpoints[endpoints_index++];
            move.end_rank = endpoints[endpoints_index++];
            move.promotion_piece = endpoints[endpoints_index++];

            if (move.end_file <= 7 && move.end_rank <= 7 && IsMoveLegal(move))
            {
               if (buf == nullptr)
                  return 1;
               SCRITTY_ASSERT(count < MAX_NUMBER_OF_LEGAL_MOVES);
               buf[count++] = move;
            }
         }
      }
   }

   // save to position table
   if (buf != nullptr)
      m_position_table->Save(*this, buf, count);

   return count;
}

Outcome Position::GetOutcome() const
{
   // check for checkmate or stalemate

   const size_t count = ListAllLegalMoves();

   if (count == 0)
   {
      // Stalemate - if the player on turn has no legal move but is not in
      // check, this is stalemate and the game is automatically a draw.
      if (m_white_to_move)
         return IsCheck(true) ? OUTCOME_WIN_BLACK : OUTCOME_DRAW;
      else
         return IsCheck(false) ? OUTCOME_WIN_WHITE : OUTCOME_DRAW;
   }

   // TODO P2: decide which draw situations to check for here
   // See SearchingEngine's evaluation for checking for draw conditions
   // Arena automatically draws on threefold repetition, even if not claimed

   return OUTCOME_UNDECIDED;
}

unsigned int Position::GetHash() const
{
   if (m_hash == POSITION_HASH_MODULUS)
   {
      // should be uniform and fairly fast
      // inspired by CBC-MAC

      unsigned __int64 x = m_en_passant_allowed_on;

      if (m_white_to_move)
         x += 256;

      if (m_black_may_castle_long)
         x += 512;

      if (m_black_may_castle_short)
         x += 1024;

      if (m_white_may_castle_long)
         x += 2048;

      if (m_white_may_castle_short)
         x += 4096;

      x = powmod(x);

      x = powmod(*((unsigned __int64 *)(m_squares[0])) ^ x);
      x = powmod(*((unsigned __int64 *)(m_squares[1])) ^ x);
      x = powmod(*((unsigned __int64 *)(m_squares[2])) ^ x);
      x = powmod(*((unsigned __int64 *)(m_squares[3])) ^ x);
      x = powmod(*((unsigned __int64 *)(m_squares[4])) ^ x);
      x = powmod(*((unsigned __int64 *)(m_squares[5])) ^ x);
      x = powmod(*((unsigned __int64 *)(m_squares[6])) ^ x);
      x = powmod(*((unsigned __int64 *)(m_squares[7])) ^ x);

      m_hash = (unsigned int)x;
   }

   return m_hash;
}

void PositionTable::Save(const Position &position, const Move* possible_moves,
   size_t possible_moves_size)
{
   SCRITTY_ASSERT(possible_moves != nullptr);

   // save the entry
   PositionTableElement *element = m_table + position.GetHash();
   CalculatedPosition *calculated = element->m_head;
   calculated->position = position; // copy
   memcpy(calculated->possible_moves, possible_moves,
      sizeof(Move)*possible_moves_size);
   calculated->possible_moves_size = possible_moves_size;

   // advanced the head
   ++(element->m_head);

   // move the head back to the beginning if we have passed the end
   if (element->m_head
      == element->m_positions + MAX_CALCULATED_POSITIONS_PER_ELEMENT)
      element->m_head = element->m_positions;

   // increase valid entries if we have not maxed out
   if (element->m_valid_entries < MAX_CALCULATED_POSITIONS_PER_ELEMENT)
      ++(element->m_valid_entries);
}

bool PositionTable::Lookup(const Position &position, Move* possible_moves,
   size_t *possible_moves_size)
{
   // pass in null buffer to test if there are any legal moves

   SCRITTY_ASSERT(possible_moves_size != nullptr);

   PositionTableElement *element = m_table + position.GetHash();

   // start with last inserted and work backwards

   CalculatedPosition *cursor = element->m_head;

   for (size_t entries_checked = 0; entries_checked < element->m_valid_entries;
      ++entries_checked)
   {
      // move the cursor back one
      --cursor;

      // maybe move the cursor to the end of the buffer

      if (cursor < element->m_positions)
         cursor
         = element->m_positions + MAX_CALCULATED_POSITIONS_PER_ELEMENT - 1;

      // check the cursor
      if (cursor->position == position)
      {
         // found

         if (possible_moves != nullptr)
            memcpy(possible_moves, cursor->possible_moves,
            sizeof(Move)*cursor->possible_moves_size);

         *possible_moves_size = cursor->possible_moves_size;
         return true;
      }
   }

   *possible_moves_size = 0;
   return false;
}

void PositionTable::PrintStats() const
{
   size_t entry_counts[MAX_CALCULATED_POSITIONS_PER_ELEMENT + 1];

   for (size_t i = 0; i < MAX_CALCULATED_POSITIONS_PER_ELEMENT; ++i)
      entry_counts[i] = 0;

   for (size_t i = 0; i < POSITION_HASH_MODULUS; ++i)
      ++entry_counts[m_table[i].m_valid_entries];

   std::cout << "Position Table Counts:" << std::endl;
   size_t total = 0;

   for (size_t i = 0; i < MAX_CALCULATED_POSITIONS_PER_ELEMENT; ++i)
   {
      std::cout << i << ": " << entry_counts[i] << std::endl;
      if (i > 0)
         total += entry_counts[i];
   }

   std::cout << "Total non-zero: " << total << std::endl;
}
