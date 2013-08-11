// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_ENGINE_H
#define SCRITTY_ENGINE_H

#include <cstring>
#include <string>

#define NO_PIECE '\0'
#define NO_EN_PASSANT 8

// TODO: whittle this down with more thinking
// 16 pieces times 63 squares plus 16 promotion squares times 4 promotion pieces
// plus 4 castles
#define MAX_NUMBER_OF_LEGAL_MOVES 16*63 + 16*4 + 4

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
      Position() {}

      Board m_board;
      bool m_white_to_move;
      bool m_white_may_castle_short, m_white_may_castle_long;
      bool m_black_may_castle_short, m_black_may_castle_long;
      unsigned char en_passant_allowed_on;
   };

   class Engine
   {
   public:
      static void WritePositionToStdout(const Position &position);

      // methods not guaranteed to be efficient
      Engine();
      void SetToStartPos();
      bool ApplyMove(const std::string &str); // algebraic notation
      char GetPieceAt(const std::string &square) const; // algebraic notation
      bool IsWhiteToMove() const;
      Outcome GetOutcome() const;
      void GetPosition(Position *position) const;

      // returns a draw outcome if engine is offering a draw
      // returns win for other side on resignation
      // don't call if there are no valid moves
      virtual Outcome GetBestMove(std::string *best) const = 0; // algebraic

      // these methods should be optimized
      static bool IsCheck(
         const Position &position, const char which_king);
      static size_t ListAllLegalMoves(
         const Position &position, Move *buf = nullptr);

   protected:
      static bool IsMoveLegal(
         const Position &position, const Move &move, bool check_king = true);
      static inline bool IsOpponentsPiece(char mine, char theirs);
      static inline bool IsOnBoard(unsigned char file, unsigned char rank);
      static inline bool IsRookMoveLegal(
         const Position &position, const Move &move);
      static inline bool IsBishopMoveLegal(
         const Position &position, const Move &move);
      static inline bool IsKnightMoveLegal(
         const Position &position, const Move &move);
      static void ApplyKnownLegalMoveToPosition(const Move &move,
         Position *position);
      static bool IsOpponentAttackingSquare(
         unsigned char file, unsigned char rank,
         const Position &position);
      static Outcome GetOutcome(const Position &position);

      Position m_position;
   };
}

#endif // #ifndef SCRITTY_ENGINE_H
