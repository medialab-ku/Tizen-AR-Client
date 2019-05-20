#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#define TIZEN
#ifdef TIZEN
#include <app_common.h>
#endif
#include <string>

class FileSystem
{
public:
	static std::string GetResourcePath(std::string filename)
	{
#ifdef TIZEN
		std::string appResourcePath( app_get_resource_path() );
		return appResourcePath + filename;
#else
		std::string appResourcePath( "../res/" );
		return appResourcePath + filename;
#endif
	}

	static std::string GetDataPath(std::string filename)
	{
#ifdef TIZEN
		std::string appDataPath ( app_get_data_path() );
		return appDataPath + filename;
#else
		std::string appDataPath ( "" );
		return appDataPath + filename;
#endif
	}

private:
	FileSystem() {}
};

#endif
