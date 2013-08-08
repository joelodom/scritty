// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "gtest/gtest.h"
#include "UCIParser.h"
#include "Engine.h"
#include "UCIHandler.h"
#include <fstream>

#define GAMES_FILE "C:\\Users\\Joel\\Google Drive\\chess_games_db" \
   "\\3965020games.uci"
#define GAMES_IN_FILE 3965020

using namespace scritty;

// TODO: add huge PGN database and test to check all moves for validity

TEST(test_gtest, test_gtest)
{
   EXPECT_TRUE(true) << "law of noncontradiction failed";
}

namespace scritty // for FRIEND_TEST
{
   TEST(integration_tests, shall_we_play_a_game)
   {
      UCIHandler handler;

      uci_tokens tokens;
      UCIParser::BreakIntoTokens("position startpos moves e2e4", &tokens);
      EXPECT_TRUE(handler.handle_position(tokens));
      EXPECT_EQ('P', handler.m_engine.GetPieceAt("e4"));

      tokens.clear();
      UCIParser::BreakIntoTokens("go movetime 2000", &tokens);
      EXPECT_TRUE(handler.handle_go(tokens));
      //EXPECT_EQ('p', handler.m_engine.GetPieceAt("e7"));

      tokens.clear();
      UCIParser::BreakIntoTokens(
         "position startpos moves e2e4 e7e5 g1f3", &tokens);
      EXPECT_TRUE(handler.handle_position(tokens));
   }

   TEST(integration_tests, play_through_game_database)
   {
      // open the massive games file
      std::fstream in_file(GAMES_FILE);
      ASSERT_TRUE(in_file.good());

      // read file line-by-line
      // use ASSERTs so that we stop on first problem

      std::string line;
      size_t game = 1;
      while (std::getline(in_file, line))
      {
         if (line.size() < 1 || line[0] == '[')
            continue;

         std::cout << "Processing game " << game << " of "
            << GAMES_IN_FILE << "." << std::endl;

         UCIHandler handler;
         uci_tokens tokens;
         UCIParser::BreakIntoTokens("position startpos moves " + line, &tokens);

         ASSERT_GT(tokens.size(), 0);
         ASSERT_TRUE(tokens[tokens.size() - 1] == "0-1"
            || tokens[tokens.size() - 1] == "1-0"
            || tokens[tokens.size() - 1] == "1/2-1/2");
         tokens.pop_back();

         ASSERT_TRUE(handler.handle_position(tokens)) << "Failed on " << line;

         ++game;
      }

      in_file.close();
   }
}

TEST(uciparser_tests, test_break_into_tokens)
{
   uci_tokens tokens;
   UCIParser::BreakIntoTokens("  token1 \ttoken2   token3\r  \n\t", &tokens);

   EXPECT_STREQ("token1", tokens[0].c_str());
   EXPECT_STREQ("token2", tokens[1].c_str());
   EXPECT_STREQ("token3", tokens[2].c_str());
}

TEST(uciparser_tests, test_parse_move)
{
   Move move;

   EXPECT_TRUE(UCIParser::ParseMove("e2e4", &move));

   EXPECT_EQ(4, move.start_file);
   EXPECT_EQ(1, move.start_rank);
   EXPECT_EQ(4, move.end_file);
   EXPECT_EQ(3, move.end_rank);
   EXPECT_EQ(NO_PIECE, move.promotion_piece);

   UCIParser::ParseMove("a7a8Q", &move);

   EXPECT_EQ(0, move.start_file);
   EXPECT_EQ(6, move.start_rank);
   EXPECT_EQ(0, move.end_file);
   EXPECT_EQ(7, move.end_rank);
   EXPECT_EQ('Q', move.promotion_piece);
}

TEST(engine_tests, test_set_to_start_pos)
{
   // implicit
   Engine engine;
   EXPECT_EQ('P', engine.GetPieceAt("e2"));
   EXPECT_EQ('k', engine.GetPieceAt("e8"));
   EXPECT_TRUE(engine.IsWhiteToMove());

   // explicit
   engine.SetToStartPos();
   EXPECT_EQ('P', engine.GetPieceAt("e2"));
   EXPECT_EQ('k', engine.GetPieceAt("e8"));
   EXPECT_TRUE(engine.IsWhiteToMove());
}

TEST(engine_tests, test_apply_move_e2e4)
{
   Engine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_EQ(NO_PIECE, engine.GetPieceAt("e2"));
   EXPECT_EQ('P', engine.GetPieceAt("e4"));
   EXPECT_FALSE(engine.IsWhiteToMove());
}

TEST(engine_tests, test_apply_move_wrong_side)
{
   Engine engine;

   EXPECT_FALSE(engine.ApplyMove("e7e5"));
   EXPECT_EQ('p', engine.GetPieceAt("e7"));
   EXPECT_TRUE(engine.IsWhiteToMove());
}

TEST(engine_tests, test_move_pawn_two_after_first_move)
{
   Engine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));
   EXPECT_FALSE(engine.ApplyMove("e4e6"));
}

TEST(engine_tests, test_move_no_piece)
{
   Engine engine;
   EXPECT_FALSE(engine.ApplyMove("e3e4"));
}

TEST(engine_tests, test_move_same_square)
{
   Engine engine;
   EXPECT_FALSE(engine.ApplyMove("e2e2"));
}

TEST(engine_tests, test_move_off_board)
{
   Engine engine;
   EXPECT_FALSE(engine.ApplyMove("h1i1"));
}

