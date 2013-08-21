// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "GeneticTournament.h"
#include <iostream>

using namespace scritty;

void GeneticEngine::GetParameterName(size_t index, std::string *name) const
{
   SCRITTY_ASSERT(index < m_parameters_size);
   *name = m_parameters[index].name;
}

double GeneticEngine::GetParameterValue(size_t index) const
{
   SCRITTY_ASSERT(index < m_parameters_size);
   return m_parameters[index].value;
}

void GeneticEngine::PrintParameters() const
{
   for (size_t i = 0; i < m_parameters_size; ++i)
      std::cout << m_parameters[i].name << ": "
      << m_parameters[i].value << std::endl;
}

#define big_rand() (rand()*rand())

void GeneticEngine::RandomizeParameters(double max_deviation)
{
   for (size_t i = 0; i < m_parameters_size; ++i)
   {
      const double PRECISION_MULTIPLIER = 10000.0;
      int a = (unsigned int)::abs(
         m_parameters[i].value*max_deviation*PRECISION_MULTIPLIER);
      int deviation = big_rand() % (2*a) - a;
      m_parameters[i].value += deviation / PRECISION_MULTIPLIER;
   }
}

/*static*/ void GeneticEngine::Breed(
   const GeneticEngine &mate1, const GeneticEngine &mate2, GeneticEngine *child)
{
   // engines should normally be same species, though possible to cross breed
   // if chromosomes are similar enough
   SCRITTY_ASSERT(mate1.m_parameters_size == mate2.m_parameters_size);
   
   for (size_t i = 0; i < mate1.m_parameters_size; ++i)
   {
      SCRITTY_ASSERT(strcmp(
         mate1.m_parameters[i].name, mate2.m_parameters[i].name) == 0);
      strcpy_s(child->m_parameters[i].name, MAX_PARAMETER_NAME_LEN + 1,
         mate1.m_parameters[i].name);
      child->m_parameters[i].value = rand() % 2 == 0
         ? mate1.m_parameters[i].value : mate2.m_parameters[i].value;
   }

   child->RandomizeParameters(MAX_INCREMENTAL_DEVIATION);
}

TestGeneticEngine::TestGeneticEngine()
{
   // initially all start out at 50.0
   m_parameters_size = 10;
   m_parameters = new ParameterPair[m_parameters_size];
   for (size_t i = 0; i < 10; ++i)
   {
      strcpy_s(m_parameters[i].name, MAX_PARAMETER_NAME_LEN + 1,
         "Test Parameter");
      m_parameters[i].value = 50.0;
   }
}

TestGeneticEngine *TestGeneticEngine::Clone() const
{
   TestGeneticEngine *clone = new TestGeneticEngine;
   SCRITTY_ASSERT(clone->m_parameters_size == m_parameters_size);

   for (size_t i = 0; i < m_parameters_size; ++i)
   {
      SCRITTY_ASSERT(strcmp(
         clone->m_parameters[i].name, m_parameters[i].name) == 0);
      clone->m_parameters[i].value = m_parameters[i].value;
   }

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
