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

#define PARTICIPANTS 4
#define MAX_INITIAL_DEVIATION 0.5 // 50%
#define MAX_INCREMENTAL_DEVIATION 0.01 // 1%
#define ROUNDS 1

   class GeneticEngine : public Engine
   {
   public:
      GeneticEngine() : Engine()
      {
      }

      void GetParameterName(size_t index, std::string *name) const;
      double GetParameterValue(size_t index) const;
      void PrintParameters() const;

      // (0.01 for 1% max)
      void RandomizeParameters(double max_deviation);

      static void Breed(const GeneticEngine &mate1, const GeneticEngine &mate,
         GeneticEngine *child);

      virtual int Compare(GeneticEngine *first, // 1, 0 or -1
         GeneticEngine *second) const = 0; // 1 = first wins

   protected:
      // name / value pair
      std::vector<std::pair<std::string, double>> m_parameters;

   private:
      GeneticEngine(const GeneticEngine &); // copy disallowed
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

      TestGeneticEngine *Clone() const;

   private:
      TestGeneticEngine(const TestGeneticEngine &); // copy disallowed
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
            T *clone = prototype.Clone();
            clone->RandomizeParameters(MAX_INITIAL_DEVIATION);
            m_participants.push_back(clone);
         }
      }

      ~GeneticTournament()
      {
         for (auto it = m_participants.begin();
            it != m_participants.end(); ++it)
            delete *it;
      }

      void Go(T **winner) // TODO: larger deviations at first, narrowing down???
      {
         // caller should delete winner
         SCRITTY_ASSERT(winner != nullptr);

         // single elimination tournament where eliminated participants are
         // replaced by children of winners at the end of each round

         for (size_t round_number = 1; round_number <= ROUNDS; ++round_number)
         {
            std::cout << "Hosting round " << round_number
               << " of " << ROUNDS << "." << std::endl;

            std::vector<T *> winners;

            while (m_participants.size() > 1) // could leave one unpaired
            {
               std::cout << m_participants.size() / 2
                  << " games remaining to play in this round." << std::endl;

               // pair two random participants
               size_t i = rand() % m_participants.size();
               T *first = m_participants[i];
               m_participants.erase(m_participants.begin() + i);
               i = rand() % m_participants.size();
               T *second = m_participants[i];
               m_participants.erase(m_participants.begin() + i);

               // eliminate the less worthy or keep both

               int result = first->Compare(first, second);

               if (result == 1)
               {
                  delete second;
                  winners.push_back(first);
               }
               else if (result == -1)
               {
                  delete first;
                  winners.push_back(second);
               }
               else
               {
                  // we're all winners!  yea!
                  winners.push_back(first);
                  winners.push_back(second);
               }
            }

            // if this is the last round, choose a winner from the master race
            if (round_number == ROUNDS)
            {
               *winner = winners[rand() % winners.size()]->Clone();
               for (auto it = winners.begin(); it != winners.end(); ++it)
                  delete *it;
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
               T *first = winners[rand() % winners.size()];
               T *second = winners[rand() % winners.size()];
               T *child = new T;
               GeneticEngine::Breed(*first, *second, child);
               m_participants.push_back(child);
            }
         }
      }

   private:
      std::vector<T *> m_participants;
   };
}

#endif // #ifndef GENETIC_TOURNAMENT_H
