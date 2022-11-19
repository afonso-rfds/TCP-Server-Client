#pragma once


#ifndef LOGFILE_HPP
#define LOGFILE_HPP

#include <iostream>
#include <fstream>

class LogFile
{
public:
	LogFile();
	~LogFile();

	void WriteString(std::string stringToWrite);
private:
	void CreateLogFile(std::string fileName);


	std::string m_fileName;
	std::ofstream m_file;

};
#endif // LOGFILE_HPP

