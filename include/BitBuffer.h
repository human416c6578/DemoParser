#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cstring>
#include <array>
#include <limits>

enum class EndianType { Little, Big };

class BitBuffer {
private:
	std::vector<uint8_t> data;
	size_t currentBit = 0;
	EndianType endian = EndianType::Little;

	void checkBounds(size_t nBits) const {
		const size_t totalBits = data.size() * 8;
		if (currentBit > totalBits || nBits > totalBits - currentBit) {
			throw std::runtime_error("BitBuffer out of range");
		}
	}

public:
	BitBuffer() = default;

	explicit BitBuffer(const std::vector<uint8_t>& inputData)
		: data(inputData) {}

	size_t length() const { return data.size(); }
	size_t bitsLeft() const { return data.size() * 8 - currentBit; }
	size_t bytesLeft() const { return data.size() - (currentBit / 8); }
	size_t currentByte() const { return currentBit / 8; }

	void setEndian(EndianType e) { endian = e; }
	EndianType getEndian() const { return endian; }

	void seekBits(std::int64_t offset) { seekBits(offset, std::ios_base::cur); }

	void seekBits(std::int64_t offset, std::ios_base::seekdir origin) {
		const size_t totalBits = data.size() * 8;
		const std::int64_t base =
			origin == std::ios_base::beg ? 0 :
			origin == std::ios_base::end ? static_cast<std::int64_t>(totalBits) :
			static_cast<std::int64_t>(currentBit);
		if (offset > 0 && base > std::numeric_limits<std::int64_t>::max() - offset)
			throw std::runtime_error("BitBuffer seek overflow");
		if (offset == std::numeric_limits<std::int64_t>::min() ||
			(offset < 0 && base < std::numeric_limits<std::int64_t>::min() - offset))
			throw std::runtime_error("BitBuffer seek overflow");
		const std::int64_t target = base + offset;

		if (target < 0 || static_cast<std::uint64_t>(target) > totalBits) {
			throw std::runtime_error("BitBuffer out of range");
		}

		currentBit = static_cast<size_t>(target);
	}

	void seekBytes(std::int64_t offset) {
		if ((offset > 0 && offset > std::numeric_limits<std::int64_t>::max() / 8) ||
			(offset < 0 && offset < std::numeric_limits<std::int64_t>::min() / 8))
			throw std::runtime_error("BitBuffer byte seek overflow");
		seekBits(offset * 8);
	}
	void seekBytes(std::int64_t offset, std::ios_base::seekdir origin) {
		if ((offset > 0 && offset > std::numeric_limits<std::int64_t>::max() / 8) ||
			(offset < 0 && offset < std::numeric_limits<std::int64_t>::min() / 8))
			throw std::runtime_error("BitBuffer byte seek overflow");
		seekBits(offset * 8, origin);
	}

	void skipRemainingBits() {
		size_t bitOffset = currentBit % 8;
		if (bitOffset != 0) seekBits(8 - bitOffset);
	}

	bool readBoolean() {
		checkBounds(1);
		size_t byteIndex = currentBit / 8;
		size_t bitIndex = currentBit % 8;
		bool value = false;

		if (endian == EndianType::Little) {
			value = (data[byteIndex] >> bitIndex) & 1;
		} else {
			value = (data[byteIndex] >> (7 - bitIndex)) & 1;
		}

		currentBit++;
		return value;
	}

	uint32_t readUnsignedBits(int nBits) {
		if (nBits <= 0 || nBits > 32) throw std::invalid_argument("nBits must be 1-32");
		checkBounds(nBits);

		uint32_t result = 0;

		if (endian == EndianType::Little) {
			for (int i = 0; i < nBits; ++i) {
				if (readBoolean()) result |= (1u << i);
			}
		} else {
			for (int i = 0; i < nBits; ++i) {
				if (readBoolean()) result |= (1u << (nBits - 1 - i));
			}
		}

		return result;
	}

	int32_t readBits(int nBits) {
		if (nBits <= 0 || nBits > 32) throw std::invalid_argument("nBits must be 1-32");

		uint32_t magnitude = nBits == 1 ? 0 : readUnsignedBits(nBits - 1);
		bool sign = readBoolean();

		const uint32_t signMagnitude = uint32_t{1} << (nBits - 1);
		const std::int64_t value = sign
			? -static_cast<std::int64_t>(signMagnitude - magnitude)
			: static_cast<std::int64_t>(magnitude);
		return static_cast<int32_t>(value);
	}

