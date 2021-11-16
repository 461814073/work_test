#ifndef _LOG_H_
#define _LOG_H_

#include "spdlog/spdlog.h"
#include "spdlog/sinks/syslog_sink.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/sinks/daily_file_sink.h"
#include "spdlog/async.h"
#include "spdlog/fmt/bin_to_hex.h"
#include "spdlog/fmt/ostr.h" // must be included
#include <iostream>

//在  spdlog.h   之前定义，才有效
#ifndef SPDLOG_TRACE_ON
#define SPDLOG_TRACE_ON
#endif
 
#ifndef SPDLOG_DEBUG_ON
#define SPDLOG_DEBUG_ON
#endif

/******* Console Debug Interface *******/
#define Console_Trace(...) Log::GetInstance().GetConsole()->trace(__VA_ARGS__)
#define Console_Debug(...) Log::GetInstance().GetConsole()->debug(__VA_ARGS__)
#define Console_Info(...) Log::GetInstance().GetConsole()->info(__VA_ARGS__)
#define Console_Warn(...) Log::GetInstance().GetConsole()->warn(__VA_ARGS__)
#define Console_Error(...) Log::GetInstance().GetConsole()->error(__VA_ARGS__)
#define Console_Critical(...) Log::GetInstance().GetConsole()->critical(__VA_ARGS__)

/******* Basic Logger Interface *******/
#define Basic_Trace(...) Log::GetInstance().GetBasicLogger()->trace(__VA_ARGS__)
#define Basic_Debug(...) Log::GetInstance().GetBasicLogger()->debug(__VA_ARGS__)
#define Basic_Info(...) Log::GetInstance().GetBasicLogger()->info(__VA_ARGS__)
#define Basic_Warn(...) Log::GetInstance().GetBasicLogger()->warn(__VA_ARGS__)
#define Basic_Error(...) Log::GetInstance().GetBasicLogger()->error(__VA_ARGS__)
#define Basic_Critical(...) Log::GetInstance().GetBasicLogger()->critical(__VA_ARGS__)

/******* Daily Logger Interface *******/
#define Daily_Trace(...) Log::GetInstance().GetDailyLogger()->trace(__VA_ARGS__)
#define Daily_Debug(...) Log::GetInstance().GetDailyLogger()->debug(__VA_ARGS__)
#define Daily_Info(...) Log::GetInstance().GetDailyLogger()->info(__VA_ARGS__)
#define Daily_Warn(...) Log::GetInstance().GetDailyLogger()->warn(__VA_ARGS__)
#define Daily_Error(...) Log::GetInstance().GetDailyLogger()->error(__VA_ARGS__)
#define Daily_Critical(...) Log::GetInstance().GetDailyLogger()->critical(__VA_ARGS__)

/******* Rotating Logger Interface *******/
#define Rotating_Trace(...) Log::GetInstance().GetRotatingLogger() ->trace(__VA_ARGS__);//Console_Trace(__VA_ARGS__)
#define Rotating_Debug(...) Log::GetInstance().GetRotatingLogger() ->debug(__VA_ARGS__);//Console_Debug(__VA_ARGS__)
#define Rotating_Info(...) Log::GetInstance().GetRotatingLogger() ->info(__VA_ARGS__);//Console_Info(__VA_ARGS__)
#define Rotating_Warn(...) Log::GetInstance().GetRotatingLogger() ->warn(__VA_ARGS__);//Console_Warn(__VA_ARGS__)
#define Rotating_Error(...) Log::GetInstance().GetRotatingLogger() ->error(__VA_ARGS__);//Console_Error(__VA_ARGS__)
#define Rotating_Critical(...) Log::GetInstance().GetRotatingLogger() ->critical(__VA_ARGS__);//Console_Critical(__VA_ARGS__)


//#define BASIC_LOG_PATH 		"/usr/local/bzl_robot/log/task_control0-basic.txt"			//基础日志
//#define DAILY_LOG_PATH 		"/usr/local/bzl_robot/log/task_control0-daily.txt"			//定时日志
//#define ROTATING_LOG_PATH 	"/usr/local/bzl_robot/log/task_control0-rotating.txt"		//循环日志

#define BASIC_LOG_PATH 		"log/task_control0-basic.txt"			//基础日志
#define DAILY_LOG_PATH 		"log/task_control0-daily.txt"			//定时日志
#define ROTATING_LOG_PATH 	"log/task_control0-rotating.txt"		//循环日志

using namespace std;

class Log
{
 
public:
	static Log& GetInstance() 
	{
		static Log m_instance;
		return m_instance;
	}
	
	std::shared_ptr<spdlog::logger>  
	GetConsole() 
	{ 
		return console;
	}
	
	std::shared_ptr<spdlog::logger> 
	GetBasicLogger() 
	{ 
		return basic_logger;
	}
	
	std::shared_ptr<spdlog::logger> 
	GetDailyLogger() 
	{ 
		return daily_logger;
	}
	
	std::shared_ptr<spdlog::logger> 
	GetRotatingLogger() 
	{ 
		return rotating_logger;
	}
 
private:
	Log() 
	{
			//console
		console = spdlog::stdout_color_mt("console");
		console->set_level(spdlog::level::trace);

		//basic logger
		//basic_logger = spdlog::basic_logger_mt("basic", BASIC_LOG_PATH);

		//daily logger
		//程序每运行一次，会建一个新日志，若程序不间断运行，会每天新建一个日志，如下
		//每天15:00创建日志
		//daily_logger = spdlog::daily_logger_mt("daily", DAILY_LOG_PATH,15,00);
		//如果log严重性高于err则触发更新
		//daily_logger->flush_on(spdlog::level::err);
		//roating logger
		//循环日志，最大5M，最多3个文件
		rotating_logger = spdlog::rotating_logger_mt("roate_log", ROTATING_LOG_PATH, 1048576 * 5, 3);
		spdlog::flush_every(std::chrono::seconds(10)); //每15秒写一次log	
	}
	~Log() 
	{
		spdlog::drop_all();
	}
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
 
private:
		
   std::shared_ptr<spdlog::logger> console;
   std::shared_ptr<spdlog::logger> basic_logger;	
   std::shared_ptr<spdlog::logger> daily_logger;
   std::shared_ptr<spdlog::logger> rotating_logger;
   
};
 
 
#endif
