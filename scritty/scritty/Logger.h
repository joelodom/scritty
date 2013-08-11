// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#ifndef SCRITTY_LOGGER_H
#define SCRITTY_LOGGER_H

#include <string>

namespace scritty
{
   class Logger
   {
   public:
      static void LogMessage(const std::string &message);
      static std::ostream& GetStream();

      Logger();
      ~Logger();

   private:
      void StartStream();
      void InternalLogMessage(const std::string &message);

      std::ofstream* m_out_stream;

      static Logger s_instance;
   };
}

#endif // #ifndef SCRITTY_LOGGER_H
