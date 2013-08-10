// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "gtest/gtest.h"
#include "UCIParser.h"
#include "Engine.h"
#include "UCIHandler.h"
#include <fstream>
#include "Logger.h"

#define GAMES_FILE "C:\\Users\\Joel\\Google Drive\\chess_games_db" \
   "\\3965020games.uci"
#define GAMES_IN_FILE 3965020
#define MAX_GAMES_TO_PLAY 1

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
      while (game <= MAX_GAMES_TO_PLAY && std::getline(in_file, line))
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

         if (!handler.handle_position(tokens))
         {
            Logger::GetStream() << "Failed game: " << line << std::endl;
            GTEST_FAIL() << "Failed on " << line;
         }

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

TEST(engine_tests, en_passant_test)
{
   Engine engine;

   EXPECT_TRUE(engine.ApplyMove("b1c3"));
   EXPECT_TRUE(engine.ApplyMove("b7b6"));
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("c8b7"));
   EXPECT_TRUE(engine.ApplyMove("g1f3"));
   EXPECT_TRUE(engine.ApplyMove("e7e6"));
   EXPECT_TRUE(engine.ApplyMove("d2d4"));
   EXPECT_TRUE(engine.ApplyMove("f8b4"));
   EXPECT_TRUE(engine.ApplyMove("f1d3"));
   EXPECT_TRUE(engine.ApplyMove("c7c5"));
   EXPECT_TRUE(engine.ApplyMove("e1g1"));
   EXPECT_TRUE(engine.ApplyMove("c5d4"));
   EXPECT_TRUE(engine.ApplyMove("c3e2"));
   EXPECT_TRUE(engine.ApplyMove("g8f6"));
   EXPECT_TRUE(engine.ApplyMove("c1g5"));
   EXPECT_TRUE(engine.ApplyMove("b4e7"));
   EXPECT_TRUE(engine.ApplyMove("g5f6"));
   EXPECT_TRUE(engine.ApplyMove("e7f6"));
   EXPECT_TRUE(engine.ApplyMove("e2d4"));
   EXPECT_TRUE(engine.ApplyMove("e8g8"));
   EXPECT_TRUE(engine.ApplyMove("e4e5"));
   EXPECT_TRUE(engine.ApplyMove("f6e7"));
   EXPECT_TRUE(engine.ApplyMove("d4b5"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("e5d6"));

   EXPECT_EQ(NO_PIECE, engine.GetPieceAt("d5"));

   EXPECT_TRUE(engine.ApplyMove("e7f6"));
   EXPECT_TRUE(engine.ApplyMove("c2c3"));
   EXPECT_TRUE(engine.ApplyMove("b8d7"));
   EXPECT_TRUE(engine.ApplyMove("f1e1"));
   EXPECT_TRUE(engine.ApplyMove("a7a6"));
   EXPECT_TRUE(engine.ApplyMove("b5d4"));
   EXPECT_TRUE(engine.ApplyMove("d7c5"));
   EXPECT_TRUE(engine.ApplyMove("d4b3"));
   EXPECT_TRUE(engine.ApplyMove("a8c8"));
   EXPECT_TRUE(engine.ApplyMove("b3c5"));
   EXPECT_TRUE(engine.ApplyMove("c8c5"));
   EXPECT_TRUE(engine.ApplyMove("d1c2"));
   EXPECT_TRUE(engine.ApplyMove("g7g6"));
   EXPECT_TRUE(engine.ApplyMove("a1d1"));
   EXPECT_TRUE(engine.ApplyMove("b7f3"));
   EXPECT_TRUE(engine.ApplyMove("g2f3"));
   EXPECT_TRUE(engine.ApplyMove("c5g5"));
   EXPECT_TRUE(engine.ApplyMove("g1h1"));
   EXPECT_TRUE(engine.ApplyMove("f6e5"));
   EXPECT_TRUE(engine.ApplyMove("c2d2"));
   EXPECT_TRUE(engine.ApplyMove("g5h5"));
   EXPECT_TRUE(engine.ApplyMove("f3f4"));
   EXPECT_TRUE(engine.ApplyMove("d8h4"));
   EXPECT_TRUE(engine.ApplyMove("f2f3"));
   EXPECT_TRUE(engine.ApplyMove("e5f4"));
   EXPECT_TRUE(engine.ApplyMove("d2g2"));
   EXPECT_TRUE(engine.ApplyMove("f4h2"));
   EXPECT_TRUE(engine.ApplyMove("e1e4"));
   EXPECT_TRUE(engine.ApplyMove("h2f4"));
   EXPECT_TRUE(engine.ApplyMove("h1g1"));
   EXPECT_TRUE(engine.ApplyMove("h5g5"));
   EXPECT_TRUE(engine.ApplyMove("g2g5"));
   EXPECT_TRUE(engine.ApplyMove("h4g5"));
   EXPECT_TRUE(engine.ApplyMove("g1f1"));
   EXPECT_TRUE(engine.ApplyMove("f4d6"));
   EXPECT_TRUE(engine.ApplyMove("e4g4"));
   EXPECT_TRUE(engine.ApplyMove("g5h5"));
   EXPECT_TRUE(engine.ApplyMove("d3a6"));
   EXPECT_TRUE(engine.ApplyMove("d6c5"));
   EXPECT_TRUE(engine.ApplyMove("f1g2"));
   EXPECT_TRUE(engine.ApplyMove("f7f5"));
   EXPECT_TRUE(engine.ApplyMove("g4g3"));
   EXPECT_TRUE(engine.ApplyMove("h5h4"));
   EXPECT_TRUE(engine.ApplyMove("d1h1"));
   EXPECT_TRUE(engine.ApplyMove("h4a4"));
   EXPECT_TRUE(engine.ApplyMove("g3h3"));
   EXPECT_TRUE(engine.ApplyMove("a4c2"));
   EXPECT_TRUE(engine.ApplyMove("g2g3"));
   EXPECT_TRUE(engine.ApplyMove("f5f4"));
}

