#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <variant>
#include <stdexcept>
#include <cstdint>
#include <BitBuffer.h>

// Variant type to store delta values
using DeltaValue = std::variant<int8_t, uint8_t, int16_t, uint16_t, int32_t, uint32_t, float, std::string>;

inline float toFloat(const DeltaValue& value) {
    return std::visit([](auto&& arg) -> float {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_arithmetic_v<T>) {  // numeric types
            return static_cast<float>(arg);       // convert to float
        } else {
            throw std::runtime_error("Cannot convert non-numeric type to float");
        }
    }, value);
}

class HalfLifeDelta {
private:
    struct Entry {
        std::string name;
        DeltaValue value;
    };

    std::vector<Entry> entries;
    std::unordered_map<std::string, size_t> nameIndexMap;

public:
    HalfLifeDelta(size_t nEntries = 0) { entries.reserve(nEntries); }

    void addEntry(const std::string& name) {
        entries.push_back({name, DeltaValue{}});
        nameIndexMap[name] = entries.size() - 1;
    }

    DeltaValue* findEntryValue(const std::string& name) {
        auto it = nameIndexMap.find(name);
        if (it == nameIndexMap.end()) return nullptr;
        return &entries[it->second].value;
    }

    const DeltaValue* findEntryValue(const std::string& name) const {
        auto it = nameIndexMap.find(name);
        if (it == nameIndexMap.end()) return nullptr;
        return &entries[it->second].value;
    }

    void setEntryValue(const std::string& name, const DeltaValue& value) {
        DeltaValue* v = findEntryValue(name);
        if (!v) throw std::runtime_error("Delta entry not found: " + name);
        *v = value;
    }

    void setEntryValue(size_t index, const DeltaValue& value) {
        if (index >= entries.size()) throw std::out_of_range("Delta entry index out of range");
        entries[index].value = value;
    }

    size_t size() const { return entries.size(); }
};


class HalfLifeDeltaStructure {
public:
    enum class EntryFlags : uint32_t {
        Byte = 1 << 0,
        Short = 1 << 1,
        Float = 1 << 2,
        Integer = 1 << 3,
        Angle = 1 << 4,
        TimeWindow8 = 1 << 5,
        TimeWindowBig = 1 << 6,
        String = 1 << 7,
        Signed = 1u << 31
    };

    struct Entry {
        std::string name;
        uint32_t nBits;
        float divisor;
        EntryFlags flags;
        float preMultiplier = 1.0f;
    };

private:
    std::string name;
    std::vector<Entry> entryList;

public:
    HalfLifeDeltaStructure(const std::string& name_) : name(name_) {}

    const std::string& getName() const { return name; }

    void addEntry(const std::string& entryName, uint32_t nBits, float divisor, EntryFlags flags) {
        entryList.push_back({entryName, nBits, divisor, flags, 1.0f});
    }

    void addEntry(const HalfLifeDelta& delta) {
        const DeltaValue* nameVal    = delta.findEntryValue("name");
        const DeltaValue* nBitsVal   = delta.findEntryValue("nBits");
        const DeltaValue* divisorVal = delta.findEntryValue("divisor");
        const DeltaValue* flagsVal   = delta.findEntryValue("flags");

        // Helper lambda to safely extract a value from DeltaValue
        auto extractString = [](const DeltaValue* val, const std::string& key) -> std::string {
            if (!val) throw std::runtime_error("Missing entry: " + key);
            if (auto ptr = std::get_if<std::string>(val)) return *ptr;
            return ""; // treat missing or wrong type as empty
        };

        auto extractUInt32 = [](const DeltaValue* val, const std::string& key) -> uint32_t {
            if (!val) return 0;
            if (auto ptr = std::get_if<uint32_t>(val)) return *ptr;
            if (auto ptr = std::get_if<int32_t>(val)) return static_cast<uint32_t>(*ptr);
            return 0; // fallback
        };

        auto extractFloat = [](const DeltaValue* val, const std::string& key) -> float {
            if (!val) return 1.0f; // default divisor
            if (auto ptr = std::get_if<float>(val)) return *ptr;
            if (auto ptr = std::get_if<uint32_t>(val)) return static_cast<float>(*ptr);
            if (auto ptr = std::get_if<int32_t>(val)) return static_cast<float>(*ptr);
            return 1.0f; // fallback
        };

        std::string name    = extractString(nameVal, "name");
        uint32_t nBits      = extractUInt32(nBitsVal, "nBits");
        float divisor       = extractFloat(divisorVal, "divisor");
        HalfLifeDeltaStructure::EntryFlags flags = static_cast<HalfLifeDeltaStructure::EntryFlags>(
            extractUInt32(flagsVal, "flags")
        );

        addEntry(name, nBits, divisor, flags);
    }


