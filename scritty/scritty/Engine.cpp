// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "Engine.h"
#include "UCIParser.h"
#include "Logger.h"
#include <iostream>

using namespace scritty;

void Engine::SetToStartPos()
{
   memcpy (m_position.m_board.m_squares,
      "RP\0\0\0\0pr"
      "NP\0\0\0\0pn"
      "BP\0\0\0\0pb"
      "QP\0\0\0\0pq"
      "KP\0\0\0\0pk"
      "BP\0\0\0\0pb"
      "NP\0\0\0\0pn"
      "RP\0\0\0\0pr", 64);

   m_position.m_white_to_move = true;

   m_position.m_white_may_castle_short = true;
   m_position.m_white_may_castle_long = true;
   m_position.m_black_may_castle_short = true;
   m_position.m_black_may_castle_long = true;

   m_position.en_passant_allowed_on = NO_EN_PASSANT;
}

Engine::Engine()
{
   SetToStartPos();
}

/*static*/ void Engine::ApplyKnownLegalMoveToPosition(const Move &move,
   Position *position)
{
   // move the piece
   position->m_board.m_squares[move.end_file][move.end_rank]
   = position->m_board.m_squares[move.start_file][move.start_rank];
   position->m_board.m_squares[move.start_file][move.start_rank] = NO_PIECE;

   // handle castle rules

   char piece = position->m_board.m_squares[move.end_file][move.end_rank];

   if (move.start_rank == 0)
   {
      if (move.start_file == 0)
      {
         position->m_white_may_castle_long = false;
      }
      else if (move.start_file == 7)
      {
         position->m_white_may_castle_short = false;
      }
      else if (move.start_file == 4)
      {
         // possibly move rook
         if (piece == 'K')
         {
            if (move.end_file == 2)
            {
               position->m_board.m_squares[0][0] = NO_PIECE;
               position->m_board.m_squares[3][0] = 'R';
            }
            else if (move.end_file == 6)
            {
               position->m_board.m_squares[7][0] = NO_PIECE;
               position->m_board.m_squares[5][0] = 'R';
            }

            position->m_white_may_castle_long = false;
            position->m_white_may_castle_short = false;
         }
      }
   }
   else if (move.start_rank == 7)
   {
      if (move.start_file == 0)
      {
         position->m_black_may_castle_long = false;
      }
      else if (move.start_file == 7)
      {
         position->m_black_may_castle_short = false;
      }
      else if (move.start_file == 4)
      {
         // possibly move rook
         if (piece == 'k')
         {
            if (move.end_file == 2)
            {
               position->m_board.m_squares[0][7] = NO_PIECE;
               position->m_board.m_squares[3][7] = 'r';
            }
            else if (move.end_file == 6)
            {
               position->m_board.m_squares[7][7] = NO_PIECE;
               position->m_board.m_squares[5][7] = 'r';
            }

            position->m_black_may_castle_long = false;
            position->m_black_may_castle_short = false;
         }
      }
   }

   // handle en passant rules

   if (move.start_rank == 4 && piece == 'P'
      && (move.end_file == move.start_file - 1
      || move.end_file == move.start_file + 1)
      && position->en_passant_allowed_on == move.end_file)
   {
      position->m_board.m_squares[move.end_file][4] = NO_PIECE;
   }
   else if (move.start_rank == 3 && piece == 'p'
      && (move.end_file == move.start_file - 1
      || move.end_file == move.start_file + 1)
      && position->en_passant_allowed_on == move.end_file)
   {
      position->m_board.m_squares[move.end_file][3] = NO_PIECE;
   }

   if (move.start_rank == 1 && move.end_rank == 3 && piece == 'P')
   {
      position->en_passant_allowed_on = move.start_file;
   }
   else if (move.start_rank == 6 && move.end_rank == 4 && piece == 'p')
   {
      position->en_passant_allowed_on = move.start_file;
   }
   else
   {
      position->en_passant_allowed_on = NO_EN_PASSANT;
   }

   // handle promotion
   if ((move.end_rank == 7 && piece == 'P')
      || (move.end_rank == 0 && piece == 'p'))
   {
      // in some UCI output, promotion pieces are not cased as expected
      position->m_board.m_squares[move.end_file][move.end_rank]
      = position->m_white_to_move
         ? ::toupper(move.promotion_piece) : ::tolower(move.promotion_piece);
   }

   // switch sides
   position->m_white_to_move = !position->m_white_to_move;
}

