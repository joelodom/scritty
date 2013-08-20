// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "UCIParser.h"
#include "RandomEngine.h"
#include "UCIHandler.h"
#include <fstream>
#include "Logger.h"
#include "scritty.h"
#include "SearchingEngine.h"

#define GAMES_FILE "C:\\Users\\Joel\\Google Drive\\chess_games_db" \
   "\\3965020games.uci"
#define GAMES_IN_FILE 3965020
#define MAX_GAMES_TO_PLAY 50

using namespace scritty;

/*static*/ bool ScrittyTestEnvironment::s_tests_were_run = false;
/*static*/ _CrtMemState ScrittyTestEnvironment::s_mem_state;

TEST(test_gtest, test_gtest)
{
   EXPECT_TRUE(true) << "law of noncontradiction failed";
}

namespace scritty // for FRIEND_TEST
{
   TEST(integration_tests, shall_we_play_a_game)
   {
      RandomEngine engine;
      UCIHandler handler(&engine);

      uci_tokens tokens;
      UCIParser::BreakIntoTokens("position startpos moves e2e4", &tokens);
      EXPECT_TRUE(handler.handle_position(tokens));
      EXPECT_EQ('P', engine.GetPieceAt("e4"));

      tokens.clear();
      UCIParser::BreakIntoTokens("go movetime 2000", &tokens);
      EXPECT_TRUE(handler.handle_go(tokens));
      //EXPECT_EQ('p', engine.GetPieceAt("e7"));

      tokens.clear();
      UCIParser::BreakIntoTokens(
         "position startpos moves e2e4 e7e5 g1f3", &tokens);
      EXPECT_TRUE(handler.handle_position(tokens));
   }

   TEST(integration_tests, DISABLED_play_through_game_database)
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

         RandomEngine engine;
         UCIHandler handler(&engine);
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
            FAIL() << "Failed on " << line;
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
   RandomEngine engine;
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
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_EQ(NO_PIECE, engine.GetPieceAt("e2"));
   EXPECT_EQ('P', engine.GetPieceAt("e4"));
   EXPECT_FALSE(engine.IsWhiteToMove());
}

TEST(engine_tests, test_apply_move_wrong_side)
{
   RandomEngine engine;

   EXPECT_FALSE(engine.ApplyMove("e7e5"));
   EXPECT_EQ('p', engine.GetPieceAt("e7"));
   EXPECT_TRUE(engine.IsWhiteToMove());
}

TEST(engine_tests, test_move_pawn_two_after_first_move)
{
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));
   EXPECT_FALSE(engine.ApplyMove("e4e6"));
}

TEST(engine_tests, test_move_no_piece)
{
   RandomEngine engine;
   EXPECT_FALSE(engine.ApplyMove("e3e4"));
}

TEST(engine_tests, test_move_same_square)
{
   RandomEngine engine;
   EXPECT_FALSE(engine.ApplyMove("e2e2"));
}

TEST(engine_tests, test_move_off_board)
{
   RandomEngine engine;
   EXPECT_FALSE(engine.ApplyMove("h1i1"));
}

TEST(engine_tests, test_legal_rook_moves)
{
   RandomEngine engine;

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
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("a2a4"));
   EXPECT_TRUE(engine.ApplyMove("a7a5"));
   EXPECT_FALSE(engine.ApplyMove("a1a5"));
}

TEST(engine_tests, test_legal_bishop_moves)
{
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));
   EXPECT_TRUE(engine.ApplyMove("f1c4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("c4d5")); // capture
}

TEST(engine_tests, test_legal_knight_move)
{
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));
   EXPECT_TRUE(engine.ApplyMove("g1f3"));
   EXPECT_EQ('N', engine.GetPieceAt("f3"));
}

TEST(engine_tests, illegal_move_test_1)
{
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e3"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("d2d4"));
   EXPECT_FALSE(engine.ApplyMove("f8e3"));
}

TEST(engine_tests, illegal_move_test_2)
{
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("d2d4"));
   EXPECT_FALSE(engine.ApplyMove("f8e7"));
}

TEST(engine_tests, move_test_pawn_capture)
{
   RandomEngine engine;

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
   RandomEngine engine;

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
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("b1c3"));
   EXPECT_FALSE(engine.ApplyMove("a7b6")); // pawn diagonal without capture
}

