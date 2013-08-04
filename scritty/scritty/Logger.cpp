// Scritty is Copyright (c) 2013 by Joel Odom, Marietta, GA, All Rights Reserved

#include "Logger.h"
#include <Windows.h>
#include <fstream>

using namespace scritty;

/*static*/ Logger Logger::s_instance;

Logger::Logger() : m_out_stream(nullptr)
{
}

Logger::~Logger()
{
   m_out_stream->close();
   delete m_out_stream;
}

/*static*/ void Logger::LogMessage(const std::string &message)
{
   s_instance.InternalLogMessage(message);
}

/*static*/ std::ostream& Logger::GetStream()
{
   return *s_instance.m_out_stream;
}

/*static*/ void Logger::InternalLogMessage(const std::string &message)
{
   if (m_out_stream == nullptr)
   {
      // open a log file (TODO: include date and time when started)
      CHAR temp_path[MAX_PATH + 1];
      ::GetTempPath(MAX_PATH + 1, temp_path);
      m_out_stream = new std::ofstream(std::string(temp_path) + "scritty.log",
         std::ios_base::out | std::ios_base::app);
   }

   *m_out_stream << message << std::endl;
}