bool Engine::ApplyMove(const std::string &str)
{
   Move move;

   if (!UCIParser::ParseMove(str, &move))
   {
      Logger::LogMessage("ParseMove failed.");
      return false;
   }

   if (IsMoveLegal(m_position, move))
   {
      ApplyKnownLegalMoveToPosition(move, &m_position);
      return true;
   }

   Logger::GetStream() << "Illegal move: " << str << std::endl;
   return false;
}

bool Engine::IsWhiteToMove() const
{
   return m_position.m_white_to_move;
}

/*static*/ inline bool Engine::IsOpponentsPiece(char mine, char theirs)
{
   return (mine < 'Z' && theirs > 'a') || (mine > 'a' && theirs < 'Z');
}

/*static*/ inline bool Engine::IsRookMoveLegal(
   const Position &position, const Move &move)
{
   if (move.start_file == move.end_file)
   {
      if (move.end_rank > move.start_rank)
      {
         for (unsigned char rank = move.start_rank + 1;
            rank <= move.end_rank; ++rank)
         {
            if (position.m_board.m_squares[move.start_file][rank] != NO_PIECE)
              return IsOpponentsPiece(
              position.m_board.m_squares[move.start_file][move.start_rank],
              position.m_board.m_squares[move.end_file][rank])
              && rank == move.end_rank;
         }
      }
      else
      {
         for (unsigned char rank = move.start_rank - 1;
            (char)rank >= move.end_rank; --rank)
         {
            if (position.m_board.m_squares[move.start_file][rank] != NO_PIECE)
              return IsOpponentsPiece(
              position.m_board.m_squares[move.start_file][move.start_rank],
              position.m_board.m_squares[move.end_file][rank])
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
            if (position.m_board.m_squares[file][move.start_rank] != NO_PIECE)
              return IsOpponentsPiece(
              position.m_board.m_squares[move.start_file][move.start_rank],
              position.m_board.m_squares[file][move.end_rank])
              && file == move.end_file;
         }
      }
      else
      {
         for (unsigned char file = move.start_file - 1;
            (char)file >= move.end_file; --file)
         {
            if (position.m_board.m_squares[file][move.start_rank] != NO_PIECE)
              return IsOpponentsPiece(
              position.m_board.m_squares[move.start_file][move.start_rank],
              position.m_board.m_squares[file][move.end_rank])
              && file == move.end_file;
         }
      }

      return true; // reached end of board
   }

   return false; // moved off rank or file
}

/*static*/ inline bool Engine::IsBishopMoveLegal(
   const Position &position, const Move &move)
{
   char file_increment = move.end_file > move.start_file ? 1 : -1;
   char rank_increment = move.end_rank > move.start_rank ? 1 : -1;

   char file = move.start_file + file_increment;
   char rank = move.start_rank + rank_increment;

   while (file != move.end_file || rank != move.end_rank)
   {
      if (file > 7 || file < 0 || rank > 7 || rank < 0)
         return false;
      if (position.m_board.m_squares[file][rank] != NO_PIECE)
         return false;
      file += file_increment;
      rank += rank_increment;
   }

   // we are at the end file and rank, so motion was legal and not blocked

   return position.m_board.m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(
      position.m_board.m_squares[move.start_file][move.start_rank],
      position.m_board.m_squares[file][rank]);
}

/*static*/ inline bool Engine::IsKnightMoveLegal(
   const Position &position, const Move &move)
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
      return position.m_board.m_squares[move.end_file][move.end_rank]
      == NO_PIECE || IsOpponentsPiece(
         position.m_board.m_squares[move.start_file][move.start_rank],
         position.m_board.m_squares[move.end_file][move.end_rank]);
   }

   return false;
}

