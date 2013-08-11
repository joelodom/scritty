// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "GeneticTournament.h"
#include <iostream>

std::vector<std::string> * const GeneticEngine::GetParameterNames()
{
   return &m_parameter_names;
}

std::vector<double> * const GeneticEngine::GetParameters()
{
   return &m_parameters;
}

void GeneticEngine::RandomizeParameters(double max_deviation)
{
   for (size_t i = 0; i < m_parameters.size(); ++i)
   {
      const double PRECISION_MULTIPLIER = 10000.0;
      int a = (unsigned int)::abs(
         m_parameters[i]*max_deviation*PRECISION_MULTIPLIER);
      int deviation = rand() % (2*a) - a;
      m_parameters[i] += deviation / PRECISION_MULTIPLIER;
   }
}

void GeneticEngine::Breed(const GeneticEngine &mate, GeneticEngine *child) const
{
   if (mate.m_parameters.size() != m_parameters.size())
      return; // engines should really be same species
   
   *child = *this; // copy myself

   for (size_t i = 0; i < m_parameters.size(); ++i)
   {
      if (rand() % 2 == 1)
         child->m_parameters[i] = mate.m_parameters[i];
   }
}

void GeneticTournament::Go()
{
   for (size_t round_number = 1;
      round_number <= m_number_of_rounds; ++round_number)
   {
      std::cout << "Hosting round " << round_number
         << " of " << m_number_of_rounds << "." << std::endl;

//TODO #error working here....  need a games per round
   }
}
