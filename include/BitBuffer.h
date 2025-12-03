#pragma once

#include <vector>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <sstream>
#include <cstring>
#include <array>

enum class EndianType { Little, Big };

class BitBuffer {
private:
	std::vector<uint8_t> data;
	size_t currentBit = 0;
	EndianType endian = EndianType::Little;

	void checkBounds(size_t nBits) const {
		if (currentBit + nBits > data.size() * 8) {
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

	void seekBits(int offset) { seekBits(offset, std::ios_base::cur); }

	void seekBits(int offset, std::ios_base::seekdir origin) {
		if (origin == std::ios_base::beg) currentBit = offset;
		else if (origin == std::ios_base::cur) currentBit += offset;
		else if (origin == std::ios_base::end) currentBit = data.size() * 8 - offset;

		if (currentBit < 0 || currentBit > data.size() * 8) {
			throw std::runtime_error("BitBuffer out of range");
		}
	}

	void seekBytes(int offset) { seekBits(offset * 8); }
	void seekBytes(int offset, std::ios_base::seekdir origin) { seekBits(offset * 8, origin); }

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

		uint32_t magnitude = readUnsignedBits(nBits - 1);
		bool sign = readBoolean();

		if (sign) {
			return -static_cast<int32_t>((1 << (nBits - 1)) - magnitude);
		} else {
			return static_cast<int32_t>(magnitude);
		}
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
		uint8_t bytes[4];
		auto b = readBytes(4);
		std::memcpy(bytes, b.data(), 4);
		float val;
		std::memcpy(&val, bytes, sizeof(float));
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
		size_t startBit = currentBit;
		std::string s = readString();
		size_t bitsRead = currentBit - startBit;
		size_t remainingBits = length * 8 - bitsRead;
		if (remainingBits > 0) seekBits(remainingBits);
		return s;
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

		float value = intValue + static_cast<float>(fractionValue) * 1.0f / 32.0f;
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
		for (size_t i = 0; i < nBits; ++i) {
			size_t byteIndex = currentBit / 8;
			size_t bitIndex = currentBit % 8;
			data[byteIndex] &= ~(1 << bitIndex);  // clear the bit
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
