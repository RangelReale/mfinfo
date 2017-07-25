#include "MFInfo.h"

#include <iostream>

int main(int argc, char* argv[])
{
	try
	{
		mfinfo::MediaInfo info;
		//info.load("M:\\VM\\data\\video\\f_50.bin.mp4");
		info.load("M:\\VM\\data\\video\\f_207152.bin.mp4");

		for (mfinfo::MediaInfo::videolist_t::iterator vi=info.videoList().begin(); vi!=info.videoList().end(); ++vi)
		{
			std::cout << "=== VIDEO INDEX " << (*vi)->index << " ===" << std::endl;
			std::cout << "    " << (*vi)->description << std::endl;
		}

		for (mfinfo::MediaInfo::audiolist_t::iterator ai=info.audioList().begin(); ai!=info.audioList().end(); ++ai)
		{
			std::cout << "=== AUDIO INDEX " << (*ai)->index << " ===" << std::endl;
			std::cout << "    " << (*ai)->description << std::endl;
		}
	}
	catch (std::exception &e)
	{
		std::cout << "ERROR: " << e.what() << std::endl;
	}

	std::cout << "PRESS ANY KEY TO CONTINUE" << std::endl;
	std::cin.ignore();
	return 0;
}