TEST(engine_tests, illegal_move_test_5)
{
   RandomEngine engine;

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
   RandomEngine engine;
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("f1e2"));
}

TEST(engine_tests, castle_test)
{
   RandomEngine engine;
   UCIHandler handler(&engine);
   uci_tokens tokens;
   UCIParser::BreakIntoTokens(
      "position startpos moves e2e4 d7d5 e4d5 b7b5 f1b5 b8d7 g1f3 f7f6 e1g1",
      &tokens);
   EXPECT_TRUE(handler.handle_position(tokens));
}

TEST(engine_tests, illegal_move_test_6)
{
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("d7d5"));
   EXPECT_TRUE(engine.ApplyMove("e4d5"));
   EXPECT_FALSE(engine.ApplyMove("e8e5"));
}

TEST(engine_tests, illegal_move_test_7)
{
   RandomEngine engine;

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
   RandomEngine engine;

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
   RandomEngine engine;

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
   RandomEngine engine;

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

TEST(engine_tests, illegal_move_test_9)
{
   RandomEngine engine;

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
   EXPECT_TRUE(engine.ApplyMove("e5e6"));
   EXPECT_TRUE(engine.ApplyMove("b3a4"));
   EXPECT_TRUE(engine.ApplyMove("a7a5"));
   EXPECT_TRUE(engine.ApplyMove("g2d5"));
   EXPECT_TRUE(engine.ApplyMove("e6b6"));
   EXPECT_TRUE(engine.ApplyMove("f2f4"));
   EXPECT_TRUE(engine.ApplyMove("b6b3"));
   EXPECT_TRUE(engine.ApplyMove("d5g2"));
   EXPECT_TRUE(engine.ApplyMove("e7e6"));
   EXPECT_TRUE(engine.ApplyMove("g1f3"));
   EXPECT_TRUE(engine.ApplyMove("b3c4"));
   EXPECT_TRUE(engine.ApplyMove("f3g5"));
   EXPECT_TRUE(engine.ApplyMove("c4d3"));
   EXPECT_TRUE(engine.ApplyMove("g2h3"));
   EXPECT_TRUE(engine.ApplyMove("f8b4"));
   EXPECT_TRUE(engine.ApplyMove("g4f5"));
   EXPECT_TRUE(engine.ApplyMove("d3b1"));
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("b1b2"));
   EXPECT_TRUE(engine.ApplyMove("e1g1"));
   EXPECT_TRUE(engine.ApplyMove("b8d7"));
   EXPECT_TRUE(engine.ApplyMove("g5f7"));
   EXPECT_TRUE(engine.ApplyMove("a8c8"));
   EXPECT_TRUE(engine.ApplyMove("f7d8"));
   EXPECT_TRUE(engine.ApplyMove("b2c3"));
   EXPECT_TRUE(engine.ApplyMove("f1d1"));
   EXPECT_TRUE(engine.ApplyMove("d7f8"));
   EXPECT_TRUE(engine.ApplyMove("d2d4"));
   EXPECT_TRUE(engine.ApplyMove("b4a3"));
   EXPECT_TRUE(engine.ApplyMove("f5f6"));
   EXPECT_TRUE(engine.ApplyMove("c3b2"));
   EXPECT_TRUE(engine.ApplyMove("d8e6"));
   EXPECT_TRUE(engine.ApplyMove("b2d2"));
   EXPECT_TRUE(engine.ApplyMove("g1h1"));
   EXPECT_TRUE(engine.ApplyMove("d2c1"));
   EXPECT_TRUE(engine.ApplyMove("a1a2"));
   EXPECT_TRUE(engine.ApplyMove("c1c6"));
   EXPECT_TRUE(engine.ApplyMove("e6f8"));
   EXPECT_TRUE(engine.ApplyMove("c8a8"));
   EXPECT_TRUE(engine.ApplyMove("d1e1"));
   EXPECT_TRUE(engine.ApplyMove("g7f6"));
   EXPECT_TRUE(engine.ApplyMove("e1g1"));
   EXPECT_TRUE(engine.ApplyMove("a8a6"));
   EXPECT_TRUE(engine.ApplyMove("g1f1"));
   EXPECT_TRUE(engine.ApplyMove("c6e6"));
   EXPECT_TRUE(engine.ApplyMove("a2f2"));
   EXPECT_TRUE(engine.ApplyMove("a6a8"));
   EXPECT_TRUE(engine.ApplyMove("d4d5"));
   EXPECT_TRUE(engine.ApplyMove("e6f5"));
   EXPECT_TRUE(engine.ApplyMove("f8e6"));
   EXPECT_TRUE(engine.ApplyMove("f5g5"));
   EXPECT_TRUE(engine.ApplyMove("e6d4"));
   EXPECT_TRUE(engine.ApplyMove("e8e7"));
   EXPECT_TRUE(engine.ApplyMove("h3g4"));
   EXPECT_TRUE(engine.ApplyMove("g5f5"));
   EXPECT_TRUE(engine.ApplyMove("d4e6"));
   EXPECT_TRUE(engine.ApplyMove("g8h6"));
   EXPECT_TRUE(engine.ApplyMove("e6f8"));
   EXPECT_TRUE(engine.ApplyMove("h6g4"));
   EXPECT_TRUE(engine.ApplyMove("f8g6"));

   EXPECT_FALSE(engine.ApplyMove("g4f2")); // leaves king in check
}

