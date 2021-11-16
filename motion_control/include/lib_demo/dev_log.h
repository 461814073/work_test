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
#include <fstream> 
#include <time.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>


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
#define Devflow_Trace(...) Log::GetInstance().GetDeviceFlowLogger()->trace(__VA_ARGS__)
#define Devflow_Debug(...) Log::GetInstance().GetDeviceFlowLogger()->debug(__VA_ARGS__)
#define Devflow_Info(...) Log::GetInstance().GetDeviceFlowLogger()->info(__VA_ARGS__)
#define Devflow_Warn(...) Log::GetInstance().GetDeviceFlowLogger()->warn(__VA_ARGS__)
#define Devflow_Error(...) Log::GetInstance().GetDeviceFlowLogger()->error(__VA_ARGS__)
#define Devflow_Critical(...) Log::GetInstance().GetDeviceFlowLogger()->critical(__VA_ARGS__)

/******* Rotating Logger Interface *******/
#define DevStatus_Trace(...) Log::GetInstance().GetDeviceStatusLogger() ->trace(__VA_ARGS__);//Console_Trace(__VA_ARGS__)
#define DevStatus_Debug(...) Log::GetInstance().GetDeviceStatusLogger() ->debug(__VA_ARGS__);//Console_Debug(__VA_ARGS__)
#define DevStatus_Info(...) Log::GetInstance().GetDeviceStatusLogger() ->info(__VA_ARGS__);//Console_Info(__VA_ARGS__)
#define DevStatus_Warn(...) Log::GetInstance().GetDeviceStatusLogger() ->warn(__VA_ARGS__);//Console_Warn(__VA_ARGS__)
#define DevStatus_Error(...) Log::GetInstance().GetDeviceStatusLogger() ->error(__VA_ARGS__);//Console_Error(__VA_ARGS__)
#define DevStatus_Critical(...) Log::GetInstance().GetDeviceStatusLogger() ->critical(__VA_ARGS__);//Console_Critical(__VA_ARGS__)


//#define BASIC_LOG_PATH 		"/usr/local/bzl_robot/log/task_control0-basic.txt"			//基础日志
//#define DAILY_LOG_PATH 		"/usr/local/bzl_robot/log/task_control0-daily.txt"			//定时日志
//#define ROTATING_LOG_PATH 	"/usr/local/bzl_robot/log/task_control0-rotating.txt"		//循环日志

//#define BASIC_LOG_PATH 		"log/task_control0-basic.txt"			//基础日志
//#define DAILY_LOG_PATH 		"log/task_control0-daily.txt"			//定时日志
//#define ROTATING_LOG_PATH 	"log/task_control0-rotating.txt"		//循环日志

//#define BASIC_LOG_PATH			"/home/nvidia/wpxj/log/"
//#define DEVFLOW_LOG_PATH			"/home/nvidia/wpxj/DevFlow/"
//#define DEVSTATUS_LOG_PATH		"/home/nvidia/wpxj/DevStatus/"

#define BASIC_LOG_PATH				"/usr/local/bzl_robot/ewsr_ms/log/basket_control/"
#define DEVFLOW_LOG_PATH			"DevFlow/"
#define DEVSTATUS_LOG_PATH			"DevStatus/"

