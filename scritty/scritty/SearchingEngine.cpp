// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "SearchingEngine.h"

using namespace scritty;

SearchingEngine::SearchingEngine()
   : m_move_buffer(new Move[MAX_NUMBER_OF_LEGAL_MOVES])
{
}

SearchingEngine::~SearchingEngine()
{
   delete[] m_move_buffer;
}

void SearchingEngine::GetBestMove(std::string *best) const
{
   // assumes that there is a legal move

   size_t num_moves = ListAllLegalMoves(m_position, m_move_buffer);

   Move *best_move = m_move_buffer;
   double best_score = -DBL_MAX;

   for (size_t i = 0; i < num_moves; i++)
   {
      double score = EvaluatePosition(m_position);

      if (m_position.m_white_to_move)
         score = -score;

      if (score > best_score)
      {
         best_move = m_move_buffer + i;
         best_score = score;
      }
   }

   best_move->ToString(best);
}

/*static*/ double SearchingEngine::EvaluatePosition(const Position &position)
{
   return (rand() % 100) / 50.0;
}