TEST(engine_tests, test_failed_promote)
{
   RandomEngine engine;

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
   EXPECT_TRUE(engine.ApplyMove("e5e6"));
   EXPECT_TRUE(engine.ApplyMove("b3a4"));
   EXPECT_TRUE(engine.ApplyMove("a7a5"));
   EXPECT_TRUE(engine.ApplyMove("g2d5"));
   EXPECT_TRUE(engine.ApplyMove("e6b6"));
   EXPECT_TRUE(engine.ApplyMove("f2f4"));
   EXPECT_TRUE(engine.ApplyMove("b6b3"));
   EXPECT_TRUE(engine.ApplyMove("d5g2"));
   EXPECT_TRUE(engine.ApplyMove("e7e6"));
   EXPECT_TRUE(engine.ApplyMove("g1f3"));
   EXPECT_TRUE(engine.ApplyMove("b3c4"));
   EXPECT_TRUE(engine.ApplyMove("f3g5"));
   EXPECT_TRUE(engine.ApplyMove("c4d3"));
   EXPECT_TRUE(engine.ApplyMove("g2h3"));
   EXPECT_TRUE(engine.ApplyMove("f8b4"));
   EXPECT_TRUE(engine.ApplyMove("g4f5"));
   EXPECT_TRUE(engine.ApplyMove("d3b1"));
   EXPECT_TRUE(engine.ApplyMove("e2e4"));
   EXPECT_TRUE(engine.ApplyMove("b1b2"));
   EXPECT_TRUE(engine.ApplyMove("e1g1"));
   EXPECT_TRUE(engine.ApplyMove("b8d7"));
   EXPECT_TRUE(engine.ApplyMove("g5f7"));
   EXPECT_TRUE(engine.ApplyMove("a8c8"));
   EXPECT_TRUE(engine.ApplyMove("f7d8"));
   EXPECT_TRUE(engine.ApplyMove("b2c3"));
   EXPECT_TRUE(engine.ApplyMove("f1d1"));
   EXPECT_TRUE(engine.ApplyMove("d7f8"));
   EXPECT_TRUE(engine.ApplyMove("d2d4"));
   EXPECT_TRUE(engine.ApplyMove("b4a3"));
   EXPECT_TRUE(engine.ApplyMove("f5f6"));
   EXPECT_TRUE(engine.ApplyMove("c3b2"));
   EXPECT_TRUE(engine.ApplyMove("d8e6"));
   EXPECT_TRUE(engine.ApplyMove("b2d2"));
   EXPECT_TRUE(engine.ApplyMove("g1h1"));
   EXPECT_TRUE(engine.ApplyMove("d2c1"));
   EXPECT_TRUE(engine.ApplyMove("a1a2"));
   EXPECT_TRUE(engine.ApplyMove("c1c6"));
   EXPECT_TRUE(engine.ApplyMove("e6f8"));
   EXPECT_TRUE(engine.ApplyMove("c8a8"));
   EXPECT_TRUE(engine.ApplyMove("d1e1"));
   EXPECT_TRUE(engine.ApplyMove("g7f6"));
   EXPECT_TRUE(engine.ApplyMove("e1g1"));
   EXPECT_TRUE(engine.ApplyMove("a8a6"));
   EXPECT_TRUE(engine.ApplyMove("g1f1"));
   EXPECT_TRUE(engine.ApplyMove("c6e6"));
   EXPECT_TRUE(engine.ApplyMove("a2f2"));
   EXPECT_TRUE(engine.ApplyMove("a6a8"));
   EXPECT_TRUE(engine.ApplyMove("d4d5"));
   EXPECT_TRUE(engine.ApplyMove("e6f5"));
   EXPECT_TRUE(engine.ApplyMove("f8e6"));
   EXPECT_TRUE(engine.ApplyMove("f5g5"));
   EXPECT_TRUE(engine.ApplyMove("e6d4"));
   EXPECT_TRUE(engine.ApplyMove("e8e7"));
   EXPECT_TRUE(engine.ApplyMove("h3g4"));
   EXPECT_TRUE(engine.ApplyMove("g5f5"));
   EXPECT_TRUE(engine.ApplyMove("d4e6"));
   EXPECT_TRUE(engine.ApplyMove("g8h6"));
   EXPECT_TRUE(engine.ApplyMove("e6f8"));
   EXPECT_TRUE(engine.ApplyMove("h6g4"));
   EXPECT_TRUE(engine.ApplyMove("f8g6"));
   EXPECT_TRUE(engine.ApplyMove("e7d6"));
   EXPECT_TRUE(engine.ApplyMove("h2h4"));
   EXPECT_TRUE(engine.ApplyMove("h8c8"));
   EXPECT_TRUE(engine.ApplyMove("e4e5"));
   EXPECT_TRUE(engine.ApplyMove("d6c5"));
   EXPECT_TRUE(engine.ApplyMove("f2e2"));
   EXPECT_TRUE(engine.ApplyMove("h7h6"));
   EXPECT_TRUE(engine.ApplyMove("d5d6"));
   EXPECT_TRUE(engine.ApplyMove("c8f8"));
   EXPECT_TRUE(engine.ApplyMove("e2d2"));
   EXPECT_TRUE(engine.ApplyMove("f5d3"));
   EXPECT_TRUE(engine.ApplyMove("g6f8"));
   EXPECT_TRUE(engine.ApplyMove("a8a6"));
   EXPECT_TRUE(engine.ApplyMove("d2b2"));
   EXPECT_TRUE(engine.ApplyMove("d3e4"));
   EXPECT_TRUE(engine.ApplyMove("h1g1"));
   EXPECT_TRUE(engine.ApplyMove("e4f3"));
   EXPECT_TRUE(engine.ApplyMove("f1b1"));
   EXPECT_TRUE(engine.ApplyMove("g4f2"));
   EXPECT_TRUE(engine.ApplyMove("h4h5"));
   EXPECT_TRUE(engine.ApplyMove("c5d4"));
   EXPECT_TRUE(engine.ApplyMove("b2b5"));
   EXPECT_TRUE(engine.ApplyMove("f2h3"));
   EXPECT_TRUE(engine.ApplyMove("g1h2"));
   EXPECT_TRUE(engine.ApplyMove("a6c6"));
   EXPECT_TRUE(engine.ApplyMove("b1b2"));
   EXPECT_TRUE(engine.ApplyMove("f3c3"));
   EXPECT_TRUE(engine.ApplyMove("b5b6"));
   EXPECT_TRUE(engine.ApplyMove("c3b3"));
   EXPECT_TRUE(engine.ApplyMove("e5f6"));
   EXPECT_TRUE(engine.ApplyMove("d4c3"));
   EXPECT_TRUE(engine.ApplyMove("d6d7"));
   EXPECT_TRUE(engine.ApplyMove("c3d4"));
   EXPECT_TRUE(engine.ApplyMove("f8g6"));
   EXPECT_TRUE(engine.ApplyMove("b3a2"));
   EXPECT_TRUE(engine.ApplyMove("b6c6"));
   EXPECT_TRUE(engine.ApplyMove("d4e3"));
   EXPECT_TRUE(engine.ApplyMove("c6b6"));
   EXPECT_TRUE(engine.ApplyMove("a2a1"));
   EXPECT_TRUE(engine.ApplyMove("g6h4"));
   EXPECT_TRUE(engine.ApplyMove("e3f4"));
   EXPECT_TRUE(engine.ApplyMove("b6c6"));
   EXPECT_TRUE(engine.ApplyMove("f4g4"));
   EXPECT_TRUE(engine.ApplyMove("b2b1"));
   EXPECT_TRUE(engine.ApplyMove("a1b1"));
   EXPECT_TRUE(engine.ApplyMove("h2g2"));
   EXPECT_TRUE(engine.ApplyMove("b1b5"));
   EXPECT_TRUE(engine.ApplyMove("c6d6"));
   EXPECT_TRUE(engine.ApplyMove("h3f4"));
   EXPECT_TRUE(engine.ApplyMove("g2f2"));
   EXPECT_TRUE(engine.ApplyMove("a3b4"));
   EXPECT_TRUE(engine.ApplyMove("d6d1"));
   EXPECT_TRUE(engine.ApplyMove("b4d6"));
   EXPECT_TRUE(engine.ApplyMove("d1d2"));
   EXPECT_TRUE(engine.ApplyMove("b5d3"));
   EXPECT_TRUE(engine.ApplyMove("h4f3"));
   EXPECT_TRUE(engine.ApplyMove("d3c3"));
   EXPECT_TRUE(engine.ApplyMove("d2c2"));
   EXPECT_TRUE(engine.ApplyMove("c3c2"));
   EXPECT_TRUE(engine.ApplyMove("f2e3"));
   EXPECT_TRUE(engine.ApplyMove("f4e6"));
   EXPECT_TRUE(engine.ApplyMove("f6f7"));
   EXPECT_TRUE(engine.ApplyMove("e6c5"));
   EXPECT_TRUE(engine.ApplyMove("e3d4"));
   EXPECT_TRUE(engine.ApplyMove("c2f5"));
   EXPECT_TRUE(engine.ApplyMove("f3h4"));
   EXPECT_TRUE(engine.ApplyMove("f5f4"));
   EXPECT_TRUE(engine.ApplyMove("d4d5"));
   EXPECT_TRUE(engine.ApplyMove("f4d2"));
   EXPECT_TRUE(engine.ApplyMove("d5c6"));
   EXPECT_TRUE(engine.ApplyMove("d6g3"));
   EXPECT_TRUE(engine.ApplyMove("c6c5"));
   EXPECT_TRUE(engine.ApplyMove("d2g2"));
   EXPECT_TRUE(engine.ApplyMove("h4g2"));
   EXPECT_TRUE(engine.ApplyMove("g4g5"));
   EXPECT_TRUE(engine.ApplyMove("g2e3"));
   EXPECT_TRUE(engine.ApplyMove("g5h4"));
   EXPECT_TRUE(engine.ApplyMove("e3f1"));
   EXPECT_TRUE(engine.ApplyMove("g3f4"));
   EXPECT_TRUE(engine.ApplyMove("c5c4"));
   EXPECT_TRUE(engine.ApplyMove("f4e5"));

   EXPECT_FALSE(engine.ApplyMove("d7d8")); // failed to promote
}

