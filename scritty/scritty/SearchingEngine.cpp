// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "SearchingEngine.h"
#include <iostream>
#include "scritty.h"
#include "UCIHandler.h"

using namespace scritty;

SearchingEngine::SearchingEngine() : GeneticEngine()
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
      "Square Control Value", 0.01));
}

SearchingEngine *SearchingEngine::Clone() const
{
   SearchingEngine *clone = new SearchingEngine;
   SCRITTY_ASSERT(clone->m_parameters.size() == m_parameters.size());

   for (size_t i = 0; i < m_parameters.size(); ++i)
      clone->m_parameters[i] = m_parameters[i];

   return clone;
}

Outcome SearchingEngine::GetBestMove(std::string *best) const
{
   // the move buffer for all depths is allocated once for performance
   Move *move_buffer = new Move[MAX_SEARCH_DEPTH*MAX_NUMBER_OF_LEGAL_MOVES];
   Move move, suggestion, *move_ptr;

   // first pass

   move_ptr = &suggestion;

   double evaluation
      = GetBestMove(*m_position, nullptr, FIRST_PASS_SEARCH_DEPTH,
      -DBL_MAX, DBL_MAX, m_position->m_white_to_move, &move_ptr, move_buffer);

   if (move_ptr == nullptr)
   {
      // no moves available, so give up only at this point
      delete[] move_buffer;
      if (evaluation == 0.0)
         return OUTCOME_DRAW;
      return GetOutcome(*m_position);
   }

   // final pass
   move_ptr = &move;
   evaluation = GetBestMove(*m_position, &suggestion, MAX_SEARCH_DEPTH,
      -DBL_MAX, DBL_MAX, m_position->m_white_to_move, &move_ptr, move_buffer);

   SCRITTY_ASSERT(move_ptr != nullptr);
   SCRITTY_ASSERT(move.start_file <= 7 && move.start_rank <= 7
      && move.end_file <= 7 && move.end_rank <= 7);

   delete[] move_buffer;
   move.ToString(best);

   return OUTCOME_UNDECIDED; // don't ever give up
}

double SearchingEngine::GetBestMove(const Position &position,
   const Move *suggestion,
   size_t current_depth, double alpha, double beta, bool maximize,
   Move **best, Move *move_buffer) const
{
   // if best != null, *best must not be null
   // if no move, resets *best to nullptr
   SCRITTY_ASSERT(best == nullptr || *best != nullptr);

   // if this is beyond max depth, just evaluate the position
   if (current_depth == 0)
      return EvaluatePosition(position);

   // for now consider all positions that MAY be claimed as a draw as terminal
   // nodes as the underdog would normally claim a draw
   if (position.MayClaimDraw())
   {
      if (best != nullptr)
         *best = nullptr;
      return 0.0;
   }

   // get all legal moves
   size_t num_moves = ListAllLegalMoves(position, move_buffer);

   // if this is a terminal node, return the value of the outcome

   if (num_moves == 0)
   {
      if (best != nullptr)
         *best = nullptr;

      Outcome outcome = GetOutcome(position);

      if (outcome == OUTCOME_WIN_WHITE)
         return DBL_MAX;
      else if (outcome == OUTCOME_WIN_BLACK)
         return -DBL_MAX;
      else
         return 0.0;
   }

   // algorithm is most efficient when best moves evaluated first

   if (suggestion != nullptr)
   {
      for (size_t i = 1; i < num_moves; ++i)
      {
         if (move_buffer[i] == *suggestion)
         {
            move_buffer[i] = *move_buffer;
            *move_buffer = *suggestion;
            break;
         }
      }
   }

   if (best != nullptr)
      **best = *move_buffer;  // must return something

   // alpha-beta pruning minimax search

   if (maximize)
   {
      for (size_t i = 0; i < num_moves; ++i)
      {
         Position& must_roll_back = const_cast<Position&>(position);
         must_roll_back.ApplyKnownLegalMove(move_buffer[i]);

         double evaluation = GetBestMove(must_roll_back, nullptr,
            current_depth - 1, alpha,
            beta, !maximize, nullptr, move_buffer + MAX_NUMBER_OF_LEGAL_MOVES);

         must_roll_back.RollBackOneMove();

         if (current_depth == MAX_SEARCH_DEPTH)
         {
            std::stringstream ss;
            ss << "score cp " << (int)(100*evaluation) << " ";
            ss << "currmovenumber " << (i + 1);
            UCIHandler::send_info(ss.str());
         }

         if (evaluation > alpha)
         {
            alpha = evaluation; // reassignment of formal parameter intentional
            if (best != nullptr)
               **best = move_buffer[i];
         }

         if (alpha >= beta)
            return alpha;
      }

      return alpha;
   }
   else
   {
      for (size_t i = 0; i < num_moves; ++i)
      {
         Position& must_roll_back = const_cast<Position&>(position);
         must_roll_back.ApplyKnownLegalMove(move_buffer[i]);

         double evaluation = GetBestMove(must_roll_back, nullptr,
            current_depth - 1, alpha,
            beta, !maximize, nullptr, move_buffer + MAX_NUMBER_OF_LEGAL_MOVES);

         must_roll_back.RollBackOneMove();

         if (current_depth == MAX_SEARCH_DEPTH)
         {
            std::stringstream ss;
            ss << "score cp " << (int)(-100*evaluation) << " ";
            ss << "currmovenumber " << (i + 1);
            UCIHandler::send_info(ss.str());
         }

         if (evaluation < beta)
         {
            beta = evaluation; // reassignment of formal parameter intentional
            if (best != nullptr)
               **best = move_buffer[i];
         }

         if (beta <= alpha)
            return beta;
      }

      return beta;
   }
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

   std::cout << "White:" << std::endl;
   white->PrintParameters();

   std::cout << "Black:" << std::endl;
   black->PrintParameters();

   // shake hands

   white->SetToStartPos();
   black->SetToStartPos();

   // play

   Outcome outcome;
   size_t moves = 1;

   for (;;)
   {
      std::cout << moves << ". ";

      // let white move
      std::string white_move;
      outcome = white->GetBestMove(&white_move);
      if (outcome != OUTCOME_UNDECIDED)
         break;
      white->ApplyMove(white_move);
      std::cout << white_move << " ";

      // check for win, loose or draw
      Position position = white->GetPosition();
      outcome = Engine::GetOutcome(position);
      if (outcome != OUTCOME_UNDECIDED)
         break;

      // let black move
      black->ApplyMove(white_move);
      std::string black_move;
      outcome = black->GetBestMove(&black_move);
      if (outcome != OUTCOME_UNDECIDED)
         break;
      black->ApplyMove(black_move);
      std::cout << black_move << std::endl;

      // check for win, loose or draw
      position = black->GetPosition();
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
