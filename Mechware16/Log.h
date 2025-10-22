/*
    _   ___ ___  _   _ ___ _  _  ___  _    ___   ___
   /_\ | _ \   \| | | |_ _| \| |/ _ \| |  / _ \ / __|
  / _ \|   / |) | |_| || || .` | (_) | |_| (_) | (_ |
 /_/ \_\_|_\___/ \___/|___|_|\_|\___/|____\___/ \___|

  Log library for Arduino
  version 1.0.3
  https://github.com/thijse/Arduino-Log

  Licensed under the MIT License <http://opensource.org/licenses/MIT>.

  Modified by SukkoPera to fit in a single header file.
  Log levels renamed after an idea by ChristianBauerAMDC.

*/

/**
 * \file Log.h
 * \author SukkoPera <software@sukkology.net>
 * \date 22 Feb 2020
 * \brief Log Library
 *
 * Please refer to the GitHub page and wiki for any information:
 * https://github.com/SukkoPera/MechBoard16
 */

#ifndef LOGGING_H
#define LOGGING_H

//#include <stdint.h>
//#include <stddef.h>
#include <inttypes.h>
#include <stdarg.h>
#include <Arduino.h>
#include "config.h"

typedef void (*printfunction)(Print*);

#define LOG_LEVEL_SILENT  0
#define LOG_LEVEL_FATAL   1
#define LOG_LEVEL_ERROR   2
#define LOG_LEVEL_WARN    3
#define LOG_LEVEL_INFO    4
#define LOG_LEVEL_DEBUG   5
#define LOG_LEVEL_TRACE   6

typedef int DebugLevel;

#define L_CR "\n"
#define LOGGING_VERSION 1_0_3

/**
 * Logging is a helper class to output informations over
 * RS232. If you know log4j or log4net, this logging class
 * is more or less similar ;-) <br>
 * Different loglevels can be used to extend or reduce output
 * All methods are able to handle any number of output parameters.
 * All methods print out a formated string (like printf).<br>
 * To reduce output and program size, reduce loglevel.
 * 
 * Output format string can contain below wildcards. Every wildcard
 * must be start with percent sign (\%)
 * 
 * ---- Wildcards
 * 
 * %s replace with a string (char*)
 * %c replace with a character
 * %d replace with an integer value
 * %l replace with a long value
 * %x replace and convert integer value to hex
 * %X like %x but prefix with 0x
 * %b replace and convert integer value to binary
 * %B like %x but prefix with 0b
 * %t replace and convert boolean value to "t" or "f"
 * %T like %t but convert to "true" or "false"
 * 
 * ---- Loglevels
 * 
 * 0 - LOG_LEVEL_SILENT     no output
 * 1 - LOG_LEVEL_FATAL      fatal errors
 * 2 - LOG_LEVEL_ERROR      all errors
 * 3 - LOG_LEVEL_WARN       errors and warnings
 * 4 - LOG_LEVEL_INFO       errors, warnings and infos
 * 5 - LOG_LEVEL_DEBUG      errors, warnings, infos, debugs
 * 6 - LOG_LEVEL_TRACE      all
 */

class Logging
{
public:
  /**
   * default Constructor
   */
  Logging()
#ifndef DISABLE_LOGGING
    : _level(LOG_LEVEL_SILENT),
    _showLevel(true),
    _logOutput(NULL)
#endif
  {

  }

  /**
   * Initializing, must be called as first. Note that if you use
   * this variant of Init, you need to initialize the baud rate
   * yourself, if printer happens to be a serial port.
   * 
   * \param level - logging levels <= this will be logged.
   * \param printer - place that logging output will be sent to.
   * \return void
   *
   */
  void begin(int level, Print *logOutput, bool showLevel = true)
  {
#ifndef DISABLE_LOGGING
    setLevel(level);
    setShowLevel(showLevel);
    _logOutput = logOutput;
#else
	(void) level;
	(void) logOutput;
	(void) showLevel;
#endif
  }

  /**
   * Set the log level.
   * 
   * \param level - The new log level.
   * \return void
   */
  void setLevel(int level)
  {
#ifndef DISABLE_LOGGING
    _level = constrain(level, LOG_LEVEL_SILENT, LOG_LEVEL_TRACE);
#else
	(void) level;
#endif
  }

  /**
   * Get the log level.
   *
   * \return The current log level.
   */
  int getLevel() const
  {
#ifndef DISABLE_LOGGING
    return _level;
#else
    return 0;
#endif
  }