TEST(engine_tests, test_is_check)
{
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("f2f3"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));
   EXPECT_TRUE(engine.ApplyMove("g2g4"));

   Position position = engine.GetPosition();
   EXPECT_FALSE(Engine::IsCheck(position, 'K'));
   EXPECT_FALSE(Engine::IsCheck(position, 'k'));

   EXPECT_TRUE(engine.ApplyMove("d8h4")); // mate

   position = engine.GetPosition();
   EXPECT_TRUE(Engine::IsCheck(position, 'K'));
   EXPECT_FALSE(Engine::IsCheck(position, 'k'));
}

TEST(engine_tests, test_list_all_legal_moves)
{
   RandomEngine engine;
   Position position = engine.GetPosition();
   EXPECT_EQ(1, Engine::ListAllLegalMoves(position));
}

TEST(engine_tests, test_get_outcome)
{
   RandomEngine engine;

   EXPECT_TRUE(engine.ApplyMove("f2f3"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));
   EXPECT_TRUE(engine.ApplyMove("g2g4"));

   Position position = engine.GetPosition();
   EXPECT_FALSE(Engine::IsCheck(position, 'K'));
   EXPECT_FALSE(Engine::IsCheck(position, 'k'));

   EXPECT_TRUE(engine.ApplyMove("d8h4")); // mate

   position = engine.GetPosition();
   EXPECT_TRUE(Engine::IsCheck(position, 'K'));
   EXPECT_FALSE(Engine::IsCheck(position, 'k'));

   EXPECT_EQ(OUTCOME_WIN_BLACK, engine.GetOutcome());
}

