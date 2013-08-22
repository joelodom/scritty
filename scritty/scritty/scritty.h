// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_H
#define SCRITTY_H

#include "gtest/gtest.h"

#ifdef _DEBUG
#define SCRITTY_NAME "Scritty 0.0 Pre-alpha DEBUG"
#else
#define SCRITTY_NAME "Scritty 0.0 Pre-alpha RELEASE"
#endif

#define SCRITTY_AUTHOR "Joel Odom"

#ifdef _DEBUG
#define SCRITTY_ASSERT(x) if (!(x)) throw std::string("assert failure");
#else
#define SCRITTY_ASSERT(x)
#endif

namespace scritty
{
   class ScrittyTestEnvironment : public ::testing::Environment
   {
   public:
      virtual ~ScrittyTestEnvironment() {}

      virtual void SetUp()
      {
      }

      virtual void TearDown()
      {
         // TODO P3: I can't figure out how to ignore Google Test global objects
         // so I miss leaks during testing, which is kind of stupid.
         _CrtMemCheckpoint(&ScrittyTestEnvironment::s_mem_state);
      }

      static bool s_tests_were_run;
      static _CrtMemState s_mem_state;
   };

   enum Outcome
   {
      OUTCOME_UNDECIDED,
      OUTCOME_WIN_WHITE,
      OUTCOME_WIN_BLACK,
      OUTCOME_DRAW
   };
}

#endif // #ifndef SCRITTY_H
