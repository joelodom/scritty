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

/*static*/ inline bool Engine::IsOnBoard(unsigned char file, unsigned char rank)
{
   return rank <= 7 && rank >= 0 && file <= 7 && file >= 0;
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
      if (!IsOnBoard(file, rank))
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

/*static*/ bool Engine::IsOpponentAttackingSquare(
   unsigned char file, unsigned char rank, const Position &position)
{
   Move move;
   move.end_file = file;
   move.end_rank = rank;
   move.promotion_piece = NO_PIECE;

   // MUST switch back to obey const
   const_cast<Position&>(position).m_white_to_move = !position.m_white_to_move;
   
   for (move.start_rank = 0; move.start_rank <= 7; ++move.start_rank)
   {
      for (move.start_file = 0; move.start_file <= 7; ++move.start_file)
      {
         // notice that we don't check to see if our king is in check
         if (IsMoveLegal(position, move, false))
         {
            const_cast<Position&>(position).m_white_to_move
               = !position.m_white_to_move;
            return true;
         }
      }
   }

   const_cast<Position&>(position).m_white_to_move = !position.m_white_to_move;
   return false;
}

/*static*/ bool Engine::IsMoveLegal(
   const Position &position, const Move &move, bool check_king /*= true*/)
{
   // TODO: go to pre-calculated move lists like:
   // legal_moves[piece][start_file][start_rank][end_file][end_rank] is a null
   // terminated list of squares to check for interposing pieces

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

   // is the new position on the board?

   if (!IsOnBoard(move.end_file, move.end_rank))
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
            if (move.end_file != position.en_passant_allowed_on)
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
            if (move.end_file != position.en_passant_allowed_on)
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

         if (IsOpponentAttackingSquare(4, 0, position)
            || IsOpponentAttackingSquare(5, 0, position))
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

         if (IsOpponentAttackingSquare(3, 0, position)
            || IsOpponentAttackingSquare(4, 0, position))
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

         if (IsOpponentAttackingSquare(4, 7, position)
            || IsOpponentAttackingSquare(5, 7, position))
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

         if (IsOpponentAttackingSquare(3, 7, position)
            || IsOpponentAttackingSquare(4, 7, position))
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
      char my_king = position.m_white_to_move ? 'K' : 'k';

      for (unsigned char rank = 0; rank <= 7; ++rank)
      {
         for (unsigned char file = 0; file <= 7; ++file)
         {
            if (new_position.m_board.m_squares[file][rank] == my_king)
            {
               if (IsOpponentAttackingSquare(file, rank, new_position))
                  return false;
            }
         }
      }
   }

   return true;
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

void Engine::GetBestMove(std::string *best)
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

/*static*/ void Engine::WritePositionToStdout(const Position &position)
{
   for (unsigned char rank = 7; (char)rank >= 0; --rank)
   {
      for (unsigned char file = 0; file <= 7; ++file)
      {
         std::cout << position.m_board.m_squares[file][rank];
      }
      std::cout << std::endl;
   }
   std::cout << (position.m_white_to_move ? "White" : "Black") << " to move."
      << std::endl;
}
