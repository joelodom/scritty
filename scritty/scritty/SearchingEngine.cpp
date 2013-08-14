// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "SearchingEngine.h"
#include <iostream>

using namespace scritty;

SearchingEngine::SearchingEngine()
{
   m_parameters.push_back(std::pair<std::string, double>(
      "Pawn Value", 1.00));
   m_parameters.push_back(std::pair<std::string, double>(
      "Bishop Value", 3.00));
   m_parameters.push_back(std::pair<std::string, double>(
      "Knight Value", 3.00));
   m_parameters.push_back(std::pair<std::string, double>(
      "Rook Value", 5.00));
   m_parameters.push_back(std::pair<std::string, double>(
      "Queen Value", 9.00));

   m_parameters.push_back(std::pair<std::string, double>(
      "Square Control Value", 0.03));
}

SearchingEngine::~SearchingEngine()
{
}

Outcome SearchingEngine::GetBestMove(std::string *best) const
{
   Move move;
   // the move buffer for all depths is allocated once for performance
   Move *move_buffer = new Move[SEARCH_DEPTH*MAX_NUMBER_OF_LEGAL_MOVES];
   double evaluation = GetBestMove(
      m_position, SEARCH_DEPTH, &move, move_buffer);
   delete[] move_buffer;
   move.ToString(best);
   return OUTCOME_UNDECIDED; // don't ever give up
}

double SearchingEngine::GetBestMove(
   const Position &position, size_t plies, Move *best, Move *move_buffer) const
{
   // one ply is one half-turn in chess

   size_t num_moves = ListAllLegalMoves(position, move_buffer);

   if (num_moves == 0)
   {
      Outcome outcome = GetOutcome(position);

      if (outcome == OUTCOME_WIN_WHITE)
         return DBL_MAX;
      else if (outcome == OUTCOME_WIN_BLACK)
         return -DBL_MAX;
      else
         return 0.0;
   }

   Move *best_move = move_buffer;
   double best_evaluation = position.m_white_to_move ? -DBL_MAX : DBL_MAX;

   for (size_t i = 0; i < num_moves; ++i)
   {
      Position new_position = position;
      Engine::ApplyKnownLegalMoveToPosition(move_buffer[i], &new_position);

      // TODO: account for outcomes

      double new_position_evaluation = plies == 1
         ? new_position_evaluation = EvaluatePosition(new_position)
         : GetBestMove(new_position, plies - 1, nullptr,
         move_buffer + MAX_NUMBER_OF_LEGAL_MOVES);

      if (position.m_white_to_move ? new_position_evaluation > best_evaluation
         : new_position_evaluation < best_evaluation)
      {
         best_move = move_buffer + i;
         best_evaluation = new_position_evaluation;
      }
   }

   if (best != nullptr)
      *best = *best_move;

   return best_evaluation;
}

double SearchingEngine::EvaluatePosition(const Position &position) const
{
   double evaluation = 0.0;
   unsigned char endpoints[8*8*4 + 1]; // f1, r1, promotion1, f2, ..., MAGIC_NUM

   for (unsigned char file = 0; file <= 7; ++file)
   {
      for (unsigned char rank = 0; rank <= 7; ++rank)
      {
         switch (position.m_board.m_squares[file][rank])
         {
         case 'P':
            // when pieces are valued based on square and game phase,
            // pawn controlled squares should be captured
            evaluation += m_parameters[0].second;
            break;
         case 'p':
            evaluation -= m_parameters[0].second;
            break;
         case 'B':
            // at present it is busy work to write endpoints to array, but
            // will come into play when individual squares have different
            // values
            evaluation += m_parameters[1].second;
            evaluation += m_parameters[5].second
               *PopulateBishopEndpoints(position, file, rank, endpoints);
            break;
         case 'b':
            evaluation -= m_parameters[1].second;
            evaluation -= m_parameters[5].second
               *PopulateBishopEndpoints(position, file, rank, endpoints);
            break;
         case 'N':
            evaluation += m_parameters[2].second;
            evaluation += m_parameters[5].second
               *PopulateKnightEndpoints(position, file, rank, endpoints);
            break;
         case 'n':
            evaluation -= m_parameters[2].second;
            evaluation -= m_parameters[5].second
               *PopulateKnightEndpoints(position, file, rank, endpoints);
            break;
         case 'R':
            evaluation += m_parameters[3].second;
            evaluation += m_parameters[5].second
               *PopulateRookEndpoints(position, file, rank, endpoints);
            break;
         case 'r':
            evaluation -= m_parameters[3].second;
            evaluation -= m_parameters[5].second
               *PopulateRookEndpoints(position, file, rank, endpoints);
            break;
         case 'Q':
            evaluation += m_parameters[4].second;
            evaluation += m_parameters[5].second
               *PopulateQueenEndpoints(position, file, rank, endpoints);
            break;
         case 'q':
            evaluation -= m_parameters[4].second;
            evaluation -= m_parameters[5].second
               *PopulateQueenEndpoints(position, file, rank, endpoints);
            break;
         case 'K':
            // castle not considered
            evaluation += m_parameters[5].second
               *PopulateKingEndpoints(position, file, rank, endpoints);
            break;
         case 'k':
            evaluation -= m_parameters[5].second
               *PopulateKingEndpoints(position, file, rank, endpoints);
            break;
         }
      }
   }

   return evaluation;
}

/*virtual*/ int SearchingEngine::Compare(GeneticEngine *first,
   GeneticEngine *second) const
{
   // choose sides

   GeneticEngine *white, *black;

   if (rand() % 2 == 0)
   {
      white = first;
      black = second;
   }
   else
   {
      white = second;
      black = first;
   }

   // shake hands

   white->SetToStartPos();
   black->SetToStartPos();

   // play

   Outcome outcome;
   size_t moves = 1;

   for (;;)
   {
      std::cout << "Move " << moves << std::endl;

      // let white move
      std::string white_move;
      outcome = white->GetBestMove(&white_move);
      if (outcome != OUTCOME_UNDECIDED)
         break;

      // check for win, loose or draw
      Position position;
      white->GetPosition(&position);
      outcome = Engine::GetOutcome(position);
      if (outcome != OUTCOME_UNDECIDED)
         break;

      // let black move
      black->ApplyMove(white_move);
      std::string black_move;
      outcome = black->GetBestMove(&black_move);
      if (outcome != OUTCOME_UNDECIDED)
         break;

      // check for win, loose or draw
      black->GetPosition(&position);
      outcome = Engine::GetOutcome(position);
      if (outcome != OUTCOME_UNDECIDED)
         break;

      // possibly adjudicate a draw after too many moves
      if (moves++ == 200)
      {
         outcome = OUTCOME_DRAW;
         break;
      }

      white->ApplyMove(black_move);
   }

   // sign the score sheet

   if (outcome == OUTCOME_DRAW)
   {
      std::cout << "1/2-1/2" << std::endl;
      return 0;
   }

   if (outcome == OUTCOME_WIN_WHITE)
   {
      std::cout << "1-0" << std::endl;
      return first == white ? 1 : 0;
   }

   std::cout << "0-1" << std::endl;
   return second == white ? 1 : 0;
}
