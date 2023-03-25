//
//  RNBO_DataBuffer.cpp
//  RNBOApp
//
//  Created by Rob Sussman on 12/21/15.
//
//

#include "RNBO_DataBuffer.h"

namespace RNBO {

	DataBuffer::DataBuffer(size_t size)
	: _data(size)
	{

	}

	DataBuffer::DataBuffer(const char* dataToCopy, size_t sizeOfDataToCopy)
	: _data(sizeOfDataToCopy)
	{
		memcpy(_data.data(), dataToCopy, _data.size());
	}

	DataBuffer::DataBuffer(const char* stringToCopy)
	: _data(strlen(stringToCopy)+1)
	{
		memcpy(_data.data(), stringToCopy, _data.size());
	}

	void DataBuffer::resize(size_t size)
	{
		_data.resize(size);
	}


} // namespace RNBO
