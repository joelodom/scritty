// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_ENGINE_H
#define SCRITTY_ENGINE_H

#include <cstring>

namespace scritty
{
   struct Move
   {
      unsigned char start_file; // 0 to 7
      unsigned char start_rank; // 0 to 7
      unsigned char end_file;
      unsigned char end_rank;
      char promotion_piece; // '\0' for none
   };

   class Board
   {
   public:
      void SetToStartPos();

   private:
      char m_squares[8][8];
   };

   class Position
   {
   public:
      void SetToStartPos();

   private:
      Board m_board;
   };

   class Engine
   {
   public:
      Engine();
      void SetToStartPos();

   private:
      Position m_position;
   };
}

#endif // #ifndef SCRITTY_ENGINE_H
