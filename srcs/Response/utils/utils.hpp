#ifndef UTILS_HPP
#define UTILS_HPP
#include <ctime>
#include <string>
#include <iomanip>
#include <sstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <utility>
#include <map>
#include <exception>

namespace ft
{
	struct 		UtilsGlobal
	{
		class ExtenstionNotFound: public std::exception
		{
			const char		*what() const throw();
		};
		class FileWithoutExtension: public std::exception
		{
			const char 		*what() const throw();	
		};
		static std::string 							DaysStr[];
		static std::string							MonthStr[];
		static std::pair<std::string, std::string> 	mimeTypes[];
		static size_t								mimeTypesSize;
	};
	std::string		getCurrentTime(void);
	std::string		getFileModifiedDate(const char*	path);
	std::string		getFileExtension(const std::string & file);
	std::string		getContentType(const std::string & file);
	off_t			getFileSize(const char * file);
}
#endif