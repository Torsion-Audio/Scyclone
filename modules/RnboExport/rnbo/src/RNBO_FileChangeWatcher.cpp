//
//  RNBO_FileChangeWatcher.cpp
//  RNBOApp
//
//  Created by Rob Sussman on 12/11/15.
//
//

#include "RNBO_FileChangeWatcher.h"

#if defined(USE_DYNAMIC_COMPILATION) && !defined(RNBO_NO_JUCE)

namespace RNBO {

	FileChangeWatcher::FileChangeWatcher(const char* fullPathToFile,
										 std::function<void(FileChangeWatcher*)> callback)
	: _fileToWatch(File(fullPathToFile))
	, _callback(callback)
	{
		// init _lastModificationTime to avoid an initial notification
		_lastModificationTime = _fileToWatch.getLastModificationTime();
		startTimer(1000);
	}

	FileChangeWatcher::~FileChangeWatcher()
	{

	}

	bool FileChangeWatcher::fileExists() const
	{
		return _fileToWatch.exists();
	}

	const char* FileChangeWatcher::getFullPathToWatchedFile() const
	{
		return _fileToWatch.getFullPathName().toRawUTF8();
	}

	void FileChangeWatcher::timerCallback()
	{
		Time modtime = _fileToWatch.getLastModificationTime();
		if (_lastModificationTime != modtime) {
			_lastModificationTime = modtime;
			_callback(this);
		}
	}

} // namespace RNBO

#endif // #ifdef USE_DYNAMIC_COMPILATION