	uint8_t readByte() { return static_cast<uint8_t>(readUnsignedBits(8)); }
	int8_t readSByte() { return static_cast<int8_t>(readBits(8)); }

	std::vector<uint8_t> readBytes(size_t nBytes) {
		std::vector<uint8_t> result(nBytes);
		for (size_t i = 0; i < nBytes; ++i) {
			result[i] = readByte();
		}
		return result;
	}

	int16_t readInt16() { return static_cast<int16_t>(readBits(16)); }
	uint16_t readUInt16() { return static_cast<uint16_t>(readUnsignedBits(16)); }

	int32_t readInt32() { return readBits(32); }
	uint32_t readUInt32() { return readUnsignedBits(32); }

	float readFloat() {
		const auto bytes = readBytes(4);
		const uint32_t bits = static_cast<uint32_t>(bytes[0]) |
			(static_cast<uint32_t>(bytes[1]) << 8) |
			(static_cast<uint32_t>(bytes[2]) << 16) |
			(static_cast<uint32_t>(bytes[3]) << 24);
		float val = 0.0f;
		std::memcpy(&val, &bits, sizeof(val));
		return val;
	}

	std::string readString() {
		std::stringstream ss;
		while (true) {
			uint8_t b = readByte();
			if (b == 0) break;
			ss << static_cast<char>(b);
		}
		return ss.str();
	}

	std::string readString(size_t length) {
		if (length == 0) return {};
		if (length > std::numeric_limits<size_t>::max() / 8)
			throw std::invalid_argument("String length is too large");

		checkBounds(length * 8);

		std::string result;
		result.reserve(length);
		for (size_t i = 0; i < length; ++i) {
			const uint8_t byte = readByte();
			if (byte == 0) {
				seekBytes(static_cast<std::int64_t>(length - i - 1));
				return result;
			}
			result.push_back(static_cast<char>(byte));
		}

		return result;
	}

	std::array<float, 3> readVectorCoord() {
		bool xFlag = readBoolean();
		bool yFlag = readBoolean();
		bool zFlag = readBoolean();

		std::array<float, 3> result = {0.0f, 0.0f, 0.0f};

		if (xFlag) result[0] = readCoord();
		if (yFlag) result[1] = readCoord();
		if (zFlag) result[2] = readCoord();

		return result;
	}

	float readCoord() {
		bool intFlag = readBoolean();
		bool fractionFlag = readBoolean();
		if (!intFlag && !fractionFlag) return 0.0f;

		bool sign = readBoolean();
		uint32_t intValue = 0;
		uint32_t fractionValue = 0;

		if (intFlag) {
			intValue = readUnsignedBits(12);
		}

		if (fractionFlag) {
			fractionValue = readUnsignedBits(3);
		}

		float value = static_cast<float>(intValue) + static_cast<float>(fractionValue) * 1.0f / 32.0f;
		if (sign) value = -value;

		return value;
	}

	void insertBytes(const std::vector<uint8_t>& insertData) {
		if (currentBit % 8 != 0) throw std::runtime_error("InsertBytes must be byte-aligned");
		data.insert(data.begin() + currentByte(), insertData.begin(), insertData.end());
		currentBit += insertData.size() * 8;
	}

	void removeBytes(size_t count) {
		if (currentBit % 8 != 0) throw std::runtime_error("RemoveBytes must be byte-aligned");
		if (currentByte() + count > data.size()) throw std::runtime_error("RemoveBytes out of range");
		data.erase(data.begin() + currentByte(), data.begin() + currentByte() + count);
	}

	void zeroOutBits(size_t nBits) {
		checkBounds(nBits);
		for (size_t i = 0; i < nBits; ++i) {
			size_t byteIndex = currentBit / 8;
			size_t bitIndex = currentBit % 8;
			data[byteIndex] &= static_cast<uint8_t>(~(uint8_t{1} << bitIndex));  // clear the bit
			++currentBit;
		}
	}

	void printBits(std::ostream& out, size_t nBits) {
		for (size_t i = 0; i < nBits; ++i) {
			out << (readBoolean() ? '1' : '0');
		}
		out << "\n";
	}

	const std::vector<uint8_t>& getData() const { return data; }
};
