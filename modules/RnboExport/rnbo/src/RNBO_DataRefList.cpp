//
//  RNBO_DataRefList.cpp
//  RNBO
//
//  Created by Sam Tarakajian on 2/10/20.
//

#include "RNBO_DataRefList.h"

#ifndef RNBO_NOSTDLIB

RNBO_PUSH_DISABLE_WARNINGS
#include "3rdparty/json/json.hpp"
RNBO_POP_DISABLE_WARNINGS

using Json = nlohmann::json;

namespace RNBO {

DataRefList::DataRefList(std::string jsonString)
{
    Json json = Json::parse(jsonString);
    for (Json::iterator it = json.begin(); it != json.end(); it++) {
        if (it->is_object()) {
            Json& j = *it;

            if (j.contains("id") && (j.contains("file") || j.contains("url"))) {
                std::string rid = j["id"];
				std::string location = j.contains("file") ? j["file"] : j["url"];
				auto type = j.contains("file") ? DataRefType::File : DataRefType::URL;

                _ids.push_back(rid);
                _locations.push_back(location);
				_types.push_back(type);
            }
        }
    }
}

DataRefList::~DataRefList() {}

size_t DataRefList::size()
{
    return _ids.size();
}

std::string DataRefList::datarefIdAtIndex(size_t index)
{
	if (index > this->size()) return "";
    return _ids[index];
}

std::string DataRefList::datarefLocationAtIndex(size_t index)
{
	if (index > this->size()) return "";
    return _locations[index];
}

DataRefType DataRefList::datarefTypeAtIndex(size_t index)
{
	if (index > this->size()) return DataRefType::File;
	return _types[index];
}

}

#endif /* RNBO_NOPRESETS */