/*static*/ bool Engine::IsAttackingSquare(bool white,
   unsigned char file, unsigned char rank, const Position &position)
{
   Move move;
   move.end_file = file;
   move.end_rank = rank;
   move.promotion_piece = NO_PIECE;

   // MUST switch back to obey const
   bool was_white_to_move = position.m_white_to_move;
   const_cast<Position&>(position).m_white_to_move = white;

   for (move.start_rank = 0; move.start_rank <= 7; ++move.start_rank)
   {
      for (move.start_file = 0; move.start_file <= 7; ++move.start_file)
      {
         char piece
            = position.m_board.m_squares[move.start_file][move.start_rank];

         if (piece != NO_PIECE
            && ((white && piece < 'Z')
            || (!white && piece > 'a')))
         {
            // notice that we don't check to see if king is in check
            if (IsMoveLegal(position, move, false))
            {
               const_cast<Position&>(position).m_white_to_move
                  = was_white_to_move;
               return true;
            }
         }
      }
   }

   const_cast<Position&>(position).m_white_to_move = was_white_to_move;
   return false;
}

/*static*/ bool Engine::IsMoveLegal(
   const Position &position, const Move &move, bool check_king /*= true*/)
{
   // TODO: go to pre-calculated move lists like:
   // legal_moves[piece][start_file][start_rank][end_file][end_rank] is a null
   // terminated list of squares to check for interposing pieces

   // is the new position on the board?

   if (move.end_file > 7 || move.end_rank > 7)
      return false;

   // is this the correct player to move and is there a piece there?

   const char piece
      = position.m_board.m_squares[move.start_file][move.start_rank];

   if ((piece == NO_PIECE)
      || (piece < 'Z' && !position.m_white_to_move)
      || (piece > 'a' && position.m_white_to_move))
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

         if (position.m_board.m_squares[move.end_file][move.end_rank]
         == NO_PIECE || !IsOpponentsPiece(piece,
            position.m_board.m_squares[move.end_file][move.end_rank]))
         {
            // handle en passant
            if (move.start_rank != 4
               || move.end_file != position.en_passant_allowed_on)
               return false;
         }

         break;
      }

      if (move.start_rank == 1)
      {
         if (move.end_rank - move.start_rank > 2)
            return false;
         if (position.m_board.m_squares[move.end_file][move.start_rank + 1]
         != NO_PIECE)
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
         || position.m_board.m_squares[move.end_file][move.end_rank]
      != NO_PIECE)
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

         if (position.m_board.m_squares[move.end_file][move.end_rank]
         == NO_PIECE || !IsOpponentsPiece(piece,
            position.m_board.m_squares[move.end_file][move.end_rank]))
         {
            // handle en passant
            if (move.start_rank != 3
               || move.end_file != position.en_passant_allowed_on)
               return false;
         }

         break;
      }

      if (move.start_rank == 6)
      {
         if (move.start_rank - move.end_rank > 2)
            return false;
         if (position.m_board.m_squares[move.end_file][move.start_rank - 1]
         != NO_PIECE)
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
         || position.m_board.m_squares[move.end_file][move.end_rank]
      != NO_PIECE)
         return false;

      // handle promotion
      if (move.end_rank == 0 && move.promotion_piece == NO_PIECE)
         return false;

      break;
   case 'R':
   case 'r':
      if (!IsRookMoveLegal(position, move))
         return false;
      break;
   case 'B':
   case 'b':
      if (!IsBishopMoveLegal(position, move))
         return false;
      break;
   case 'Q':
   case 'q':
      if (!IsRookMoveLegal(position, move)
         && !IsBishopMoveLegal(position, move))
         return false;
      break;
   case 'N':
   case 'n':
      if (!IsKnightMoveLegal(position, move))
         return false;
      break;
   case 'K':
      if (move.start_file == 4 && move.start_rank == 0
         && move.end_file == 6 && move.end_rank == 0) // castle short
      {
         if (!position.m_white_may_castle_short)
            return false;

         // check that there are no pieces at f1 or g1

         if (position.m_board.m_squares[5][0] != NO_PIECE
            || position.m_board.m_squares[6][0] != NO_PIECE)
            return false;

         // check that black does not attack e1 or f1

         if (IsAttackingSquare(!position.m_white_to_move, 4, 0, position)
            || IsAttackingSquare(
            !position.m_white_to_move, 5, 0, position))
            return false;
      }
      else if (move.start_file == 4 && move.start_rank == 0
         && move.end_file == 2 && move.end_rank == 0) // castle long
      {
         if (!position.m_white_may_castle_long)
            return false;

         // check that there are no pieces at b1, c1 or d1

         if (position.m_board.m_squares[1][0] != NO_PIECE
            || position.m_board.m_squares[2][0] != NO_PIECE
            || position.m_board.m_squares[3][0] != NO_PIECE)
            return false;

         // check that black does not attack d1 or e1

         if (IsAttackingSquare(!position.m_white_to_move, 3, 0, position)
            || IsAttackingSquare(!position.m_white_to_move, 4, 0, position))
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
         if (position.m_board.m_squares[move.end_file][move.end_rank]
         != NO_PIECE && !IsOpponentsPiece(piece,
            position.m_board.m_squares[move.end_file][move.end_rank]))
            return false;
      }
      break;
   case 'k':
      if (move.start_file == 4 && move.start_rank == 7
         && move.end_file == 6 && move.end_rank == 7) // castle short
      {
         if (!position.m_black_may_castle_short)
            return false;

         // check that there are no pieces at f8 or g8

         if (position.m_board.m_squares[5][7] != NO_PIECE
            || position.m_board.m_squares[6][7] != NO_PIECE)
            return false;

         // check that white does not attack e8 or f8

         if (IsAttackingSquare(!position.m_white_to_move, 4, 7, position)
            || IsAttackingSquare(!position.m_white_to_move, 5, 7, position))
            return false;
      }
      else if (move.start_file == 4 && move.start_rank == 7
         && move.end_file == 2 && move.end_rank == 7) // castle long
      {
         if (!position.m_black_may_castle_long)
            return false;

         // check that there are no pieces at b8, c8 or d8

         if (position.m_board.m_squares[1][7] != NO_PIECE
            || position.m_board.m_squares[2][7] != NO_PIECE
            || position.m_board.m_squares[3][7] != NO_PIECE)
            return false;

         // check that white does not attack d8 or e8

         if (IsAttackingSquare(!position.m_white_to_move, 3, 7, position)
            || IsAttackingSquare(!position.m_white_to_move, 4, 7, position))
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
         if (position.m_board.m_squares[move.end_file][move.end_rank]
         != NO_PIECE && !IsOpponentsPiece(piece,
            position.m_board.m_squares[move.end_file][move.end_rank]))
            return false;
      }
      break;
   default:
      return false; // ???
   }

   // is my king in check after the move?

   if (check_king)
   {
      Position new_position(position);
      ApplyKnownLegalMoveToPosition(move, &new_position);
      new_position.m_white_to_move = !new_position.m_white_to_move;
      if (IsCheck(new_position, position.m_white_to_move ? 'K' : 'k'))
         return false;
   }

   return true;
}