TEST(searching_engine_tests, DISABLED_test_get_best_move)
{
   SearchingEngine engine;

   EXPECT_TRUE(engine.ApplyMove("d2d4"));
   EXPECT_TRUE(engine.ApplyMove("e7e5"));

   std::string best;
   engine.GetBestMove(&best);

   //EXPECT_STREQ("d4e5", best.c_str());
}

TEST(searching_engine_tests, debug_crash)
{
   SearchingEngine engine;
   UCIHandler handler(&engine);

   uci_tokens tokens;
   UCIParser::BreakIntoTokens("position startpos moves e2e4 a7a5 f1c4 a5a4 "
      "d1h5 e7e6 g1f3 a8a7 f3e5 g7g6 h5f3 d8f6 f3f6 g8f6 b1c3 a7a5 d2d4 f8b4 "
      "e1g1 a4a3 b2b3 b4c3 a1b1 a5a7 c1e3 c7c5 d4c5 c3e5 c5c6 a7a5 b3b4 a5a4 "
      "c4b5 e5h2 g1h2 f6g4 h2g3 g4e3 f2e3 a4a7 c6d7 c8d7 b5d7 b8d7 b1d1 a7a4 "
      "c2c3 a4a7 c3c4 a7a4 d1d3 a4b4 d3a3 d7b6 a3b3 b4b3 a2b3 b6a8 f1a1 a8b6 "
      "c4c5 f7f5 a1a8 b6a8 b3b4 a8c7 c5c6 b7c6 e4f5 e6f5 g3f4 c7d5 f4e5 d5b4 "
      "e5d6 b4c2 d6c6 c2e3 g2g3 e3f1 g3g4 f5g4 c6d5 e8d7 d5d4 d7c6 d4e5 c6b5 "
      "e5d6 b5a4 d6c6 a4a3 c6b5 a3a2 b5b4 a2a1 b4b3 a1b1 b3c3 b1a1 c3d3 a1a2 "
      "d3e2", &tokens);
   EXPECT_TRUE(handler.handle_position(tokens));

   tokens.clear();
   UCIParser::BreakIntoTokens("go movetime 10000", &tokens);
   EXPECT_TRUE(handler.handle_go(tokens));
}