#define LOG_HEAD			        "DevStatus"
#define MAX_LOG_FILE_TOTAL			1000

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
	GetDeviceFlowLogger() 
	{ 
		return device_flow_logger;
	}
	
	std::shared_ptr<spdlog::logger> 
	GetDeviceStatusLogger() 
	{ 
		return device_status_logger;
	}

	
	static int dateFilter(const struct dirent *pDir)//返回是数字的文件名
	{
		if (strcmp(pDir->d_name, ".") != 0 && strcmp(pDir->d_name, "..") != 0)
		{
			return 1;
		}
		return 0;
	}

	static int logfileFilter(const struct dirent *pDir)//返回是数字的文件名
	{
		string loghead=LOG_HEAD;
		string file(pDir->d_name);
		if (strcmp(pDir->d_name, ".") != 0 && strcmp(pDir->d_name, "..") != 0 && strncmp(file.c_str(),loghead.c_str(),9)==0)
		{
			return 1;
		}
		return 0;
	}


	void scanfiles(vector<string> &fileNames ,int & count)
	{
		
		struct dirent **datelist;
		struct dirent **loglist;
		int n,m;
		int i,j;
		string loghead=LOG_HEAD;
		string basepath=BASIC_LOG_PATH;
		string logpath;
		string logpfile;
		count = 0 ;
		n = scandir(basepath.c_str(), &datelist, dateFilter, alphasort);//customFilter 表明条件
		if (n < 0)
		{
			printf("error\n");
			return ; 
		}

		for (i = 0; i < n; i++)
		{
			logpath = basepath + datelist[i]->d_name + "/" + loghead;
			m = scandir(logpath.c_str(), &loglist, logfileFilter, alphasort);
			if (m < 0)
			{
				printf("error\n");
			}
			else
			{
				for (j = 0; j < m; j++)
				{
					logpfile = logpath + "/" + loglist[j]->d_name;
					count++;
					fileNames.push_back(logpfile);
					//std::cout<<"logpfile: "<<logpfile<<std::endl;
				}
				free(loglist);
			}
		}
		free(datelist);

	}

	void logfile_buff_check(void)
	{
		if(total_logfile > MAX_LOG_FILE_TOTAL)
		{
			int i = system(("rm " + fileNames[0]).c_str());
			fileNames.clear();
			scanfiles(fileNames,total_logfile);
			//std::cout<<total_logfile<<std::endl;
		}
	}


	void listDir(const char *name, vector<string> &fileNames, bool lastSlash)
	{
		DIR *dir;
		struct dirent *entry;
		struct stat statbuf;
		struct tm      *tm;
		time_t rawtime;
		if (!(dir = opendir(name))) 
		{   
			std::cout<<"Couldn't open the file or dir"<<name<<std::endl;
			return;
		}
		if (!(entry = readdir(dir)))
		{
			std::cout<<"Couldn't read the file or dir"<<name<<std::endl;
			return;
		}
		
		do 
		{
			string slash="";
			if(!lastSlash)
			slash = "/";
		
			string loghead="DevStatus";
			string parent(name);
			string file(entry->d_name);
			string final = parent + slash + file;
			std::cout<<"file: "<<file<<std::endl;
			if(stat(final.c_str(), &statbuf)==-1)
			{
				std::cout<<"Couldn't get the stat info of file or dir: "<<final<<std::endl;
				return;
			}
			if (entry->d_type == DT_DIR) //its a directory
			{
				//skip the . and .. directory
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
					continue;
				listDir(final.c_str(), fileNames, false);
			}
			else if(strncmp(file.c_str(),loghead.c_str(),9)==0)// it is a file
			{
				fileNames.push_back(final);
			}
		}while (entry = readdir(dir));
		closedir(dir);	
	}

	int find_dir_file(const char *dir_name, vector<string>& v)   //文件夹地址，文件列表  
	{
		
		DIR *dirp;
		struct dirent *dp;
		dirp = opendir(dir_name);
		while ((dp = readdir(dirp)) != NULL) {

			if( strcmp( dp->d_name , "." ) == 0 || 
				strcmp( dp->d_name , "..") == 0    )
			continue;

			std::cout << "file_name = " << dp->d_name << std::endl;
			v.push_back(std::string(dp->d_name ));
		}
		(void) closedir(dirp);
		
		return 0;
	}

	void get_Devflow_log_list(int year, int mon , int day, vector<string>& v)
	{
		std::string date_path = BASIC_LOG_PATH + to_string(year) + "-" + to_string(mon) + "-" + to_string(day) + "/" + DEVFLOW_LOG_PATH;

		int i = access(date_path.c_str(), 0);
		
		if(i==0)
		{
			find_dir_file(date_path.c_str(),v);
		}
		else
		{

		}
	}

	void get_DevStatus_log_list(int year, int mon , int day, vector<string>& v)
	{
		std::string date_path = BASIC_LOG_PATH + to_string(year) + "-" + to_string(mon) + "-" + to_string(day) + "/" + DEVSTATUS_LOG_PATH;

		int i = access(date_path.c_str(), 0);
		
		if(i==0)
		{
			find_dir_file(date_path.c_str(),v);
		}
		else
		{

		}

	}

	int check_log_max(string path,string name)
	{
		//std::string date_path = BASIC_LOG_PATH + to_string(year) + "-" + to_string(mon) + "-" + to_string(day) + "/" + DEVSTATUS_LOG_PATH;
		//std::cout << "date_path222 = " << date_path << std::endl;
		//int i = access(date_path.c_str(), 0);
		//std::cout << "date_path = " << i << std::endl;
		
		std::string delim = ".";
		int nPos = 0;
		int max=0;
		
		DIR *dirp;
		struct dirent *dp;
		dirp = opendir(path.c_str());
		while ((dp = readdir(dirp)) != NULL) {

			std::string file_name = dp->d_name;
			nPos = file_name.find(delim.c_str());
			string temp = file_name.substr(0, nPos);
			file_name = file_name.substr(nPos+1);

			if(-1 == nPos)
				continue;
			
			if( strcmp( temp.c_str() , name.c_str()) == 0)
			{
				nPos = file_name.find(delim.c_str());
				string temp1 = file_name.substr(0, nPos);
				file_name = file_name.substr(nPos+1);

				max = max > std::stoi(temp1) ? max : std::stoi(temp1);
			}
		}
		//std::cout << "max = " << max << std::endl;
		(void) closedir(dirp);
		
		return max;

	}

	int read_log_file(string path)
	{
		//std::string date_path = BASIC_LOG_PATH + to_string(year) + "-" + to_string(mon) + "-" + to_string(day) + "/" + DEVSTATUS_LOG_PATH;
		//std::cout << "date_path222 = " << date_path << std::endl;
		//int i = access(path.c_str(), 0);

		ifstream log_file(path.c_str());

		if (!log_file.is_open()) 
		{ 
			std::cout << "open src File  Error opening file" << std::endl;
			return -1;
		}
		char* buffer = new char[1024];
		while (!log_file.eof()) 
		{  
			log_file.read(buffer, 1024);
			std::cout << buffer << std::endl;  
		}
		log_file.close();

		return 0;
	}
 