  /**
   * Set whether to show the log level.
   * 
   * \param showLevel - true if the log level should be shown for each log
   *                    false otherwise.
   * \return void
   */
  void setShowLevel(bool showLevel)
  {
#ifndef DISABLE_LOGGING
    _showLevel = showLevel;
#else
	(void) showLevel;
#endif
  }

  /**
   * Get whether the log level is shown during logging
   * 
   * \return true if the log level is be shown for each log
   *         false otherwise.
   */
  bool getShowLevel() const
  {
#ifndef DISABLE_LOGGING
    return _showLevel;
#else
    return false;
#endif
  }

  /**
   * Sets a function to be called before each log command.
   * 
   * \param f - The function to be called
   * \return void
   */
  void setPrefix(printfunction f)
  {
#ifndef DISABLE_LOGGING
    _prefix = f;
#else
	(void) f;
#endif
  }

  /**
   * Sets a function to be called after each log command.
   * 
   * \param f - The function to be called
   * \return void
   */
  void setSuffix(printfunction f)
  {
#ifndef DISABLE_LOGGING
    _suffix = f;
#else
	(void) f;
#endif
  }

  /**
   * Output a fatal error message. Output message contains
   * F: followed by original message
   * Fatal error messages are printed out at
   * loglevels >= LOG_LEVEL_FATAL
   * 
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void fatal(T msg, Args... args)
  {
#if !defined (DISABLE_LOGGING) && (!defined (THRESHOLD_LEVEL) || THRESHOLD_LEVEL >= LOG_LEVEL_FATAL)
    printLevel(LOG_LEVEL_FATAL, msg, args...);
#endif
  }

  /**
   * Output an error message. Output message contains
   * E: followed by original message
   * Error messages are printed out at
   * loglevels >= LOG_LEVEL_ERROR
   * 
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void error(T msg, Args... args){
#if !defined (DISABLE_LOGGING) && (!defined (THRESHOLD_LEVEL) || THRESHOLD_LEVEL >= LOG_LEVEL_ERROR)
    printLevel(LOG_LEVEL_ERROR, msg, args...);
#else
	(void) msg;
	//~ (void) args;
#endif
  }

  /**
   * Output a warning message. Output message contains
   * W: followed by original message
   * Warning messages are printed out at
   * loglevels >= LOG_LEVEL_WARN
   * 
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void warn(T msg, Args...args)
  {
#if !defined (DISABLE_LOGGING) && (!defined (THRESHOLD_LEVEL) || THRESHOLD_LEVEL >= LOG_LEVEL_WARN)
    printLevel(LOG_LEVEL_WARN, msg, args...);
#else
	(void) msg;
	//~ (void) args;
#endif
  }

  /**
   * Output a info message. Output message contains
   * N: followed by original message
   * Notice messages are printed out at
   * loglevels >= LOG_LEVEL_INFO
   * 
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void info(T msg, Args...args)
  {
#if !defined (DISABLE_LOGGING) && (!defined (THRESHOLD_LEVEL) || THRESHOLD_LEVEL >= LOG_LEVEL_INFO)
    printLevel(LOG_LEVEL_INFO, msg, args...);
#else
	(void) msg;
	//~ (void) args;
#endif
  }

  /**
   * Output a debug message. Output message contains
   * N: followed by original message
   * Trace messages are printed out at
   * loglevels >= LOG_LEVEL_DEBUG
   * 
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
  */
  template <class T, typename... Args> void debug(T msg, Args... args)
  {
#if !defined (DISABLE_LOGGING) && (!defined (THRESHOLD_LEVEL) || THRESHOLD_LEVEL >= LOG_LEVEL_DEBUG)
    printLevel(LOG_LEVEL_DEBUG, msg, args...);
#else
	(void) msg;
	//~ (void) args;
#endif
  }