TEST(searching_engine_tests, debug_crash2)
{
   SearchingEngine engine;
   UCIHandler handler(&engine);

   uci_tokens tokens;
   UCIParser::BreakIntoTokens("position startpos moves e2e4 a7a5 g1f3 a5a4 "
      "f1c4 a8a5 e1g1 a5a7 d2d4 a7a5 b1c3 a5a7 c1f4 a4a3 b2b3 a7a5 d1d3 a5a7 "
      "f1e1 a7a5 e4e5 a5a7 a1d1 a7a5 c4b5 a5a7 e5e6 f7e6 e1e6 b8c6 b5c6 b7c6 "
      "e6e5 d7d5 f4g5 a7a5 d1e1 a5a7 g5e7 f8e7 e5e7 g8e7 f3g5 a7a5 d3f3 d8d7 "
      "f3f7 e8d8 e1e7 d7e7 f7e7 d8e7 f2f4 a5a6 h2h3 c8f5 g2g4 f5c2 f4f5 a6a5 "
      "g5e6 g7g6 c3e2 a5a7 e2f4 g6f5 g4f5 c2b1 f4h5 b1a2 f5f6 e7e6 g1f2 a2b3 "
      "f2f3 b3d1 f3f4 d1h5 f4g5 a3a2 g5h6 a2a1q h6h5 a1d4 h5h6", &tokens);
   EXPECT_TRUE(handler.handle_position(tokens));

   tokens.clear();
   UCIParser::BreakIntoTokens("go movetime 10000", &tokens);
   EXPECT_TRUE(handler.handle_go(tokens));
}

TEST(genetic_tournament_tests, DISABLED_test_genetic_tournament)
{
   TestGeneticEngine engine;
   GeneticTournament<TestGeneticEngine> tournament(engine);
   tournament.Go(&engine);
   EXPECT_NEAR(engine.GetParameterValue(0), 60.0, 1.0);
}

TEST(engine_tests, illegal_move_test_10)
{
   RandomEngine engine;
   EXPECT_TRUE(engine.ApplyMove("a2a4"));
   EXPECT_FALSE(engine.ApplyMove("b7a6"));
}

