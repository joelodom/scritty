// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "Engine.h"
#include "UCIParser.h"
#include "Logger.h"

using namespace scritty;

void Move::ToString(std::string *str)
{
   *str = start_file + 'a';
   *str += start_rank + '1';
   *str += end_file + 'a';
   *str += end_rank + '1';

   if (promotion_piece != NO_PIECE)
      *str += promotion_piece;
}

bool Move::operator==(const Move &other) const
{
   return start_file == other.start_file
      && start_rank == other.start_rank
      && end_file == other.end_file
      && end_rank == other.end_rank
      && promotion_piece == other.promotion_piece;
}

Move& Move::operator=(const Move &rhs)
{
   if (this != &rhs)
   {
      start_file = rhs.start_file;
      start_rank = rhs.start_rank;
      end_file = rhs.end_file;
      end_rank = rhs.end_rank;
      promotion_piece = rhs.promotion_piece;
   }

   return *this;
}

bool Engine::ApplyMove(const std::string &str)
{
   Move move;

   if (!UCIParser::ParseMove(str, &move))
   {
      Logger::LogMessage("ParseMove failed.");
      return false;
   }

   if (m_position->IsMoveLegal(move))
   {
      m_position->ApplyKnownLegalMove(move);
      return true;
   }

   Logger::GetStream() << "Illegal move: " << str << std::endl;
   return false;
}

bool Engine::IsWhiteToMove() const
{
   return m_position->IsWhiteToMove();
}

char Engine::GetPieceAt(const std::string &square) const
{
   return m_position->GetPieceAt(square[0] - 'a', square[1] - '1');
}

Outcome Engine::GetOutcome() const
{
   return m_position->GetOutcome();
}

const Position &Engine::GetPosition() const
{
   return *m_position;
}