/*static*/ bool Engine::IsCheck(
   const Position &position, const char which_king)
{
   for (unsigned char rank = 0; rank <= 7; ++rank)
   {
      for (unsigned char file = 0; file <= 7; ++file)
      {
         if (position.m_board.m_squares[file][rank] == which_king)
            return IsAttackingSquare(
            !position.m_white_to_move, file, rank, position);
      }
   }

   return false; // should never get here if king is on board
}

char Engine::GetPieceAt(const std::string &square) const
{
   return m_position.m_board.m_squares[square[0] - 'a'][square[1] - '1'];
}

void Move::ToString(std::string *str)
{
   *str = start_file + 'a';
   *str += start_rank + '1';
   *str += end_file + 'a';
   *str += end_rank + '1';

   if (promotion_piece != NO_PIECE)
      *str += promotion_piece;
}

/*static*/ void Engine::WritePositionToStdout(const Position &position)
{
   for (unsigned char rank = 7; (char)rank >= 0; --rank)
   {
      for (unsigned char file = 0; file <= 7; ++file)
      {
         if (position.m_board.m_squares[file][rank] == NO_PIECE)
            std::cout << '.';
         else
            std::cout << position.m_board.m_squares[file][rank];
      }

      std::cout << std::endl;
   }

   std::cout << (position.m_white_to_move ? "White" : "Black") << " to move."
      << std::endl;
}

