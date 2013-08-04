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
      m_position.m_board.m_squares[move.end_file][move.end_rank]
         = m_position.m_board.m_squares[move.start_file][move.start_rank];
      m_position.m_board.m_squares[move.start_file][move.start_rank] = '\0';
      m_position.m_white_to_move = !m_position.m_white_to_move;
      return true;
   }

   Logger::LogMessage("illegal move");
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

/*static*/ bool Engine::IsRookMoveLegal(
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

/*static*/ bool Engine::IsBishopMoveLegal(
   const Position &position, const Move &move)
{
   char file_increment = move.end_file > move.start_file ? 1 : -1;
   char rank_increment = move.end_rank > move.start_rank ? 1 : -1;

   char file = move.start_file + file_increment;
   char rank = move.start_rank + rank_increment;

   while (file != move.end_file && rank != move.end_rank)
   {
      if (!IsOnBoard(file, rank))
         return false;
      if (position.m_board.m_squares[file][rank] != '\0')
         return IsOpponentsPiece(
         position.m_board.m_squares[move.start_file][move.start_rank],
         position.m_board.m_squares[file][rank])
         && file == move.end_file && rank == move.end_rank;
      file += file_increment;
      rank += rank_increment;
   }

   return true;
}

/*static*/ bool Engine::IsMoveLegal(const Position &position, const Move &move)
{
   // is this the correct player to move and is there a piece there?

   const char piece
      = position.m_board.m_squares[move.start_file][move.start_rank];

   if ((piece < 'Z' && !position.m_white_to_move)
      || (piece > 'a' && position.m_white_to_move)
      || (piece == '\0'))
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
   case 'P': // TODO: en passant, promotion, captures
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
   case 'K': // TODO
   case 'k':
   default:
      return false; // ???
   }

   // if king was in check, is now king out of check? TODO

   return true;
}

char Engine::GetPieceAt(const std::string &square) const
{
   return m_position.m_board.m_squares[square[0] - 'a'][square[1] - '1'];
}