  /**
   * Output a trace message. Output message contains
   * V: followed by original message
   * Debug messages are printed out at
   * loglevels >= LOG_LEVEL_TRACE
   * 
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void trace(T msg, Args... args)
  {
#if !defined (DISABLE_LOGGING) && (!defined (THRESHOLD_LEVEL) || THRESHOLD_LEVEL >= LOG_LEVEL_TRACE)
    printLevel(LOG_LEVEL_TRACE, msg, args...);
#else
	(void) msg;
	//~ (void) args;
#endif
  }
  
  /**
   * Output a message at a given level.
   * 
   * \param msg format string to output
   * \param ... any number of variables
   * \return void
   */
  template <class T, typename... Args> void log(DebugLevel level, T msg, Args... args)
  {
#if !defined (DISABLE_LOGGING) && (!defined (THRESHOLD_LEVEL) || THRESHOLD_LEVEL >= level)
    printLevel(level, msg, args...);
#else
	(void) level;
	//~ (void) args;
#endif
  }

private:
  void print(const char *format, va_list args)
  {
#ifndef DISABLE_LOGGING     
    for (; *format != 0; ++format)
    {
      if (*format == '%')
      {
        ++format;
        printFormat(*format, &args);
      }
      else
      {
        _logOutput->print(*format);
      }
    }
#else
	(void) format;
	//~ (void) args;
#endif
  }

  void print(const __FlashStringHelper *format, va_list args)
  {
#ifndef DISABLE_LOGGING     
    PGM_P p = reinterpret_cast<PGM_P>(format);
    char c = pgm_read_byte(p++);
    for(;c != 0; c = pgm_read_byte(p++))
    {
      if (c == '%')
      {
        c = pgm_read_byte(p++);
        printFormat(c, &args);
      }
      else
      {
        _logOutput->print(c);
      }
    }
#else
	(void) format;
	(void) args;
#endif
  }

  void printFormat(const char format, va_list *args)
  {
#ifndef DISABLE_LOGGING
    if (format == '%')
    {
      _logOutput->print(format);
    }
    else if (format == 's')
    {
      register char *s = (char *)va_arg(*args, int);
      _logOutput->print(s);
    }
    else if (format == 'S')
    {
      register __FlashStringHelper *s = (__FlashStringHelper *)va_arg(*args, int);
      _logOutput->print(s);
    }
    else if (format == 'd' || format == 'i')
    {
      _logOutput->print(va_arg(*args, int), DEC);
    }
    else if (format == 'D' || format == 'F')
    {
      _logOutput->print(va_arg(*args, double));
    }
    else if (format == 'x')
    {
      _logOutput->print(va_arg(*args, int), HEX);
    }
    else if (format == 'X')
    {
      _logOutput->print("0x");
      _logOutput->print(va_arg(*args, int), HEX);
    }
    else if (format == 'b')
    {
      _logOutput->print(va_arg(*args, int), BIN);
    }
    else if (format == 'B')
    {
      _logOutput->print("0b");
      _logOutput->print(va_arg(*args, int), BIN);
    }
    else if (format == 'l')
    {
      _logOutput->print(va_arg(*args, long), DEC);
    }
    else if (format == 'u')
    {
      _logOutput->print(va_arg(*args, unsigned long), DEC);
    }
    else if (format == 'U')
    {
      _logOutput->print("0x");
      _logOutput->print(va_arg(*args, unsigned long), HEX);
    }
    else if (format == 'w')
    {
      _logOutput->print(va_arg(*args, word), DEC);
    }
    else if (format == 'c')
    {
      _logOutput->print((char) va_arg(*args, int));
    }
    else if(format == 't')
    {
      if (va_arg(*args, int) == 1)
      {
        _logOutput->print("T");
      }
      else
      {
        _logOutput->print("F");
      }
    }
    else if (format == 'T')
    {
      if (va_arg(*args, int) == 1)
      {
        _logOutput->print(F("true"));
      }
      else
      {
        _logOutput->print(F("false"));
      }
    }
#else
	(void) format;
	(void) args;
#endif
  }

  template <class T> void printLevel(int level, T msg, ...)
  {
#ifndef DISABLE_LOGGING
    if (level > _level)
    {
      return;
    }

    if (_prefix != NULL)
    {
      _prefix(_logOutput);
    }

    if (_showLevel) {
      static const char levels[] = "FEWIDT";
      _logOutput->print(level < 7 ? levels[level - 1] : '?');
      _logOutput->print(": ");
    }

    va_list args;
    va_start(args, msg);
    print(msg, args);

    if(_suffix != NULL)
    {
      _suffix(_logOutput);
    }
#endif
  }

#ifndef DISABLE_LOGGING
  int _level;
  bool _showLevel;
  Print* _logOutput;

  printfunction _prefix = NULL;
  printfunction _suffix = NULL;
#endif
};

extern Logging Log;
#endif
