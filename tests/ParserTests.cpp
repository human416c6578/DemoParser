#include <BitBuffer.h>
#include <demoanalyser/DeltaParsers.h>
#include <demoanalyser/DemoParser.h>

#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void expect(bool condition, const char* message)
{
    if (!condition) {
        std::cerr << "Test failure: " << message << '\n';
        std::exit(1);
    }
}

void putUInt32LE(std::vector<uint8_t>& data, size_t offset, uint32_t value)
{
    expect(offset + 4 <= data.size(), "write exceeds test buffer");
    data[offset] = static_cast<uint8_t>(value);
    data[offset + 1] = static_cast<uint8_t>(value >> 8);
    data[offset + 2] = static_cast<uint8_t>(value >> 16);
    data[offset + 3] = static_cast<uint8_t>(value >> 24);
}

void appendUInt32LE(std::vector<uint8_t>& data, uint32_t value)
{
    data.push_back(static_cast<uint8_t>(value));
    data.push_back(static_cast<uint8_t>(value >> 8));
    data.push_back(static_cast<uint8_t>(value >> 16));
    data.push_back(static_cast<uint8_t>(value >> 24));
}

void putFixedString(std::vector<uint8_t>& data, size_t offset, size_t length,
                    const std::string& value)
{
    expect(value.size() < length, "test string must fit in fixed field");
    expect(offset + length <= data.size(), "fixed string exceeds test buffer");
    for (size_t i = 0; i < value.size(); ++i)
        data[offset + i] = static_cast<uint8_t>(value[i]);
}

void appendDirectory(std::vector<uint8_t>& data)
{
    appendUInt32LE(data, 2);
    for (int i = 0; i < 2; ++i) {
        appendUInt32LE(data, 0);
        data.insert(data.end(), 64, 0);
        appendUInt32LE(data, 0);
        appendUInt32LE(data, 0);
        appendUInt32LE(data, 0);
        appendUInt32LE(data, 0);
        appendUInt32LE(data, 0);
        appendUInt32LE(data, 0);
    }
}

std::vector<uint8_t> makeHeader(size_t directoryOffset, size_t totalSize)
{
    std::vector<uint8_t> data(totalSize, 0);
    putFixedString(data, 0, 8, "HLDEMO");
    putUInt32LE(data, 8, 5);
    putUInt32LE(data, 12, 43);
    putFixedString(data, 16, 260, "test_map");
    putFixedString(data, 276, 260, "cstrike");
    putUInt32LE(data, 536, 0);
    putUInt32LE(data, 540, static_cast<uint32_t>(directoryOffset));
    return data;
}

void addFrameHeader(std::vector<uint8_t>& data, uint8_t type)
{
    data.push_back(type);
    appendUInt32LE(data, 0);
    appendUInt32LE(data, 0);
}

void addDirectoryMarkers(std::vector<uint8_t>& data)
{
    addFrameHeader(data, 5);
    addFrameHeader(data, 5);
}

void writeDemo(const std::filesystem::path& path, std::vector<uint8_t> data)
{
    std::ofstream output(path, std::ios::binary);
    expect(output.is_open(), "could not create test demo");
    output.write(reinterpret_cast<const char*>(data.data()),
                 static_cast<std::streamsize>(data.size()));
    expect(static_cast<bool>(output), "could not write test demo");
}

void testBitBuffer()
{
    BitBuffer fixedString({static_cast<uint8_t>('a'), static_cast<uint8_t>('b'), 0, 'x'});
    expect(fixedString.readString(4) == "ab", "fixed-width string value is incorrect");
    expect(fixedString.currentByte() == 4, "fixed-width string did not consume its field");

    BitBuffer unterminated({'a', 'b', 'c'});
    expect(unterminated.readString(3) == "abc", "full fixed-width string was truncated");

    BitBuffer signedOne({1});
    expect(signedOne.readBits(1) == -1, "one-bit signed value was not decoded");

    BitBuffer seekable({0});
    seekable.seekBits(8);
    seekable.seekBits(-8);
    expect(seekable.currentByte() == 0, "negative bit seek failed");
}

void testDeltaConversions()
{
    HalfLifeDelta delta;
    delta.addEntry("flags");
    delta.setEntryValue("flags", static_cast<uint8_t>(7));
    ClientData clientData = toClientData(delta);
    expect(clientData.flags == 7, "narrow integer delta was discarded");

    HalfLifeDelta incompleteEntity;
    EntityStatePlayer entity = toEntityStatePlayer(incompleteEntity);
    expect(entity.sequence == 0, "missing entity fields were not defaulted");
}

