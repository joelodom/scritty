// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "gtest/gtest.h"
#include "UCIParser.h"

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

   UCIParser::ParseMove("e2e4", &move);

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
