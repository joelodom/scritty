// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_ENGINE_H
#define SCRITTY_ENGINE_H

#include <cstring>
#include <string>

#define NO_PIECE '\0'
#define NO_EN_PASSANT 100

// TODO: whittle this down with more thinking
// 16 pieces times 63 squares plus 16 promotion squares times 4 promotion pieces
// plus 4 castles
#define MAX_NUMBER_OF_LEGAL_MOVES (16*63 + 16*4 + 4)

#define MAX_POSITION_CHAIN_LEN 1000 // 500 moves

namespace scritty
{
   enum Outcome
   {
      OUTCOME_UNDECIDED,
      OUTCOME_WIN_WHITE,
      OUTCOME_WIN_BLACK,
      OUTCOME_DRAW
   };

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

   class Board
   {
   public:
      Board() {}
      char m_squares[8][8];
   };

   class Position
   {
   public:
      Position(Position *chain, size_t *chain_length) : m_chain(chain),
         m_chain_length(chain_length)
      {
         SetToStartPos();
      }

      Position() {} // note: does zero setup

      Position (const Position &to_copy)
         : m_white_to_move(to_copy.m_white_to_move),
         m_white_may_castle_short(to_copy.m_white_may_castle_short),
         m_white_may_castle_long(to_copy.m_white_may_castle_long),
         m_black_may_castle_short(to_copy.m_black_may_castle_short),
         m_black_may_castle_long(to_copy.m_black_may_castle_long),
         m_en_passant_allowed_on(to_copy.m_en_passant_allowed_on),
         m_chain(to_copy.m_chain), m_chain_length(to_copy.m_chain_length)
      {
         // doesn't copy chain deeply
         memcpy(m_board.m_squares, to_copy.m_board.m_squares,
            sizeof(to_copy.m_board.m_squares));
      }

      ~Position()
      {
      }

      Board m_board;
      bool m_white_to_move;
      bool m_white_may_castle_short, m_white_may_castle_long;
      bool m_black_may_castle_short, m_black_may_castle_long;
      unsigned char m_en_passant_allowed_on;

      void SetToStartPos();
      void ApplyKnownLegalMove(const Move &move);
      void RollBackOneMove();
      bool operator==(const Position &other) const;
      bool MayClaimDraw() const;

   private:
      Position *m_chain;
      size_t *m_chain_length;
   };

   class Engine
   {
   public:
      Engine()
      {
         // instantiate a position chain and set up the starting position
         m_position_chain = new Position[MAX_POSITION_CHAIN_LEN];
         m_position_chain_length = new size_t;
         m_position = new Position(m_position_chain, m_position_chain_length);
      }

      ~Engine() 
      {
         // TODO: fix leaks
         //delete m_position;
         //delete[] m_position_chain;
         //delete m_position_chain_length;
      }

      static void WritePositionToStdout(const Position &position);

      // methods not guaranteed to be efficient
      void SetToStartPos() { m_position->SetToStartPos(); }
      bool ApplyMove(const std::string &str); // algebraic notation
      char GetPieceAt(const std::string &square) const; // algebraic notation
      bool IsWhiteToMove() const;
      Outcome GetOutcome() const;
      const Position & GetPosition() const;

      // returns a draw outcome if engine is offering a draw
      // returns win for other side on resignation
      // don't call if there are no valid moves
      virtual Outcome GetBestMove(std::string *best) const = 0; // algebraic

      // these methods should be optimized
      static bool IsCheck(
         const Position &position, const char which_king);
      static size_t ListAllLegalMoves(
         const Position &position, Move *buf = nullptr);
      static Outcome GetOutcome(const Position &position);

   protected:
      static bool IsMoveLegal(
         const Position &position, const Move &move, bool check_king = true);
      static inline bool IsOpponentsPiece(char mine, char theirs);
      static inline bool IsRookMoveLegal(
         const Position &position, const Move &move);
      static inline bool IsBishopMoveLegal(
         const Position &position, const Move &move);
      static inline bool IsKnightMoveLegal(
         const Position &position, const Move &move);
      static bool IsAttackingSquare(bool white,
         unsigned char file, unsigned char rank,
         const Position &position);
      static size_t PopulateBishopEndpoints(const Position &position,
         unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints);
      static size_t PopulateKnightEndpoints(const Position &position,
         unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints);
      static size_t PopulateRookEndpoints(const Position &position,
         unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints);
      static size_t PopulateQueenEndpoints(const Position &position,
         unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints);
      static size_t PopulateKingEndpoints(const Position &position,
         unsigned char start_file,
         unsigned char start_rank, unsigned char *endpoints);

      Position *m_position;
      Position *m_position_chain;
      size_t *m_position_chain_length;

   private:
      Engine (const Engine &); // copy disallowed
   };
}

#endif // #ifndef SCRITTY_ENGINE_H
