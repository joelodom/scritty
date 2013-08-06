// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "Engine.h"
#include "UCIParser.h"
#include "Logger.h"

using namespace scritty;

void Engine::SetToStartPos()
{
   memcpy (m_position.m_board.m_squares,
      "RP\0\0\0\0pR"
      "NP\0\0\0\0pn"
      "BP\0\0\0\0pb"
      "QP\0\0\0\0pq"
      "KP\0\0\0\0pk"
      "BP\0\0\0\0pb"
      "NP\0\0\0\0pn"
      "RP\0\0\0\0pR", 64);

   m_position.m_white_to_move = true;
   // TODO: reset all other position stuff
}

Engine::Engine()
{
   SetToStartPos();
}

/*static*/ void Engine::ApplyKnownLegalMoveToPosition(const Move &move,
   Position *position)
{
   position->m_board.m_squares[move.end_file][move.end_rank]
   = position->m_board.m_squares[move.start_file][move.start_rank];
   position->m_board.m_squares[move.start_file][move.start_rank] = '\0';
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

   Logger::LogMessage("Illegal move.");
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
            if (position.m_board.m_squares[move.start_file][rank] != '\0')
              return IsOpponentsPiece(
              position.m_board.m_squares[move.start_file][move.start_rank],
              position.m_board.m_squares[move.end_file][rank])
              && rank == move.end_rank;
         }
      }
      else
      {
         for (unsigned char rank = move.start_rank - 1;
            rank >= move.end_rank; --rank)
         {
            if (position.m_board.m_squares[move.start_file][rank] != '\0')
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
            if (position.m_board.m_squares[file][move.start_rank] != '\0')
              return IsOpponentsPiece(
              position.m_board.m_squares[move.start_file][move.start_rank],
              position.m_board.m_squares[file][move.end_rank])
              && file == move.end_file;
         }
      }
      else
      {
         for (unsigned char file = move.start_file - 1;
            file >= move.end_file; --file)
         {
            if (position.m_board.m_squares[file][move.start_rank] != '\0')
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
      if (position.m_board.m_squares[file][rank] != '\0')
         return false;
      file += file_increment;
      rank += rank_increment;
   }

   // we are at the end file and rank, so motion was legal and not blocked

   return position.m_board.m_squares[file][rank] == '\0' || IsOpponentsPiece(
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
      return position.m_board.m_squares[move.end_file][move.end_rank] == '\0'
         || IsOpponentsPiece(
         position.m_board.m_squares[move.start_file][move.start_rank],
         position.m_board.m_squares[move.end_file][move.end_rank]);
   }

   return false;
}

/*static*/ bool Engine::CanPieceBeCapturedOnNextMove(
   unsigned char file, unsigned char rank, const Position &position)
{
   Move move;
   move.end_file = file;
   move.end_rank = rank;
   
   for (move.start_rank = 0; move.start_rank <= 7; ++move.start_rank)
   {
      for (move.start_file = 0; move.start_file <= 7; ++move.start_file)
      {
         if (IsMoveLegal(position, move))
            return true;
      }
   }

   return false;
}

/*static*/ bool Engine::IsMoveLegal(const Position &position, const Move &move)
{
   // is this the correct player to move and is there a piece there?

   const char piece
      = position.m_board.m_squares[move.start_file][move.start_rank];

   if ((piece == '\0')
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
   case 'P': // TODO: en passant, promotion
      if (move.end_file == move.start_file - 1
         || move.end_file == move.start_file + 1)
      {
         // capture
         if (move.end_rank != move.start_rank + 1)
            return false;
         if (position.m_board.m_squares[move.end_file][move.end_rank] == '\0'
            || !IsOpponentsPiece(piece,
            position.m_board.m_squares[move.end_file][move.end_rank]))
            return false;
         break;
      }

      if (move.start_rank == 1)
      {
         if (move.end_rank - move.start_rank > 2)
            return false;
         if (position.m_board.m_squares[move.end_file][move.start_rank + 1]
         != '\0')
            return false; // blocked
      }
      else
      {
         if (move.end_rank - move.start_rank > 1)
            return false;
      }

      if (move.start_file != move.end_file
         || position.m_board.m_squares[move.end_file][move.end_rank] != '\0')
         return false;

      break;
   case 'p':
      if (move.end_file == move.start_file - 1
         || move.end_file == move.start_file + 1)
      {
         // capture
         if (move.end_rank != move.start_rank - 1)
            return false;
         if (position.m_board.m_squares[move.end_file][move.end_rank] == '\0'
            || !IsOpponentsPiece(piece,
            position.m_board.m_squares[move.end_file][move.end_rank]))
            return false;
         break;
      }

      if (move.start_rank == 6)
      {
         if (move.start_rank - move.end_rank > 2)
            return false;
         if (position.m_board.m_squares[move.end_file][move.start_rank - 1]
         != '\0')
            return false; // blocked
      }
      else
      {
         if (move.start_rank - move.end_rank > 1)
            return false;
      }

      if (move.start_file != move.end_file
         || position.m_board.m_squares[move.end_file][move.end_rank] != '\0')
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
   case 'N': // TODO
   case 'n':
      if (!IsKnightMoveLegal(position, move))
         return false;
      break;
   case 'K': // TODO
   case 'k':
   default:
      return false; // ???
   }

   // is my king in check after the move?

   Position new_position(position);
   ApplyKnownLegalMoveToPosition(move, &new_position);
   char my_king = position.m_white_to_move ? 'K' : 'k';

   for (unsigned char rank = 0; rank <= 7; ++rank)
   {
      for (unsigned char file = 0; file <= 7; ++file)
      {
         if (new_position.m_board.m_squares[file][rank] == my_king)
         {
            if (CanPieceBeCapturedOnNextMove(file, rank, new_position))
               return false;
         }
      }
   }

   return true;
}

char Engine::GetPieceAt(const std::string &square) const
{
   return m_position.m_board.m_squares[square[0] - 'a'][square[1] - '1'];
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

   *best = move.start_file + 'a';
   *best += move.start_rank + '1';
   *best += move.end_file + 'a';
   *best += move.end_rank + '1';
}
