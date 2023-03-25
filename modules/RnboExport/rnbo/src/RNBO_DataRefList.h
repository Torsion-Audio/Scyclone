//
//  RNBO_DataRefList.h
//  RNBO
//
//  Created by Sam Tarakajian on 2/10/20.
//

#ifndef RNBO_DataRefList_h
#define RNBO_DataRefList_h

#include "RNBO_Utils.h"
#include <vector>

#ifndef RNBO_NOSTDLIB
namespace RNBO {
	enum class DataRefType {
		File,
		URL
	};

	/**
	 * Wrapper class for exported RNBO dependencies. After reading a
	 * dependencies.json file as a string, use this class to retrieve data ref
	 * description information.
	 *
	 * @code{.cpp}
	 * #include <fstream>
	 * #include <sstream>
	 *
	 * // Read in the dependencies.json file as a std::string
	 * std::ifstream t("../dependencies.json");
	 * std::stringstream buffer;
	 * buffer << t.rdbuf();
	 *
	 * // Parse dependencies into a RNBO DataRefList
	 * DataRefList list(buffer.str());
	 * @endcode
	 */
	class DataRefList {
	public:

		DataRefList(std::string jsonString);
		~DataRefList();

		/**
		 * Number of data refs in the list.
		 */
		size_t size();

		/**
		 * Get the ID of the data ref at the given index.
		 * @code{.cpp}
		 * std::string idstr = list.datarefIdAtIndex(i);
		 * @endcode
		 */
		std::string datarefIdAtIndex(size_t index);

		/**
		 * Get the path of the data ref at the given index. Can be a URL or a
		 * file path.
		 * @code{.cpp}
		 * std::string location = list.datarefLocationAtIndex(i);
		 * @endcode
		 */
		std::string datarefLocationAtIndex(size_t index);

		/**
		 * Get the RNBO::DataRefType of the data ref at the given index.
		 * @code{.cpp}
		 * DataRefType type = list.datarefTypeAtIndex(i);
		 * @endcode
		 */
		DataRefType datarefTypeAtIndex(size_t index);

	private:
		std::vector<std::string> _ids;
		std::vector<std::string> _locations;
		std::vector<DataRefType> _types;
	};
}
#endif /* RNBO_NOSTDLIB */

#endif /* RNBO_DataRefList_h */
