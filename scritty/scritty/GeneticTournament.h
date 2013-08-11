// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef GENETIC_TOURNAMENT_H
#define GENETIC_TOURNAMENT_H

#include <string>
#include <vector>

class GeneticEngine
{
public:
   std::vector<std::string> * const GetParameterNames();
   std::vector<double> * const GetParameters();

   // (0.01 for 1% max)
   // multiple calls will randomize again
   void RandomizeParameters(double max_deviation);

   void Breed(const GeneticEngine &mate, GeneticEngine *child) const;

protected:
   std::vector<std::string> m_parameter_names;
   std::vector<double> m_parameters;
};

class GeneticTournament
{
public:
   GeneticTournament(const GeneticEngine& prototype,
      double max_deviation, // (0.01 for 1% max)
      size_t number_per_round, // number of participants per round (EVEN!)
      size_t number_to_survive, // number of top survivors to keep
      size_t number_of_rounds)
      : m_max_deviation(max_deviation),
      m_number_per_round(number_per_round),
      m_number_to_survive(number_to_survive),
      m_number_of_rounds(number_of_rounds)
   {
      // initially populate m_participants with slightly different prototypes
      for (size_t i = 0; i < m_number_per_round; i++)
      {
         GeneticEngine copy = prototype;
         copy.RandomizeParameters(m_max_deviation);
         m_participants.push_back(copy);
      }
   }

   void Go();

private:
   std::vector<GeneticEngine> m_participants;
   double m_max_deviation;
   size_t m_number_per_round;
   size_t m_number_to_survive;
   size_t m_number_of_rounds;
};

#endif // #ifndef GENETIC_TOURNAMENT_H