void testMinimalDemo(const std::filesystem::path& path)
{
    const size_t directoryOffset = 544 + 18;
    std::vector<uint8_t> data = makeHeader(directoryOffset, directoryOffset + 188);
    data.resize(544);
    addDirectoryMarkers(data);
    data.resize(directoryOffset, 0);
    std::vector<uint8_t> directory;
    appendDirectory(directory);
    data.insert(data.end(), directory.begin(), directory.end());

    writeDemo(path, data);
    demo_analyser::DemoParser parser(path.string());
    parser.parseDemo();
}

void testUnknownMessageDoesNotAbort(const std::filesystem::path& path)
{
    const size_t payloadStart = 1021;
    const size_t directoryOffset = payloadStart + 1 + 18;
    std::vector<uint8_t> data = makeHeader(directoryOffset, directoryOffset + 188);
    data.resize(544);
    addFrameHeader(data, 0);
    data.resize(payloadStart, 0);
    putUInt32LE(data, 1017, 1);
    data.push_back(59);
    addDirectoryMarkers(data);
    std::vector<uint8_t> directory;
    appendDirectory(directory);
    data.insert(data.end(), directory.begin(), directory.end());

    writeDemo(path, data);
    demo_analyser::DemoParser parser(path.string());
    parser.parseDemo();
}

void testOversizedFrameIsRejected(const std::filesystem::path& path)
{
    const size_t payloadStart = 1021;
    const size_t directoryOffset = payloadStart + 18;
    std::vector<uint8_t> data = makeHeader(directoryOffset, directoryOffset + 188);
    data.resize(544);
    addFrameHeader(data, 0);
    data.resize(payloadStart, 0);
    putUInt32LE(data, 1017, UINT32_MAX);
    addDirectoryMarkers(data);
    std::vector<uint8_t> directory;
    appendDirectory(directory);
    data.insert(data.end(), directory.begin(), directory.end());

    writeDemo(path, data);
    bool threw = false;
    try {
        demo_analyser::DemoParser parser(path.string());
        parser.parseDemo();
    } catch (const std::runtime_error&) {
        threw = true;
    }
    expect(threw, "oversized game data frame was not rejected");
}

void testMissingEndMarkerIsRejected(const std::filesystem::path& path)
{
    const size_t directoryOffset = 544 + 9;
    std::vector<uint8_t> data = makeHeader(directoryOffset, directoryOffset + 188);
    data.resize(544);
    addFrameHeader(data, 5);
    std::vector<uint8_t> directory;
    appendDirectory(directory);
    data.insert(data.end(), directory.begin(), directory.end());

    writeDemo(path, data);
    bool threw = false;
    try {
        demo_analyser::DemoParser parser(path.string());
        parser.parseDemo();
    } catch (const std::runtime_error&) {
        threw = true;
    }
    expect(threw, "demo without an end marker was accepted");
}

void testMissingFile()
{
    bool threw = false;
    try {
        demo_analyser::DemoParser parser("/path/that/does/not/exist.dem");
        static_cast<void>(parser);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    expect(threw, "missing demo file was not rejected");
}

} // namespace

int main()
{
    testBitBuffer();
    testDeltaConversions();
    testMissingFile();

    const std::filesystem::path minimalPath =
        std::filesystem::temp_directory_path() / "demo_parser_minimal.dem";
    const std::filesystem::path unsupportedPath =
        std::filesystem::temp_directory_path() / "demo_parser_unsupported.dem";
    const std::filesystem::path oversizedPath =
        std::filesystem::temp_directory_path() / "demo_parser_oversized.dem";
    const std::filesystem::path missingMarkerPath =
        std::filesystem::temp_directory_path() / "demo_parser_missing_marker.dem";

    testMinimalDemo(minimalPath);
    testUnknownMessageDoesNotAbort(unsupportedPath);
    testOversizedFrameIsRejected(oversizedPath);
    testMissingEndMarkerIsRejected(missingMarkerPath);

    std::filesystem::remove(minimalPath);
    std::filesystem::remove(unsupportedPath);
    std::filesystem::remove(oversizedPath);
    std::filesystem::remove(missingMarkerPath);
    return 0;
}
