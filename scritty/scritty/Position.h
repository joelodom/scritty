// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_POSITION_H
#define SCRITTY_POSITION_H

#include <string>
#include "scritty.h"

#define NO_PIECE '\0'
#define NO_EN_PASSANT 100

// TODO P3: whittle this down with more thinking
// 16 pieces times 63 squares plus 16 promotion squares times 4 promotion pieces
// plus 4 castles
#define MAX_NUMBER_OF_LEGAL_MOVES (16*63 + 16*4 + 4)

#define MAX_POSITION_CHAIN_LEN 1000 // 500 moves

namespace scritty
{
   class Move
   {
   public:
      unsigned char start_file; // 0 to 7
      unsigned char start_rank; // 0 to 7
      unsigned char end_file;
      unsigned char end_rank;
      char promotion_piece; // NO_PIECE for none

      void ToString(std::string *str);
      bool operator==(const Move &other) const;
      Move& operator=(const Move &rhs);
   };

   class Position
   {
   public:

      // normally use this constructor
      Position(Position *chain, size_t *chain_length) : m_chain(chain),
         m_chain_length(chain_length)
      {
         SetToStartPos();
      }

      // use this constructor for initializing an empty position chain
      Position() {} // note: does zero setup

      ~Position()
      {
      }

      void SetToStartPos();
      void ApplyKnownLegalMove(const Move &move);
      void RollBackOneMove();
      bool operator==(const Position &other) const;
      bool MayClaimDraw() const;
      bool IsWhiteToMove() const { return m_white_to_move; }
      bool IsMoveLegal(const Move &move) const;
      Outcome GetOutcome() const;
      bool IsCheck(bool white) const;
      size_t ListAllLegalMoves(Move *buf = nullptr) const;

      char GetPieceAt(unsigned char file, unsigned char rank) const
      {
         return m_squares[file][rank];
      }

      static inline bool IsOpponentsPiece(char mine, char theirs);

      bool IsRookMoveLegal(const Move &move) const;
      bool IsBishopMoveLegal(const Move &move) const;
      bool IsKnightMoveLegal(const Move &move) const;

      size_t PopulateBishopEndpoints(unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints) const;
      size_t PopulateKnightEndpoints(unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints) const;
      size_t PopulateRookEndpoints(unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints) const;
      size_t PopulateQueenEndpoints(unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints) const;
      size_t PopulateKingEndpoints(unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints) const;

      bool IsAttackingSquare(
         bool white, unsigned char file, unsigned char rank) const;
      bool IsMoveLegal(const Move &move, bool white, bool check_king) const;

   private:

      // this constructor does not copy the position chain deeply, so the
      // object to copy must have a valid position chain
      Position (const Position &to_copy)
         : m_white_to_move(to_copy.m_white_to_move),
         m_white_may_castle_short(to_copy.m_white_may_castle_short),
         m_white_may_castle_long(to_copy.m_white_may_castle_long),
         m_black_may_castle_short(to_copy.m_black_may_castle_short),
         m_black_may_castle_long(to_copy.m_black_may_castle_long),
         m_en_passant_allowed_on(to_copy.m_en_passant_allowed_on),
         m_chain(to_copy.m_chain), m_chain_length(to_copy.m_chain_length)
      {
         memcpy(m_squares, to_copy.m_squares, sizeof(to_copy.m_squares));
      }

      char m_squares[8][8];
      bool m_white_to_move;
      bool m_white_may_castle_short, m_white_may_castle_long;
      bool m_black_may_castle_short, m_black_may_castle_long;
      unsigned char m_en_passant_allowed_on;

      Position *m_chain;
      size_t *m_chain_length;
   };
}

#endif // #ifndef SCRITTY_POSITION_H