public:
	Log() 
	{
		struct tm *p;
		time_t time1 = time(0);
		p = localtime(&time1);
		std::string date_path = BASIC_LOG_PATH + to_string(p->tm_year+1900) + "-" + to_string((p->tm_mon+1)/10) + to_string((p->tm_mon+1)%10) + "-" + to_string(p->tm_mday/10) + to_string(p->tm_mday%10);
		int i = system(("mkdir -p " + date_path).c_str());

		//std::string Devflow_dir = date_path + "/" + DEVFLOW_LOG_PATH ;
		//int j = system(("mkdir -p " + Devflow_dir).c_str());
		std::string DevStatus_dir = date_path + "/" + DEVSTATUS_LOG_PATH ;
		int k = system(("mkdir -p " + DevStatus_dir).c_str());

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

		//std::string Devflow_path = date_path + "/" + DEVFLOW_LOG_PATH + "Devflow_" + to_string(p->tm_hour)+ ":" + to_string(p->tm_min);
		//std::cout << "Devflow_path = " << Devflow_path << std::endl;
		//device_status_logger = spdlog::rotating_logger_mt("警告信息", Devflow_path, 524288, 100); //循环日志，最大500K，最多100个文件
		//spdlog::flush_every(std::chrono::seconds(10)); //每10秒写一次log	

		std::string DevStatus_path = date_path + "/" + DEVSTATUS_LOG_PATH + "DevStatus_" + to_string(p->tm_hour/10) + to_string(p->tm_hour%10) + to_string(p->tm_min/10) + to_string(p->tm_min%10);
		device_status_logger = spdlog::rotating_logger_mt("设备状态", DevStatus_path, 1024*1024, 100); //循环日志，最大1M，最多100个文件
		device_status_logger->flush_on(spdlog::level::err);
		spdlog::flush_every(std::chrono::seconds(10)); //每10秒写一次log

		/*
		std::string directoryPath="/usr/local/bzl_robot/ewsr_ms/log/basket_control/";
		std::vector<string> fileNames;
		listDir(directoryPath.c_str(),fileNames,true);
	
		std::cout<<"total files: "<<fileNames.size()<<std::endl;
		std::cout<<fileNames[0]<<std::endl;
		std::cout<<fileNames[1]<<std::endl;
		std::cout<<fileNames[2]<<std::endl;
		//*/
		scanfiles(fileNames,total_logfile);
		//std::cout<<total_logfile<<std::endl;

		std::thread([this]() {
            while (1)
            {
                logfile_buff_check();
                sleep(10);
            }
        }).detach();
	}
	~Log() 
	{
		spdlog::drop_all();
	}
	Log(const Log&) = delete;
	Log& operator=(const Log&) = delete;
 
private:
   int total_logfile;
   std::vector<string> fileNames;	
   std::shared_ptr<spdlog::logger> console;
   std::shared_ptr<spdlog::logger> basic_logger;	
   std::shared_ptr<spdlog::logger> device_flow_logger;
   std::shared_ptr<spdlog::logger> device_status_logger;
   
};
 
 
#endif
