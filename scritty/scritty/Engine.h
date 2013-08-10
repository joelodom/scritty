// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_ENGINE_H
#define SCRITTY_ENGINE_H

#include <cstring>
#include <string>

#define NO_PIECE '\0'
#define NO_EN_PASSANT 8

namespace scritty
{
   struct Move
   {
      unsigned char start_file; // 0 to 7
      unsigned char start_rank; // 0 to 7
      unsigned char end_file;
      unsigned char end_rank;
      char promotion_piece; // NO_PIECE for none
   };

   class Board
   {
      friend class Engine;
      friend class Position;

   private:
      Board() {}
      char m_squares[8][8];
   };

   class Position
   {
      friend class Engine;

   private:
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

      Engine();
      void SetToStartPos();
      bool ApplyMove(const std::string &str); // algebraic notation
      char GetPieceAt(const std::string &square) const; // algebraic notation
      bool IsWhiteToMove() const;
      void GetBestMove(std::string *best); // algebraic notation

   private:
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

      Position m_position;
   };
}

#endif // #ifndef SCRITTY_ENGINE_H
