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
   if (m_out_stream != nullptr)
   {
      m_out_stream->close();
      delete m_out_stream;
      m_out_stream = nullptr;
   }
}

/*static*/ void Logger::LogMessage(const std::string &message)
{
   s_instance.InternalLogMessage(message);
}

/*static*/ std::ostream& Logger::GetStream()
{
   s_instance.StartStream();
   return *s_instance.m_out_stream;
}

/*static*/ void Logger::StartStream()
{
   if (m_out_stream == nullptr)
   {
      // open a log file (TODO P4: include date and time when started)
      CHAR temp_path[MAX_PATH + 1];
      ::GetTempPath(MAX_PATH + 1, temp_path);
      m_out_stream = new std::ofstream(std::string(temp_path) + "scritty.log",
         std::ios_base::out | std::ios_base::app);
   }
}

void Logger::InternalLogMessage(const std::string &message)
{
   StartStream();
   *m_out_stream << message << std::endl;
}
