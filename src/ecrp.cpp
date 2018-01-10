
#include <boost/asio.hpp>
#include <iostream>

#include "server.h"
#include "registry.h"
#include "processor.h"

//----------------------------------------------------------------------

bool runServer = false;
int localPort = 23712;
int processorCount = 12;
int maxPagesInMemory = -1;

bool tryReadArgument(std::string& output, char* argv, int k)
{
	try
	{
		output = std::string(&argv[k]);
		return true;
	}
	catch (std::exception&)
	{
		std::cerr << "Unable to parse argument as string after '" << argv[k - 1] << "'." << std::endl;
		exit(1001);
	}

	return false;
}

bool tryReadArgument(int& output, char* argv[], int k)
{
	try
	{
		output = atoi(argv[k]);
		return true;
	}
	catch (std::exception&)
	{
		std::cerr << "Unable to parse argument as integer after '" << argv[k - 1] << "'." << std::endl;
		exit(1002);
	}

	return false;
}

bool tryReadArgument(double& output, char* argv[], int k)
{
	try
	{
		output = atof(argv[k]);
		return true;
	}
	catch (std::exception&)
	{
		std::cerr << "Unable to parse argument as double after '" << argv[k - 1] << "'." << std::endl;
		exit(1003);
	}

	return false;
}

template<class T> bool tryReadArgument(T& output, int argc, char* argv[], int& k, const char* shortCommand, const char* longCommand)
{
	if ((shortCommand != 0 && strcmp(argv[k], shortCommand) == 0) || (longCommand != 0 && strcmp(argv[k], longCommand) == 0))
	{
		if (k + 1 < argc)
		{
			return tryReadArgument(output, argv, ++k);
		}
		else
		{
			std::cerr << "Missing argument after '" << argv[k] << "'." << std::endl;
			exit(1004);
		}
	}

	return false;
}

void parseArguments(int argc, char* argv[])
{
	for (int k = 1; k < argc; k++)
	{
		if (strcmp(argv[k], "run") == 0)
		{
			runServer = true;
			continue;
		}

		bool ok = false;
		ok = tryReadArgument(localPort, argc, argv, k, "-lp", "--localPort") || ok;
		ok = tryReadArgument(processorCount, argc, argv, k, "-pc", "--processorCount") || ok;
		ok = tryReadArgument(maxPagesInMemory, argc, argv, k, "-mp", "--maxPagesInMemory") || ok;

		if (!ok)
		{
			std::cerr << "Unknown argument '" << argv[k] << "'." << std::endl;
			exit(1005);
		}
	}

	if (localPort < 1024 || localPort > 65535)
	{
		std::cerr << "Argument 'localPort' out of range. It should be between 1024 and 65535." << std::endl;
		exit(1006);
	}

	if (processorCount < 1)
	{
		std::cerr << "Argument 'processorCount' out of range. It should be at least 1." << std::endl;
		exit(1007);
	}

	if (maxPagesInMemory != -1 && maxPagesInMemory < 256)
	{
		std::cerr << "Argument 'maxPagesInMemory' out of range. It should be at least 256 or -1 to disable the paging system." << std::endl;
		exit(1008);
	}
}

void printUsage()
{
	std::cout << "Usage: geodis run [OPTIONS]" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -lp / --localPort" << std::endl;
	std::cout << "  -pc / --processorCount" << std::endl;
	std::cout << "  -mp / --maxPagesInMemory" << std::endl;
}

int main(int argc, char* argv[])
{
	parseArguments(argc, argv);

	if (runServer)
	{
		try
		{
			geodis::registry::init(maxPagesInMemory);

			std::vector<geodis::processor*> processors;

			for (int k = 0; k < processorCount; k++)
			{
				processors.push_back(new geodis::processor());
			}

			boost::asio::io_service io_service;
			boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), localPort);

			geodis::server server(io_service, endpoint);

			std::cout << "Started geodis server on port " << localPort << " with " << processorCount << " processors." << std::endl;

			io_service.run();
		}
		catch (std::exception& e)
		{
			std::cerr << "EXCEPTION: " << e.what() << "\n";
		}
	}
	else
	{
		printUsage();
	}

	return 0;
}