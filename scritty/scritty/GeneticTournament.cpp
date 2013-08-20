// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "GeneticTournament.h"
#include <iostream>

using namespace scritty;

void GeneticEngine::GetParameterName(size_t index, std::string *name) const
{
   *name = m_parameters[index].first;
}

double GeneticEngine::GetParameterValue(size_t index) const
{
   return m_parameters[index].second;
}

void GeneticEngine::PrintParameters() const
{
   for (auto it = m_parameters.begin(); it != m_parameters.end(); ++it)
      std::cout << it->first << ": " << it->second << std::endl;
}

#define big_rand() (rand()*rand())

void GeneticEngine::RandomizeParameters(double max_deviation)
{
   for (size_t i = 0; i < m_parameters.size(); ++i)
   {
      const double PRECISION_MULTIPLIER = 10000.0;
      int a = (unsigned int)::abs(
         m_parameters[i].second*max_deviation*PRECISION_MULTIPLIER);
      int deviation = big_rand() % (2*a) - a;
      m_parameters[i].second += deviation / PRECISION_MULTIPLIER;
   }
}

/*static*/ void GeneticEngine::Breed(
   const GeneticEngine &mate1, const GeneticEngine &mate2, GeneticEngine *child)
{
   if (mate1.m_parameters.size() != mate2.m_parameters.size())
      return; // engines should really be same species
   
   for (size_t i = 0; i < mate1.m_parameters.size(); ++i)
   {
      child->m_parameters[i] = rand() % 2 == 0
         ? mate1.m_parameters[i] : mate2.m_parameters[i];
   }

   child->RandomizeParameters(MAX_INCREMENTAL_DEVIATION);
}

TestGeneticEngine::TestGeneticEngine()
{
   // initially all start out at 50.0

   for (size_t i = 0; i < 10; ++i)
      m_parameters.push_back(std::pair<std::string, double>("", 50.0));
}

TestGeneticEngine *TestGeneticEngine::Clone() const
{
   TestGeneticEngine *clone = new TestGeneticEngine;
   SCRITTY_ASSERT(clone->m_parameters.size() == m_parameters.size());

   for (size_t i = 0; i < m_parameters.size(); ++i)
      clone->m_parameters[i] = m_parameters[i];

   return clone;
}

/*virtual*/ int TestGeneticEngine::Compare(
   GeneticEngine *first, GeneticEngine *second) const
{
   // whichever has more parameters closest to 60.0 wins

   int score_first = 0;
   int score_second = 0;

   for (size_t i = 0; i < 10; ++i)
   {
      double deviation_first = ::abs(60.0 - first->GetParameterValue(i));
      double deviation_second = ::abs(60.0 - second->GetParameterValue(i));

      if (deviation_first > deviation_second)
         ++score_second;
      else if (deviation_first < deviation_second)
         ++score_first;
   }

   if (score_first == score_second)
      return 0;
   return score_first > score_second ? 1 : 0;
}