/*static*/ size_t Engine::ListAllLegalMoves(
   const Position &position, Move *buf /*= nullptr*/)
{
   // pass in null buffer to test if there are any legal moves (returns 0 or 1)

   const size_t MAGIC_NUM = 100;
   size_t count = 0;
   unsigned char endpoints[8*8*4 + 1]; // f1, r1, promotion1, f2, ..., MAGIC_NUM
   Move move;

   for (move.start_file = 0; move.start_file <= 7; ++move.start_file)
   {
      for (move.start_rank = 0; move.start_rank <= 7; ++move.start_rank)
      {
         size_t endpoints_index = 0;

         char piece
            = position.m_board.m_squares[move.start_file][move.start_rank];

         if ((piece == NO_PIECE)
            || (piece > 'a' && position.m_white_to_move)
            || (piece < 'Z' && !position.m_white_to_move))
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
               position, move.start_file, move.start_rank, endpoints);
            break;

         case 'N':
         case 'n':

            endpoints_index = PopulateKnightEndpoints(position,
               move.start_file, move.start_rank, endpoints);
            break;

         case 'R':
         case 'r':

            endpoints_index = PopulateRookEndpoints(
               position, move.start_file, move.start_rank, endpoints);
            break;

         case 'Q':
         case 'q':

            endpoints_index = PopulateQueenEndpoints(
               position, move.start_file, move.start_rank, endpoints);
            break;

         case 'K':
         case 'k':

            endpoints_index = PopulateKingEndpoints(
               position, move.start_file, move.start_rank, endpoints);

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

            if (move.end_file <= 7 && move.end_rank <= 7
               && IsMoveLegal(position, move))
            {
               if (buf == nullptr)
                  return 1;
               buf[count++] = move;
               if (count > MAX_NUMBER_OF_LEGAL_MOVES) // safety check
                  return 0;
            }
         }
      }
   }

   return count;
}

/*static*/ size_t Engine::PopulateQueenEndpoints(const Position &position,
   unsigned char start_file, unsigned char start_rank, unsigned char *endpoints)
{
   size_t endpoints_index = PopulateBishopEndpoints(
      position, start_file, start_rank, endpoints);
   endpoints_index += PopulateRookEndpoints(
      position, start_file, start_rank,
      endpoints + endpoints_index);
   return endpoints_index;
}

/*static*/ size_t Engine::PopulateKingEndpoints(const Position &position,
   unsigned char start_file, unsigned char start_rank, unsigned char *endpoints)
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