TEST(engine_tests, test_legal_rook_moves)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("a2a4"));
   EXPECT_TRUE(engine.ApplyMove("a7a5"));
   EXPECT_TRUE(engine.ApplyMove("a1a3"));
   EXPECT_TRUE(engine.ApplyMove("b7b5"));
   EXPECT_TRUE(engine.ApplyMove("a3c3"));
   EXPECT_TRUE(engine.ApplyMove("c7c5"));
   EXPECT_TRUE(engine.ApplyMove("c3c5")); // capture
}

TEST(engine_tests, test_blocked_rook)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("a2a4"));
   EXPECT_TRUE(engine.ApplyMove("a7a5"));
   EXPECT_FALSE(engine.ApplyMove("a1a5"));
}

TEST(engine_tests, test_legal_bishop_moves)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));
   EXPECT_TRUE(engine.ApplyMove("f1c4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("c4d5")); // capture
}

TEST(engine_tests, test_legal_knight_move)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));
   EXPECT_TRUE(engine.ApplyMove("g1f3"));
   EXPECT_EQ('N', engine.GetPieceAt("f3"));
}

TEST(engine_tests, illegal_move_test_1)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e3"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("d2d4"));
   EXPECT_FALSE(engine.ApplyMove("f8e3"));
}

TEST(engine_tests, illegal_move_test_2)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("d2d4"));
   EXPECT_FALSE(engine.ApplyMove("f8e7"));
}

TEST(engine_tests, move_test_pawn_capture)
{
   // e2e4 d7d5 b1c3 b7b5 c3d5 c8f5 e4f5
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("b1c3"));
   EXPECT_TRUE(engine.ApplyMove("b7b5"));
   EXPECT_TRUE(engine.ApplyMove("c3d5"));
   EXPECT_TRUE(engine.ApplyMove("c8f5"));
   EXPECT_TRUE(engine.ApplyMove("e4f5")); // pawn capture
}

TEST(engine_tests, illegal_move_test_3)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("b7b5"));
   EXPECT_TRUE(engine.ApplyMove("f1b5"));
   EXPECT_TRUE(engine.ApplyMove("c8b7"));
   EXPECT_TRUE(engine.ApplyMove("g1f3"));
   EXPECT_TRUE(engine.ApplyMove("d8c8"));
   EXPECT_TRUE(engine.ApplyMove("d2d4"));
   EXPECT_TRUE(engine.ApplyMove("a7a5"));
   EXPECT_TRUE(engine.ApplyMove("c1f4"));
   EXPECT_TRUE(engine.ApplyMove("e7e6"));
   EXPECT_TRUE(engine.ApplyMove("b1c3"));
   EXPECT_TRUE(engine.ApplyMove("f8b4"));
   EXPECT_TRUE(engine.ApplyMove("f4d2"));
   EXPECT_TRUE(engine.ApplyMove("f7f6"));
   EXPECT_TRUE(engine.ApplyMove("a2a3"));
   EXPECT_FALSE(engine.ApplyMove("d7d5")); // illegal -- puts king in check
}

TEST(engine_tests, illegal_move_test_4)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("b1c3"));
   EXPECT_FALSE(engine.ApplyMove("a7b6")); // pawn diagonal without capture
}

TEST(engine_tests, illegal_move_test_5)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("b1c3"));
   EXPECT_TRUE(engine.ApplyMove("b7b5"));
   EXPECT_TRUE(engine.ApplyMove("c3b5"));
   EXPECT_TRUE(engine.ApplyMove("c8f5"));
   EXPECT_TRUE(engine.ApplyMove("e4f5"));
   EXPECT_TRUE(engine.ApplyMove("d8d6"));
   EXPECT_TRUE(engine.ApplyMove("b5d6"));
   EXPECT_TRUE(engine.ApplyMove("e7d6"));
   EXPECT_TRUE(engine.ApplyMove("f1b5"));
   EXPECT_TRUE(engine.ApplyMove("c7c6"));
   EXPECT_TRUE(engine.ApplyMove("b5c6"));
   EXPECT_EQ(NO_PIECE, engine.GetPieceAt("d7"));
   EXPECT_FALSE(engine.ApplyMove("d6d7")); // illegal -- backward pawn move
}

TEST(engine_tests, king_move_test)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("f1e2"));
}

TEST(engine_tests, castle_test)
{
   UCIHandler handler;
   uci_tokens tokens;
   UCIParser::BreakIntoTokens(
      "position startpos moves e2e4 d7d5 e4d5 b7b5 f1b5 b8d7 g1f3 f7f6 e1g1",
      &tokens);
   EXPECT_TRUE(handler.handle_position(tokens));
}

TEST(engine_tests, illegal_move_test_6)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("e4d5"));
   EXPECT_FALSE(engine.ApplyMove("e8e5"));
}

TEST(engine_tests, illegal_move_test_7)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("b1c3"));
   EXPECT_TRUE(engine.ApplyMove("d5d4"));
   EXPECT_TRUE(engine.ApplyMove("c3e2"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));
   EXPECT_TRUE(engine.ApplyMove("d2d3"));
   EXPECT_TRUE(engine.ApplyMove("g8f6"));
   EXPECT_TRUE(engine.ApplyMove("g1f3"));
   EXPECT_TRUE(engine.ApplyMove("f8d6"));
}

#error fix failing tests and add a test for en passant, making sure that the piece is removed