TEST(engine_tests, list_all_legal_moves_test)
{
   RandomEngine engine;
   Move *move_buffer = new Move[MAX_NUMBER_OF_LEGAL_MOVES];

   engine.SetToStartPos();

   Position position = engine.GetPosition();
   EXPECT_EQ(20, Engine::ListAllLegalMoves(position, move_buffer));
   EXPECT_TRUE(engine.ApplyMove("a2a4"));

   position = engine.GetPosition();
   EXPECT_EQ(20, Engine::ListAllLegalMoves(position, move_buffer));
   EXPECT_TRUE(engine.ApplyMove("d7d6"));

   position = engine.GetPosition();
   EXPECT_EQ(21, Engine::ListAllLegalMoves(position, move_buffer));

   engine.SetToStartPos();

   EXPECT_TRUE(engine.ApplyMove("a2a3"));
   position = engine.GetPosition();
   EXPECT_EQ(20, Engine::ListAllLegalMoves(position, move_buffer));

   EXPECT_TRUE(engine.ApplyMove("f7f6"));
   position = engine.GetPosition();
   EXPECT_EQ(19, Engine::ListAllLegalMoves(position, move_buffer));

   EXPECT_TRUE(engine.ApplyMove("g2g3"));
   position = engine.GetPosition();
   EXPECT_EQ(19, Engine::ListAllLegalMoves(position, move_buffer));

   EXPECT_TRUE(engine.ApplyMove("c7c6"));
   position = engine.GetPosition();
   EXPECT_EQ(20, Engine::ListAllLegalMoves(position, move_buffer));

   EXPECT_TRUE(engine.ApplyMove("f1g2"));
   position = engine.GetPosition();
   EXPECT_EQ(20, Engine::ListAllLegalMoves(position, move_buffer));

   engine.SetToStartPos();

   EXPECT_TRUE(engine.ApplyMove("g1f3"));
   EXPECT_TRUE(engine.ApplyMove("a7a6"));
   EXPECT_TRUE(engine.ApplyMove("g2g3"));
   EXPECT_TRUE(engine.ApplyMove("a6a5"));
   EXPECT_TRUE(engine.ApplyMove("f1h3"));
   EXPECT_TRUE(engine.ApplyMove("a5a4"));
   EXPECT_EQ(20, Engine::ListAllLegalMoves(position, move_buffer));

   delete[] move_buffer;
}

TEST(engine_tests, threefold_repetition_test)
{
   RandomEngine engine;
   engine.SetToStartPos();
   
   Position position = engine.GetPosition();
   ASSERT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h2h3"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h7h6"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h1h2"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h8h7"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h2h1"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h7h8"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h1h2"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h8h7"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h2h1"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h7h8"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw()); // not yet, castle changed

   EXPECT_TRUE(engine.ApplyMove("h1h2"));
   position = engine.GetPosition();
   EXPECT_FALSE(position.MayClaimDraw());

   EXPECT_TRUE(engine.ApplyMove("h8h7"));
   position = engine.GetPosition();
   EXPECT_TRUE(position.MayClaimDraw());
}

TEST(searching_engine_tests, debug_crash3)
{
   SearchingEngine engine;
   UCIHandler handler(&engine);

   uci_tokens tokens;
   UCIParser::BreakIntoTokens("position startpos moves e2e3 e7e6 d1g4 d8f6 "
      "f1d3 f8d6 b1c3 h7h5 g4c4 d6e5 c3e4 f6e7 g1f3 d7d5 c4c5 d5e4 c5e5 e4f3 "
      "e5g7 f3g2 h1g1 e7f6 g7g3 c7c6 e1e2 e6e5 g3g2 c8g4 f2f3 g4f5 d3f5 g8h6 "
      "f5c8 e8d8 c8b7 e5e4 b7a8 h8e8 f3e4 h6g4 g2g3 d8d7 g1f1 f6e6 d2d3 b8a6 "
      "a8b7 a6c5 c1d2 c5b7 g3h3 e8h8 d2c3 f7f6 f1g1 a7a6 a1f1 b7d6 b2b3 c6c5 "
      "f1f6 g4f6 g1g7 d6f7 g7f7 d7d6 e4e5 e6e5 c3e5 d6c6 f7f6", &tokens);
   EXPECT_TRUE(handler.handle_position(tokens));

   tokens.clear();
   UCIParser::BreakIntoTokens("go movetime 2000", &tokens);
   EXPECT_TRUE(handler.handle_go(tokens));
}
