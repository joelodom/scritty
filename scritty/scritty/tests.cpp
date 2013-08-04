// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "gtest/gtest.h"
#include "UCIParser.h"
#include "Engine.h"

using namespace scritty;

TEST(test_gtest, test_gtest)
{
   EXPECT_TRUE(true) << "law of noncontradiction failed";
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
   EXPECT_EQ('\0', move.promotion_piece);

   UCIParser::ParseMove("a7a8q", &move);

   EXPECT_EQ(0, move.start_file);
   EXPECT_EQ(6, move.start_rank);
   EXPECT_EQ(0, move.end_file);
   EXPECT_EQ(7, move.end_rank);
   EXPECT_EQ('q', move.promotion_piece);
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
   EXPECT_EQ('\0', engine.GetPieceAt("e2"));
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