    HalfLifeDelta createDelta() const {
        HalfLifeDelta delta(entryList.size());
        for (const auto& e : entryList) delta.addEntry(e.name);
        return delta;
    }

    void readDelta(BitBuffer& buf) const {
        
        readDelta(buf, nullptr);
    }

    void readDelta(BitBuffer& bitBuffer, HalfLifeDelta* delta) const {

        std::vector<uint8_t> bitmaskBytes;
        // read 3-bit unsigned value for the number of bitmask bytes
        uint32_t nBitmaskBytes = bitBuffer.readUnsignedBits(3);

        if (nBitmaskBytes == 0) {
            bitmaskBytes.clear();
            return;
        }

        bitmaskBytes.resize(nBitmaskBytes);

        // read the bitmask bytes
        for (uint32_t i = 0; i < nBitmaskBytes; ++i) {
            bitmaskBytes[i] = bitBuffer.readByte();
        }

        // iterate through each bit in each byte
        for (uint32_t i = 0; i < nBitmaskBytes; ++i) {
            for (uint32_t j = 0; j < 8; ++j) {
                uint32_t index = j + i * 8;
                if (index >= entryList.size()) return;

                // check if this field is present
                if (bitmaskBytes[i] & (1 << j)) {
                    // parse the entry
                    DeltaValue value = parseEntry(bitBuffer, entryList[index]);

                    // assign value if delta is provided
                    if (delta) {
                        delta->setEntryValue(index, value);
                    }
                }
            }
        }
    }

private:
    DeltaValue parseEntry(BitBuffer& bitBuffer, const Entry& e) const {
        bool isSigned = static_cast<uint32_t>(e.flags) & static_cast<uint32_t>(EntryFlags::Signed);

        if (static_cast<uint32_t>(e.flags) & static_cast<uint32_t>(EntryFlags::Byte)) {
            return isSigned ? static_cast<int8_t>(parseInt(bitBuffer, e))
                            : static_cast<uint8_t>(parseUnsignedInt(bitBuffer, e));
        }

        if (static_cast<uint32_t>(e.flags) & static_cast<uint32_t>(EntryFlags::Short)) {
            return isSigned ? static_cast<int16_t>(parseInt(bitBuffer, e))
                            : static_cast<uint16_t>(parseUnsignedInt(bitBuffer, e));
        }

        if (static_cast<uint32_t>(e.flags) & static_cast<uint32_t>(EntryFlags::Integer)) {
            return isSigned ? parseInt(bitBuffer, e)
                            : parseUnsignedInt(bitBuffer, e);
        }

        if (static_cast<uint32_t>(e.flags) & static_cast<uint32_t>(EntryFlags::Float) ||
            static_cast<uint32_t>(e.flags) & static_cast<uint32_t>(EntryFlags::TimeWindow8) ||
            static_cast<uint32_t>(e.flags) & static_cast<uint32_t>(EntryFlags::TimeWindowBig)) {
            bool negative = false;
            int bitsToRead = static_cast<int>(e.nBits);

            if (isSigned) {
                negative = bitBuffer.readBoolean();
                bitsToRead--;
            }

            float value = static_cast<float>(bitBuffer.readUnsignedBits(bitsToRead)) / e.divisor;
            return negative ? -value : value;
        }

        if (static_cast<uint32_t>(e.flags) & static_cast<uint32_t>(EntryFlags::Angle)) {
            return static_cast<float>(bitBuffer.readUnsignedBits(static_cast<int>(e.nBits)) * (360.0f / static_cast<float>(1 << e.nBits)));
        }

        if (static_cast<uint32_t>(e.flags) & static_cast<uint32_t>(EntryFlags::String)) {
            return bitBuffer.readString();
        }

        throw std::runtime_error("Unknown delta entry type");
    }

    int32_t parseInt(BitBuffer& bitBuffer, const Entry& e) const {
        bool negative = bitBuffer.readBoolean();
        int32_t val = static_cast<int32_t>(bitBuffer.readUnsignedBits(e.nBits - 1));
        val = static_cast<int32_t>(val / e.divisor);
        return negative ? -val : val;
    }

    uint32_t parseUnsignedInt(BitBuffer& bitBuffer, const Entry& e) const {
        uint32_t val = bitBuffer.readUnsignedBits(e.nBits);
        return val / static_cast<uint32_t>(e.divisor);
    }
};
