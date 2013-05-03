/* Copyright (c) 2011 Ironclad Mobile, Inc. All rights reserved */

#include <iostream>

#ifndef ICM_LOGGER_HPP
#define ICM_LOGGER_HPP

namespace ORawrM
{
    //Unified logger interface.
    struct ILogger
    {
        enum Level
        {   
            Message = 0,
            Warning,
            Error
        };
        static const char * levelString(size_t l);
        
        virtual ~ILogger();

        virtual void log(size_t level, const std::string& message, const char * file, size_t line) = 0;
        virtual ILogger * clone() = 0;
    };

    ILogger * nullLogger();
    ILogger * stderrLogger();
    ILogger * stdoutLogger();
};

#define ORAWRM_LOG_ERROR(logger, msg) (logger)->log(ORawrM::ILogger::Error, (msg), __FILE__, __LINE__)
#define ORAWRM_LOG_MESSAGE(logger, msg) (logger)->log(ORawrM::ILogger::Message, (msg), __FILE__, __LINE__)
#define ORAWRM_LOG_WARNING(logger, msg) (logger)->log(ORawrM::ILogger::Warning, (msg), __FILE__, __LINE__)



#endif
