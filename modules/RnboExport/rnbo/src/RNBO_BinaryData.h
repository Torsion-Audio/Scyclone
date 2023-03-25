#ifndef RNBO_BINARYDATA_H
#define RNBO_BINARYDATA_H

#include <unordered_map>
#include <string>

namespace RNBO {
	class BinaryDataEntry {
		public:
			BinaryDataEntry() {}
			BinaryDataEntry(const std::string filename, const uint8_t * data, size_t length) : mFileName(filename), mData(data), mLength(length) {}
			BinaryDataEntry(const BinaryDataEntry&) = default;

			BinaryDataEntry & operator=(const BinaryDataEntry & other) {
				mFileName = other.mFileName;
				mData = other.mData;
				mLength = other.mLength;
				return *this;
			}

			const std::string& filename() const {
				return mFileName;
			}

			const uint8_t * data() const {
				return mData;
			}

			size_t length() const {
				return mLength;
			}
		private:
			std::string mFileName;
			const uint8_t * mData = nullptr;
			size_t mLength = 0;
	};

	class BinaryData {
		public:
			//TODO discover keys?
			virtual bool get(const std::string& key, BinaryDataEntry& out) const = 0;
	};

	class BinaryDataImpl : public BinaryData {
		public:
			using Storage = std::unordered_map<std::string, BinaryDataEntry>;
			BinaryDataImpl(Storage storage) : BinaryData(), mStorage(storage) {}
			BinaryDataImpl() : BinaryData() {} //empty

			bool get(const std::string& key, BinaryDataEntry& out) const override {
				auto it = mStorage.find(key);
				if (it != mStorage.end()) {
					out = it->second;
					return true;
				}
				return false;
			}

		private:
			Storage mStorage;
	};
}

#endif