TEST(engine_tests, failed_game_test)
{
   Engine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("b1c3"));
   EXPECT_TRUE(engine.ApplyMove("d5e4"));
   EXPECT_TRUE(engine.ApplyMove("c3e2"));
   EXPECT_TRUE(engine.ApplyMove("g8f6"));
   EXPECT_TRUE(engine.ApplyMove("e2g3"));
   EXPECT_TRUE(engine.ApplyMove("b7b6"));
   EXPECT_TRUE(engine.ApplyMove("f1c4"));
   EXPECT_TRUE(engine.ApplyMove("c8b7"));
   EXPECT_TRUE(engine.ApplyMove("g1h3"));
   EXPECT_TRUE(engine.ApplyMove("e7e6"));
   EXPECT_TRUE(engine.ApplyMove("g3h5"));
   EXPECT_TRUE(engine.ApplyMove("d8d4"));
   EXPECT_TRUE(engine.ApplyMove("c4b3"));
   EXPECT_TRUE(engine.ApplyMove("b8d7"));
   EXPECT_TRUE(engine.ApplyMove("d1e2"));
   EXPECT_TRUE(engine.ApplyMove("d7c5"));
   EXPECT_TRUE(engine.ApplyMove("h5f6"));
   EXPECT_TRUE(engine.ApplyMove("d4f6"));
   EXPECT_TRUE(engine.ApplyMove("e1g1"));
   EXPECT_TRUE(engine.ApplyMove("f8d6"));
   EXPECT_TRUE(engine.ApplyMove("e2g4"));
   EXPECT_TRUE(engine.ApplyMove("h7h6"));
   EXPECT_TRUE(engine.ApplyMove("b3c4"));
   EXPECT_TRUE(engine.ApplyMove("e8c8"));
   EXPECT_TRUE(engine.ApplyMove("d2d3"));
   EXPECT_TRUE(engine.ApplyMove("g7g5"));
   EXPECT_TRUE(engine.ApplyMove("c1e3"));
   EXPECT_TRUE(engine.ApplyMove("f6e5"));
   EXPECT_TRUE(engine.ApplyMove("g2g3"));
   EXPECT_TRUE(engine.ApplyMove("f7f5"));
   EXPECT_TRUE(engine.ApplyMove("d3d4"));
   EXPECT_TRUE(engine.ApplyMove("e5f6"));
   EXPECT_TRUE(engine.ApplyMove("g4h5"));
   EXPECT_TRUE(engine.ApplyMove("c5d7"));
   EXPECT_TRUE(engine.ApplyMove("f1d1"));
   EXPECT_TRUE(engine.ApplyMove("f5f4"));
   EXPECT_TRUE(engine.ApplyMove("g3f4"));
   EXPECT_TRUE(engine.ApplyMove("g5f4"));
   EXPECT_TRUE(engine.ApplyMove("e3d2"));
   EXPECT_TRUE(engine.ApplyMove("e4e3"));
   EXPECT_TRUE(engine.ApplyMove("d2e1"));
   EXPECT_TRUE(engine.ApplyMove("f6g7"));
   EXPECT_TRUE(engine.ApplyMove("g1f1"));
   EXPECT_TRUE(engine.ApplyMove("f4f3"));
   EXPECT_TRUE(engine.ApplyMove("e1c3"));
   EXPECT_TRUE(engine.ApplyMove("g7g2"));
   EXPECT_TRUE(engine.ApplyMove("f1e1"));
   EXPECT_TRUE(engine.ApplyMove("g2h1"));
   EXPECT_TRUE(engine.ApplyMove("c4f1"));
   EXPECT_TRUE(engine.ApplyMove("e3e2"));
   EXPECT_TRUE(engine.ApplyMove("e1d2"));

   EXPECT_FALSE(engine.IsWhiteToMove());
   EXPECT_TRUE(engine.ApplyMove("e2d1Q"));

   EXPECT_EQ('q', engine.GetPieceAt("d1"));
   
   EXPECT_TRUE(engine.ApplyMove("a1d1"));
}

TEST(engine_tests, illegal_move_test_8)
{
   Engine engine;
   EXPECT_TRUE(engine.ApplyMove("a2a3"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("g2g4"));
   EXPECT_TRUE(engine.ApplyMove("b7b5"));
   EXPECT_TRUE(engine.ApplyMove("f1g2"));
   EXPECT_TRUE(engine.ApplyMove("c8f5"));
   EXPECT_TRUE(engine.ApplyMove("c2c3"));
   EXPECT_TRUE(engine.ApplyMove("d8d6"));
   EXPECT_TRUE(engine.ApplyMove("d1a4"));
   EXPECT_TRUE(engine.ApplyMove("b5a4"));
   EXPECT_TRUE(engine.ApplyMove("c3c4"));
   EXPECT_TRUE(engine.ApplyMove("d6e5"));
   EXPECT_TRUE(engine.ApplyMove("b2b3"));
   EXPECT_FALSE(engine.ApplyMove("c7c8"));
}
