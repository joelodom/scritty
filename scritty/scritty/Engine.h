// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_ENGINE_H
#define SCRITTY_ENGINE_H

#include "Position.h"
#include "scritty.h"

namespace scritty
{
   class Engine
   {
   public:
      Engine() : m_position_chain(new Position[MAX_POSITION_CHAIN_LEN]),
            m_position_chain_length(new size_t),
            m_position_table(new PositionTable)
      {
         // instantiate a position chain and set up the starting position
         m_position = new Position(
            m_position_chain, m_position_chain_length, m_position_table);
      }

      ~Engine() 
      {
         SCRITTY_ASSERT(m_position != nullptr);
         delete m_position;
         m_position = nullptr;

         SCRITTY_ASSERT(m_position_chain_length != nullptr);
         delete m_position_chain_length;
         m_position_chain_length = nullptr;

         SCRITTY_ASSERT(m_position_chain != nullptr);
         delete[] m_position_chain;
         m_position_chain = nullptr;

         SCRITTY_ASSERT(m_position_table != nullptr);
         delete m_position_table;
         m_position_table = nullptr;
      }

      void StartNewGame() { m_position->SetToStartPos(); }
      bool ApplyMove(const std::string &str); // algebraic notation
      char GetPieceAt(const std::string &square) const; // algebraic notation
      bool IsWhiteToMove() const;
      Outcome GetOutcome() const;
      const Position &GetPosition() const;

      // returns a draw outcome if engine is *offering* a draw
      // returns win for other side on resignation
      // don't call if there are no valid moves
      virtual Outcome GetBestMove(std::string *best) const = 0; // algebraic

   protected:
      Position *m_position;
      Position *m_position_chain;
      size_t *m_position_chain_length;
      PositionTable *m_position_table;

   private:
      // copy disallowed because too easy to goof position chain
      Engine(const Engine &);
   };
}

#endif // #ifndef SCRITTY_ENGINE_H