/*static*/ size_t Engine::PopulateBishopEndpoints(const Position &position,
   unsigned char start_file, unsigned char start_rank, unsigned char *endpoints)
{
   size_t endpoints_index = 0;

   char file = start_file + 1;
   char rank = start_rank + 1;

   char start_piece = position.m_board.m_squares[start_file][start_rank];

   while (file <= 7 && rank <= 7)
   {
      if (position.m_board.m_squares[file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file++;
         endpoints[endpoints_index++] = rank++;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece,
            position.m_board.m_squares[file][rank]))
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
      if (position.m_board.m_squares[file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file++;
         endpoints[endpoints_index++] = rank--;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece,
            position.m_board.m_squares[file][rank]))
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
      if (position.m_board.m_squares[file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file--;
         endpoints[endpoints_index++] = rank--;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece,
            position.m_board.m_squares[file][rank]))
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
      if (position.m_board.m_squares[file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file--;
         endpoints[endpoints_index++] = rank++;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece,
            position.m_board.m_squares[file][rank]))
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

/*static*/ size_t Engine::PopulateRookEndpoints(const Position &position,
   unsigned char start_file, unsigned char start_rank, unsigned char *endpoints)
{
   size_t endpoints_index = 0;

   char start_piece = position.m_board.m_squares[start_file][start_rank];

   for (char rank = start_rank + 1; rank <= 7; ++rank)
   {
      if (position.m_board.m_squares[start_file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = start_file;
         endpoints[endpoints_index++] = rank;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece,
            position.m_board.m_squares[start_file][rank]))
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
      if (position.m_board.m_squares[start_file][rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = start_file;
         endpoints[endpoints_index++] = rank;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece,
            position.m_board.m_squares[start_file][rank]))
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
      if (position.m_board.m_squares[file][start_rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file;
         endpoints[endpoints_index++] = start_rank;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece,
            position.m_board.m_squares[file][start_rank]))
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
      if (position.m_board.m_squares[file][start_rank] == NO_PIECE)
      {
         endpoints[endpoints_index++] = file;
         endpoints[endpoints_index++] = start_rank;
         endpoints[endpoints_index++] = NO_PIECE;
      }
      else
      {
         if (IsOpponentsPiece(start_piece,
            position.m_board.m_squares[file][start_rank]))
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

/*static*/ size_t Engine::PopulateKnightEndpoints(const Position &position,
   unsigned char start_file, unsigned char start_rank, unsigned char *endpoints)
{
   // okay for some endpoints to be off board

   size_t endpoints_index = 0;

   char start_piece = position.m_board.m_squares[start_file][start_rank];

   unsigned char file = start_file + 1;
   unsigned char rank = start_rank + 2;

   if (file <= 7 && rank <= 7
      && position.m_board.m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, position.m_board.m_squares[file][rank]))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file + 2;
   rank = start_rank + 1;

   if (file <= 7 && rank <= 7
      && position.m_board.m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, position.m_board.m_squares[file][rank]))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file + 2;
   rank = start_rank - 1;

   if (file <= 7 && rank <= 7
      && position.m_board.m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, position.m_board.m_squares[file][rank]))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file + 1;
   rank = start_rank - 2;

   if (file <= 7 && rank <= 7
      && position.m_board.m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, position.m_board.m_squares[file][rank]))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file - 1;
   rank = start_rank - 2;

   if (file <= 7 && rank <= 7
      && position.m_board.m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, position.m_board.m_squares[file][rank]))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file - 2;
   rank = start_rank - 1;

   if (file <= 7 && rank <= 7
      && position.m_board.m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, position.m_board.m_squares[file][rank]))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file - 2;
   rank = start_rank + 1;

   if (file <= 7 && rank <= 7
      && position.m_board.m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, position.m_board.m_squares[file][rank]))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   file = start_file - 1;
   rank = start_rank + 2;

   if (file <= 7 && rank <= 7
      && position.m_board.m_squares[file][rank] == NO_PIECE
      || IsOpponentsPiece(start_piece, position.m_board.m_squares[file][rank]))
   {
      endpoints[endpoints_index++] = file;
      endpoints[endpoints_index++] = rank;
      endpoints[endpoints_index++] = NO_PIECE;
   }

   return endpoints_index;
}

/*static*/ Outcome Engine::GetOutcome(const Position &position)
{
   // check for checkmate or stalemate

   const size_t count = ListAllLegalMoves(position, nullptr);

   if (count == 0)
   {
      // Stalemate - if the player on turn has no legal move but is not in
      // check, this is stalemate and the game is automatically a draw.
      if (position.m_white_to_move)
         return IsCheck(position, 'K') ? OUTCOME_WIN_BLACK : OUTCOME_DRAW;
      else
         return IsCheck(position, 'k') ? OUTCOME_WIN_WHITE : OUTCOME_DRAW;
   }

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

   // TODO: a threefold repetition must be claimed as a draw, consider
   // how to handle

   // The fifty-move rule - if in the previous fifty moves by each side,
   // no pawn has moved and no capture has been made, a draw may be claimed by
   // either player. Here again, the draw is not automatic and must be claimed
   // if the player wants the draw. If the player whose turn it is to move has
   // made only 49 such moves, he may write his next move on the scoresheet and
   // claim a draw. As with the threefold repetition, the right to claim the
   // draw is forfeited if it is not used on that move, but the opportunity may
   // occur again.

   // TODO: a fifty move draw must be claimed as a draw, consider how to handle

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

   // TODO: ...

   return OUTCOME_UNDECIDED;
}

Outcome Engine::GetOutcome() const
{
   return GetOutcome(m_position);
}

void Engine::GetPosition(Position *position) const
{
   // makes a copy
   *position = m_position;
}
