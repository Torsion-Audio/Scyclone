//
//  RNBO_DataBuffer.h
//
//  Created by Rob Sussman on 12/21/15.
//
//

#ifndef _RNBO_DataBuffer_H_
#define _RNBO_DataBuffer_H_

#include <vector>
#include <memory>

namespace RNBO {

	/**
	 * DataBuffer holds an arbitrary block of data.
	 *
	 * It is designed to be owned and used from one thread at a time and has no protections/locks on any of its methods.
	 * It is also designed to minimize memory allocation, so shrinking the buffer size will not cause reallocations.
	 */
	class DataBuffer
	{
	public:

		/**
		 * @brief Initialize a DataBuffer with zeros
		 *
		 * @param size the buffer size in bytes
		 */
		DataBuffer(size_t size);

		/**
		 * @brief Initialize a DataBuffer with a block of memory
		 *
		 * @param dataToCopy source data to copy
		 * @param sizeOfDataToCopy the number of bytes to copy from the source data
		 */
		DataBuffer(const char* dataToCopy, size_t sizeOfDataToCopy);

		/**
		 * @brief Initialize a DataBuffer with a null-terminated string
		 *
		 * The string's contents are copied to a newly allocated block of memory.
		 *
		 * @param stringToCopy the null-terminated string to initialize the DataBuffer with
		 */
		DataBuffer(const char* stringToCopy);

		/**
		 * @brief Resize the DataBuffer, attempting to preserve contents
		 *
		 * When expanding, the extra space is filled with zeros.
		 *
		 * @param size the new size of the DataBuffer
		 */
		void resize(size_t size);

		/**
		 * @brief Get the current buffer size
		 *
		 * @return the size of the DataBuffer in bytes
		 */
		size_t size() const { return _data.size(); }

		/**
		 * @brief Get a pointer to the raw data
		 *
		 * This can be used to modify the data in the buffer
		 *
		 * @return a pointer to the first byte of raw data in the DataBuffer
		 */
		char* data() { return _data.data(); }

		/**
		 * @brief Get a const pointer to the raw data
		 *
		 * @return a const pointer to the first byte of raw data in the DataBuffer
		 */
		const char* data() const { return _data.data(); }

	private:
		std::vector<char> _data;
	};

	using DataBufferRef = std::shared_ptr<DataBuffer>;

} // namespace RNBO

#endif // #ifndef _RNBO_DataBuffer_H_
