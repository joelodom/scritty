// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "Engine.h"

using namespace scritty;

void Board::SetToStartPos()
{
   memcpy(m_squares[7], "rkbqkbkr", 8);
   memcpy(m_squares[6], "pppppppp", 8);
   memset(m_squares[2], 0x00, 32);
   memcpy(m_squares[1], "PPPPPPPP", 8);
   memcpy(m_squares[0], "RKBQKBKR", 8);
}

void Position::SetToStartPos()
{
   m_board.SetToStartPos();
   // TODO: reset all other position stuff
}

Engine::Engine()
{
   m_position.SetToStartPos();
}

void Engine::SetToStartPos()
{
   m_position.SetToStartPos();
}
