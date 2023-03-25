//
//  RNBO_FileChangeWatcher.h
//
//  Created by Rob Sussman on 12/11/15.
//
//

#ifndef _RNBO_FileChangeWatcher_H_
#define _RNBO_FileChangeWatcher_H_

#include "RNBO_Config.h"

#ifdef USE_DYNAMIC_COMPILATION

#include <functional>

#ifndef RNBO_NO_JUCE
#include "JuceHeader.h"
#endif

namespace RNBO {

#ifdef RNBO_NO_JUCE
	class FileChangeWatcher
	{
	public:
		FileChangeWatcher(const char* fullPathToFile,
						  std::function<void(FileChangeWatcher*)> callback)
		{
			// dummy implementation
		}
	};
#else
	class FileChangeWatcher : public Timer
	{
	public:

		/** FileChangeWatcher will call the callback when the file changes.
			Passes in a pointer to the watcher so you can query the path.
		 */
		FileChangeWatcher(const char* fullPathToFile,
						  std::function<void(FileChangeWatcher*)> callback);
		~FileChangeWatcher();

		bool fileExists() const;
		const char* getFullPathToWatchedFile() const;

		void timerCallback() override;

	private:
		File										_fileToWatch;
		Time										_lastModificationTime;
		std::function<void(FileChangeWatcher*)>		_callback;
	};
#endif // #ifdef RNBO_NO_JUCE

} // namespace RNBO

#endif // #ifdef USE_DYNAMIC_COMPILATION

#endif // #ifndef _RNBO_FileChangeWatcher_H_
