// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_ENGINE_H
#define SCRITTY_ENGINE_H

#include <cstring>

namespace scritty
{
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
