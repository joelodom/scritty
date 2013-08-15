// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef GENETIC_TOURNAMENT_H
#define GENETIC_TOURNAMENT_H

#include <string>
#include <vector>
#include "Engine.h"

namespace scritty
{
   // INTERESTING: seems to work better with fewer participants and more rounds
   // INTERESTING: setting max_deviation too large gives poor results

#define PARTICIPANTS 10
#define MAX_INITIAL_DEVIATION 1.0 // 100%!
#define MAX_INCREMENTAL_DEVIATION 0.01 // 1%
#define ROUNDS 1000

   class GeneticEngine : public Engine
   {
   public:
      void GetParameterName(size_t index, std::string *name) const;
      double GetParameterValue(size_t index) const;
      void PrintParameters() const;

      // (0.01 for 1% max)
      // multiple calls will randomize again
      void RandomizeParameters(double max_deviation);

      static void Breed(const GeneticEngine &mate1, const GeneticEngine &mate,
         GeneticEngine *child);

      virtual int Compare(GeneticEngine *first, // 1, 0 or -1
         GeneticEngine *second) const = 0; // 1 = first wins

   protected:
      // name / value pair
      std::vector<std::pair<std::string, double>> m_parameters;
   };

   class TestGeneticEngine : public GeneticEngine
   {
   public:
      TestGeneticEngine();

      virtual int Compare(GeneticEngine *first, GeneticEngine *second) const;

      virtual Outcome GetBestMove(std::string *best) const
      {
         return OUTCOME_UNDECIDED;
      }
   };

   template <class T>
   class GeneticTournament
   {
   public:
      // TODO: how to get this out of header???

      GeneticTournament(const T &prototype)
      {
         // initially populate m_participants
         // for initial diversity allow a much wider deviation to start
         for (size_t i = 0; i < PARTICIPANTS; i++)
         {
            T copy = prototype;
            copy.RandomizeParameters(MAX_INITIAL_DEVIATION);
            m_participants.push_back(copy);
         }
      }

      void Go(T *winner) // TODO: larger deviations at first, narrowing down???
      {
         // single elimination tournament where eliminated participants are
         // replaced by children of winners at the end of each round

         for (size_t round_number = 1; round_number <= ROUNDS; ++round_number)
         {
            std::cout << "Hosting round " << round_number
               << " of " << ROUNDS << "." << std::endl;

            std::vector<T> winners;

            while (m_participants.size() > 1) // could leave one unpaired
            {
               std::cout << m_participants.size() / 2
                  << " games remaining to play in this round." << std::endl;

               // pair two random participants
               size_t i = rand() % m_participants.size();
               T first = m_participants[i];
               m_participants.erase(m_participants.begin() + i);
               i = rand() % m_participants.size();
               T second = m_participants[i];
               m_participants.erase(m_participants.begin() + i);

               // eliminate the less worthy or keep both
               int result = first.Compare(&first, &second);
               if (result > -1)
                  winners.push_back(first);
               if (result < 1)
                  winners.push_back(second);
            }

            // if this is the last round, choose a winner from the master race
            if (round_number == ROUNDS)
            {
               *winner = winners[rand() % winners.size()];
               return;
            }

            // add winners back to the participants for the next round
            for (auto it = winners.begin(); it != winners.end(); ++it)
               m_participants.push_back(*it);

            // breed winners randomly and add children to participants
            // except on last round

            while (m_participants.size() < PARTICIPANTS)
            {
               // may breed a winner with self
               T first = winners[rand() % winners.size()];
               T second = winners[rand() % winners.size()];
               T child;
               GeneticEngine::Breed(first, second, &child);
               m_participants.push_back(child);
            }
         }
      }

   private:
      std::vector<T> m_participants;
   };
}

#endif // #ifndef GENETIC_TOURNAMENT_H
