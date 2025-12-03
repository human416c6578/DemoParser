#include "BitBuffer.h"
#include "HalfLifeDeltas.h"
#include "demoanalyser/EventHandlers.h"
#include <demoanalyser/DemoParser.h>
#include <demoanalyser/DeltaParsers.h>

#include <cstdint>
#include <fstream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <iostream>

namespace demo_analyser
{

	DemoParser::DemoParser(const std::string& path)
		: file(path, std::ios::binary | std::ios::ate)
	{	
		if (!file.is_open()) {
			std::cerr << "Error opening file: " << path << "\n";
		}

		{
			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_BAD),
				0,
				nullptr 
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_DELTADESCRIPTION),
				0,
				[this]() { MessageDeltaDescription(); }
			);


			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_PRINT),
				0,
				[this]() { MessagePrint(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_SERVERINFO),
				0,
				[this]() { MessageServerInfo(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_SENDEXTRAINFO),
				0,
				[this]() { MessageExtraInfo(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_NEWMOVEVARS),
				0,
				[this]() { MessageNewMoveVars(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_CDTRACK),
				2,
				nullptr
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_SETVIEW),
				2,
				nullptr
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_NEWUSERMSG),
				0,
				[this]() { MessageNewUserMsg(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_STUFFTEXT),
				0,
				[this]() { MessagePrint(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_UPDATEUSERINFO),
				0,
				[this]() { MessageUpdateUserInfo(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_NOP),
				0,
				nullptr
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_RESOURCEREQUEST),
				8,
				nullptr
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_RESOURCELOCATION),
				0,
				[this]() { MessagePrint(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_RESOURCELIST),
				0,
				[this]() { MessageResourceList(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_ROOMTYPE),
				2,
				nullptr
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_SPAWNBASELINE),
				0,
				[this]() { MessageSpawnBaseline(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_TIME),
				0,
				[this]() { MessageTime(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_LIGHTSTYLE),
				0,
				[this]() { MessageLightStyle(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_SETANGLE),
				0,
				[this]() { MessageSetAngle(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_CHOKE),
				0,
				nullptr
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_CLIENTDATA),
				0,
				[this]() { MessageClientData(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_SIGNONNUM),
				1,
				nullptr
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_VOICEINIT),
				0,
				[this]() { MessageVoiceInit(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_CUSTOMIZATION),
				0,
				[this]() { MessageCustomization(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_SENDCVARVALUE2),
				0,
				[this]() { MessageSendCvarValue2(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_PACKETENTITIES),
				0,
				[this]() { MessagePacketEntities(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_TEMPENTITY),
				0,
				[this]() { MessageTempEntity(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_DELTAPACKETENTITIES),
				0,
				[this]() { MessageDeltaPacketEntities(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_SOUND),
				0,
				[this]() { MessageSound(); }
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_WEAPONANIM),
				2,
				nullptr
			);

			AddMessageHandler(
				static_cast<uint8_t>(SVCMessage::SVC_PINGS),
				0,
				[this]() { MessagePing(); }
			);
		}
		auto deltaDescription = std::make_unique<HalfLifeDeltaStructure>("delta_description_t");

		deltaDescription->addEntry("flags", 32, 1.0f, HalfLifeDeltaStructure::EntryFlags::Integer);
		deltaDescription->addEntry("name", 8, 1.0f, HalfLifeDeltaStructure::EntryFlags::String);
		deltaDescription->addEntry("offset", 16, 1.0f, HalfLifeDeltaStructure::EntryFlags::Integer);
		deltaDescription->addEntry("size", 8, 1.0f, HalfLifeDeltaStructure::EntryFlags::Integer);
		deltaDescription->addEntry("nBits", 8, 1.0f, HalfLifeDeltaStructure::EntryFlags::Integer);
		deltaDescription->addEntry("divisor", 32, 4000.0f, HalfLifeDeltaStructure::EntryFlags::Float);
		deltaDescription->addEntry("preMultiplier", 32, 4000.0f, HalfLifeDeltaStructure::EntryFlags::Float);

		AddDeltaStructure(std::move(deltaDescription));
		
	}

	void DemoParser::parseDemo()
	{
		try {
			file.seekg(0, std::ios::end);
			std::streampos pos = file.tellg();
			if (pos == std::streampos(-1)) throw std::runtime_error("Failed to get file size");
			size_t fileSize = static_cast<size_t>(pos);
			file.seekg(0, std::ios::beg);

			std::vector<uint8_t> headerData(544);
			file.read(reinterpret_cast<char*>(headerData.data()), headerData.size());
			
			readDemoHeader(file, headerData, fileSize);

			Seek(544, std::ios::beg);
			uint8_t currentDirectory = 0;

			while (true) {
				FrameHeader frameHeader = ReadFrameHeader();

				// "no loading segment" bug
				// if (frameHeader.Type == 1 && serverInfoParsed) 
				// {
				// 	break;
				// }

				if (frameHeader.Type == 0 || frameHeader.Type == 1) 
				{
					GameDataFrameHeader gameDataFrameHeader = ReadGameDataFrameHeader();

					if (gameDataFrameHeader.Length > 0) {
						// Read frame data
						std::vector<uint8_t> frameData(gameDataFrameHeader.Length);
						file.read(reinterpret_cast<char*>(frameData.data()), gameDataFrameHeader.Length);

						try {
							ParseGameDataMessages(frameData);
						} catch (const std::exception& ex) {
							throw std::runtime_error(std::string("Error parsing gamedata frame: ") + ex.what());
						}
					}
				} else if (frameHeader.Type == 3)
				{

					std::vector<uint8_t> frameData(64);
					file.read(reinterpret_cast<char*>(frameData.data()), frameData.size());

					bitBuffer = std::make_unique<BitBuffer>(frameData);

					std::string command = bitBuffer->readString(64);
					if(OnConsoleCommand)
						OnConsoleCommand(command);

				} else  if(frameHeader.Type == 4)
				{
					try{
						std::vector<uint8_t> frameData(32);
						file.read(reinterpret_cast<char*>(frameData.data()), 32);

						bitBuffer = std::make_unique<BitBuffer>(frameData);

						PlayerState state;

						state.position[0] = bitBuffer->readFloat();
						state.position[1] = bitBuffer->readFloat();
						state.position[2] = bitBuffer->readFloat();

						state.rotation[0] = bitBuffer->readFloat();
						state.rotation[1] = bitBuffer->readFloat();
						state.rotation[2] = bitBuffer->readFloat();

						state.weaponFlags = bitBuffer->readUInt32();
						state.fov = bitBuffer->readFloat();


						if(OnPlayerState)
							OnPlayerState(state);
						
						//SkipFrame(frameHeader.Type);
						} catch (const std::exception& ex) {
							throw std::runtime_error(std::string("Error parsing gamedata frame: ") + ex.what());
						}
				}
				else if(frameHeader.Type == 5)
				{
					if(currentDirectory == 1)
						break;

					currentDirectory++;
				}
				else if (frameHeader.Type == 6)
				{
					std::vector<uint8_t> frameData(84);
					file.read(reinterpret_cast<char*>(frameData.data()),frameData.size() );

					EventFrame eventFrame = ParseEventFrame(frameData);

					if(OnEventFrame)
						OnEventFrame(eventFrame);
				}
				else 
				{
					SkipFrame(frameHeader.Type);
				}
			}
		} catch (...) 
		{
			throw; // rethrow exception
		}
	}

	void DemoParser::readDemoHeader(std::ifstream &file, const std::vector<uint8_t>& headerData, const uint32_t fileSize) 
	{
		DemoHeader header;

		bitBuffer = std::make_unique<BitBuffer>(headerData);
		
		// Skip magic
		bitBuffer->readBytes(8);

		header.demoProtocol = bitBuffer->readUInt32();
		if (header.demoProtocol != 5)
			throw std::runtime_error("Unknown demo protocol, expected 5");

		header.networkProtocol = bitBuffer->readUInt32();
		if (header.networkProtocol < 43)
			throw std::runtime_error("Unsupported network protocol, must be >= 43");

		header.mapName = bitBuffer->readString(260);

		header.gameFolderName = bitBuffer->readString(260);

		header.mapChecksum = bitBuffer->readUInt32();

		header.directoryOffset = bitBuffer->readUInt32();

		constexpr size_t DirectoryEntrySize = 92;
		uint32_t expectedOffset = static_cast<uint32_t>(fileSize - 4 - (DirectoryEntrySize * 2));

		if (header.directoryOffset != expectedOffset) {
			std::cerr << "Read offset: " << header.directoryOffset
					<< " (0x" << std::hex << header.directoryOffset << std::dec << ")\n";

			std::cerr << "Expected offset: " << expectedOffset
					<< " (0x" << std::hex << expectedOffset << std::dec << ")\n";
			
			throw std::runtime_error("Unexpected directory entries offset");
		}
		
		file.seekg(header.directoryOffset, std::ios::beg);
		
		std::vector<uint8_t> directoryEntriesData(4 + 2 * 92);
		file.read(reinterpret_cast<char*>(directoryEntriesData.data()), directoryEntriesData.size());
		bitBuffer = std::make_unique<BitBuffer>(directoryEntriesData);

		int32_t nDirectoryEntries = 0;
		nDirectoryEntries = bitBuffer->readInt32();
		
		if (nDirectoryEntries != 2)
			throw std::runtime_error("Corrupt number of directory entries");

		for (int i = 0; i < nDirectoryEntries; ++i) 
		{
			header.demoDirectory[i].type = bitBuffer->readInt32(); // type
			header.demoDirectory[i].description = bitBuffer->readString(64);
			header.demoDirectory[i].flags = bitBuffer->readInt32(); // flags, 
			header.demoDirectory[i].CDTrack = bitBuffer->readInt32(); // cdtrack
			header.demoDirectory[i].trackTime = bitBuffer->readFloat();

			header.demoDirectory[i].frameCount = bitBuffer->readInt32(); // frames
			header.demoDirectory[i].offset = bitBuffer->readInt32(); // offset
			header.demoDirectory[i].fileLength = bitBuffer->readInt32(); // length
		}

		if(OnReadHeader)
			OnReadHeader(header);
	}

	FrameHeader DemoParser::ReadFrameHeader()
	{
		FrameHeader header{};

		file.read(reinterpret_cast<char*>(&header.Type), sizeof(header.Type));

		file.read(reinterpret_cast<char*>(&header.Timestamp), sizeof(header.Timestamp));

		file.read(reinterpret_cast<char*>(&header.Number), sizeof(header.Number));

		return header;
	}

	int32_t DemoParser::GetFrameLength(uint8_t frameType) 
	{
		int32_t length = 0;

		switch (frameType) {
			case 2:
				// TODO: unknown type
				break;

			case 3: // client command
				length = 64;
				break;

			case 4:
				length = 32;
				break;

			case 5: // end of segment
				break;

			case 6:
				length = 84;
				break;

			case 7:
				length = 8;
				break;

			case 8: {
				file.seekg(4, std::ios::cur);
				int32_t val;
				file.read(reinterpret_cast<char*>(&val), sizeof(val));
				file.seekg(-8, std::ios::cur);
				length = val + 24;
				break;
			}

			case 9: {
				int32_t val;
				file.read(reinterpret_cast<char*>(&val), sizeof(val));
				length = 4 + val;
				file.seekg(-4, std::ios::cur);
				break;
			}

			default:
				throw std::runtime_error("Unknown frame type.");
		}

		return length;
	}

	void DemoParser::SkipFrame(uint8_t frameType) 
	{
		int32_t length = GetFrameLength(frameType);
		file.seekg(length, std::ios::cur);
	}

	GameDataFrameHeader DemoParser::ReadGameDataFrameHeader() 
	{
		GameDataFrameHeader header{};

		Seek(220);
		file.read(reinterpret_cast<char*>(&header.ResolutionWidth), sizeof(header.ResolutionWidth));
		file.read(reinterpret_cast<char*>(&header.ResolutionHeight), sizeof(header.ResolutionHeight));


		Seek(236);


		file.read(reinterpret_cast<char*>(&header.Length), sizeof(header.Length));

		return header;
	}

	void DemoParser::ParseGameDataMessages(const std::vector<uint8_t>& frameData)
	{
		int64_t gameDataStartOffset = file.tellg() - static_cast<std::streamoff>(frameData.size());

		// load bit buffer
		bitBuffer = std::make_unique<BitBuffer>(frameData);
		readingGameData = true;

		try {
			while (true)
			{
				int32_t messageFrameOffset = bitBuffer->currentByte();
				uint8_t messageId = bitBuffer->readByte();

				std::string messageName = SVCMessageName(messageId);
				if (messageName.empty())
					messageName = FindMessageIdString(messageId);

				MessageHandler* handler = FindMessageHandler(messageId);
				
				if (!handler)
					throw std::runtime_error(
						"Unknown message handler for ID " + std::to_string(messageId)
					);

				if (handler->Callback)
				{
					handler->Callback();
				}
				else if (handler->Length != -1)
				{
					// fixed-size message
					bitBuffer->seekBytes(handler->Length);
				}
				else
				{
					// user messages (variable length)
					if (messageId >= 64)
					{
						MessageUserDefault();
					}
					else
					{
						throw std::runtime_error(
							"Unknown variable-length message ID " + std::to_string(messageId)
						);
					}
				}

				// end of frame?
				if (bitBuffer->currentByte() == bitBuffer->length() || !readingGameData)
					break;
			}
		}
		catch (...) {
			readingGameData = false;
			file.close();
			throw;
		}

		readingGameData = false;
	}

	void DemoParser::AddMessageHandler(uint8_t id, int32_t length, std::function<void()> callback)
	{
		MessageHandler handler;
		handler.Id = id;
		handler.Length = length;
		handler.Callback = callback;
		
		messageHandlerTable[id] = handler;
	}

	MessageHandler* DemoParser::FindMessageHandler(uint8_t id)
	{
		auto it = messageHandlerTable.find(id);
		if (it == messageHandlerTable.end())
			return nullptr;

		return &it->second;
	}

	void DemoParser::AddUserMessage(uint8_t id, int8_t length, const std::string& name)
	{
		auto it = userMessageTable.find(name);
		if (it != userMessageTable.end())
		{
			userMessageTable.erase(it);
		}

		UserMessage userMessage;
		userMessage.Id = id;
		userMessage.Length = length;

		userMessageTable[name] = userMessage;

		AddMessageHandler(id, length, nullptr);
	}

	std::string DemoParser::FindMessageIdString(uint8_t id)
	{
		for (const auto& [name, message] : userMessageTable) {
			if (message.Id == id) {
				return name;
			}
		}

		return "";
	}

	void DemoParser::MessageClientData()
	{
		// Read delta sequence bit
		bool deltaSequence = bitBuffer->readBoolean();
		if (deltaSequence) {
			//printf("DELTA SEQUENCE!!!\n");
			// skip delta sequence number (8 bits)
			bitBuffer->seekBits(8);
		}

		// Read clientdata delta block
		auto clientdata_delta_structure = GetDeltaStructure("clientdata_t");

		HalfLifeDelta halflifedelta = clientdata_delta_structure->createDelta();

		clientdata_delta_structure->readDelta(*bitBuffer, &halflifedelta);

		ClientData clientData = toClientData(halflifedelta);

		if(OnClientData)
			OnClientData(clientData);

		// Weapon loop
		while (bitBuffer->readBoolean())
		{
			bitBuffer->seekBits(6);

			// Read weapon_data_t delta
			GetDeltaStructure("weapon_data_t")->readDelta(*bitBuffer);
		}

		// Skip until end of message frame
		bitBuffer->skipRemainingBits();

		// Restore default endian
		bitBuffer->setEndian(EndianType::Little);
	}

	void DemoParser::MessageSetAngle()
	{
		Angle angle;

		angle.pitch = bitBuffer->readInt16();
		angle.yaw = bitBuffer->readInt16();
		angle.roll = bitBuffer->readInt16();

		if(OnSetAngle)
			OnSetAngle(angle);
	}

	void DemoParser::MessagePrint()
    {
        std::string str = bitBuffer->readString();
		if(OnMessagePrint)
		{
			OnMessagePrint(str);
		}
    }

	void DemoParser::MessageServerInfo() {

		ServerInfo serverInfo;

		serverInfo.Protocol = bitBuffer->readUInt32();
		serverInfo.SpawnCount = bitBuffer->readUInt32();
		serverInfo.MapCRC = bitBuffer->readUInt32();

		auto data = bitBuffer->readBytes(16);
		memcpy(serverInfo.ClientDLLHash, data.data(), 16);
		
		serverInfo.MaxPlayers = bitBuffer->readByte();
		maxClients = serverInfo.MaxPlayers;

		serverInfo.PlayerIndex = bitBuffer->readByte();
		serverInfo.IsDeathmatch = bitBuffer->readByte();

		serverInfo.GameDir = bitBuffer->readString();
		serverInfo.Hostname = bitBuffer->readString();
		serverInfo.MapFileName = bitBuffer->readString();
		serverInfo.Mapcycle = bitBuffer->readString();

		serverInfo.Zero = bitBuffer->readByte();
		

		if(serverInfo.Zero > 0)
		{
			Seek(21);
		}

		if(OnServerInfo)
			OnServerInfo(serverInfo);

		serverInfoParsed = true;
	}
	
	void DemoParser::MessageExtraInfo()
	{
		std::string tempString = bitBuffer->readString();
		//std::cout<<tempString<<std::endl;
		Seek(1);
	}

	void DemoParser::MessageDeltaDescription()
    {
        std::string structureName = bitBuffer->readString();

        uint32_t nEntries = bitBuffer->readUnsignedBits(16);

		auto newDeltaStructure = std::make_unique<HalfLifeDeltaStructure>(structureName);

		// Add it to delta dictionary
		
        HalfLifeDeltaStructure* deltaDescription = GetDeltaStructure("delta_description_t");

        for (uint16_t i = 0; i < nEntries; i++)
        {
            HalfLifeDelta newDelta = deltaDescription->createDelta();
            deltaDescription->readDelta(*bitBuffer, &newDelta);

            newDeltaStructure->addEntry(newDelta);
        }

		AddDeltaStructure(std::move(newDeltaStructure));

        bitBuffer->skipRemainingBits();
		bitBuffer->setEndian(EndianType::Little);
	}

	void DemoParser::MessageNewMoveVars()
	{
        // TODO: see OHLDS, SV_SetMoveVars
        /*
        MSG_WriteFloat(buf, movevars.gravity);
        MSG_WriteFloat(buf, movevars.stopspeed);
        MSG_WriteFloat(buf, movevars.maxspeed);
        MSG_WriteFloat(buf, movevars.spectatormaxspeed);
        MSG_WriteFloat(buf, movevars.accelerate);
        MSG_WriteFloat(buf, movevars.airaccelerate);
        MSG_WriteFloat(buf, movevars.wateraccelerate);
        MSG_WriteFloat(buf, movevars.friction);
        MSG_WriteFloat(buf, movevars.edgefriction);
        MSG_WriteFloat(buf, movevars.waterfriction);
        MSG_WriteFloat(buf, movevars.entgravity);
        MSG_WriteFloat(buf, movevars.bounce);
        MSG_WriteFloat(buf, movevars.stepsize);
        MSG_WriteFloat(buf, movevars.maxvelocity);
        MSG_WriteFloat(buf, movevars.zmax);
        MSG_WriteFloat(buf, movevars.waveHeight);
        MSG_WriteByte(buf, (movevars.footsteps != 0)); //Sets it to 1 if nonzero, just in case someone's abusing the whole 'bool' thing.
        MSG_WriteFloat(buf, movevars.rollangle);
        MSG_WriteFloat(buf, movevars.rollspeed);
        MSG_WriteFloat(buf, movevars.skycolor_r);
        MSG_WriteFloat(buf, movevars.skycolor_g);
        MSG_WriteFloat(buf, movevars.skycolor_b);
        MSG_WriteFloat(buf, movevars.skyvec_x);
        MSG_WriteFloat(buf, movevars.skyvec_y);
        MSG_WriteFloat(buf, movevars.skyvec_z);
        MSG_WriteString(buf, movevars.skyName);
         */

        // same as gamedata header
        // 800, 75, 900, 500, 5, 10, 10, 4, 2, 1, 1, 1, 18, 2000, 6400, 0, 1, 0
        // + 24 bytes of unknown

        // different size in network protocols < 45 like gamedata frame header???

		MoveVars mv{};

		// 16 * 4 = 64 bytes

		mv.gravity           = bitBuffer->readFloat();
		mv.stopspeed         = bitBuffer->readFloat();
		mv.maxspeed          = bitBuffer->readFloat();
		mv.spectatormaxspeed = bitBuffer->readFloat();
		mv.accelerate        = bitBuffer->readFloat();
		mv.airaccelerate     = bitBuffer->readFloat();
		mv.wateraccelerate   = bitBuffer->readFloat();
		mv.friction          = bitBuffer->readFloat();
		mv.edgefriction      = bitBuffer->readFloat();
		mv.waterfriction     = bitBuffer->readFloat();
		mv.entgravity        = bitBuffer->readFloat();
		mv.bounce            = bitBuffer->readFloat();
		mv.stepsize          = bitBuffer->readFloat();
		mv.maxvelocity       = bitBuffer->readFloat();
		mv.zmax              = bitBuffer->readFloat();
		mv.waveHeight        = bitBuffer->readFloat();

		// 65 bytes
		mv.footsteps = bitBuffer->readByte();

		// 73 bytes
		mv.rollangle    = bitBuffer->readFloat();
		mv.rollspeed    = bitBuffer->readFloat();

		// 85 bytes

		mv.skycolor_r   = bitBuffer->readFloat();
		mv.skycolor_g   = bitBuffer->readFloat();
		mv.skycolor_b   = bitBuffer->readFloat();

		// 97
		mv.skyvec_x     = bitBuffer->readFloat();
		mv.skyvec_y     = bitBuffer->readFloat();
		mv.skyvec_z     = bitBuffer->readFloat();

		mv.skyName = bitBuffer->readString();

		if(OnNewMoveVars)
			OnNewMoveVars(mv);

        //Seek(98);
        //bitBuffer->readString();
	}

	void DemoParser::MessageNewUserMsg()
	{
		uint8_t id = bitBuffer->readByte();
        int8_t length = bitBuffer->readSByte();
        std::string name = bitBuffer->readString(16);

        AddUserMessage(id, length, name);
	}

	void DemoParser::MessageUpdateUserInfo()
	{
		UpdateUserInfo updateUserInfo;
		updateUserInfo.ClientIndex = bitBuffer->readByte();
		updateUserInfo.ClientUserID = bitBuffer->readUInt32();
		updateUserInfo.ClientUserInfo = bitBuffer->readString();

		auto data = bitBuffer->readBytes(16);
		memcpy(updateUserInfo.ClientCDKeyHash, data.data(), 16);

		if(OnUpdateUserInfo)
			OnUpdateUserInfo(updateUserInfo);
	}

	void DemoParser::MessageResourceList() {
		// Read number of resource entries (12 bits)
		uint32_t nEntries = bitBuffer->readUnsignedBits(12);

		for (int32_t i = 0; i < static_cast<int32_t>(nEntries); ++i) {
			bitBuffer->seekBits(4);           // entry type
			bitBuffer->readString();           // entry name
			bitBuffer->seekBits(36);           // index (12 bits) + file size (24 bits), signed?

			uint32_t flags = bitBuffer->readUnsignedBits(3);

			if ((flags & 4) != 0) {           // md5 hash present
				bitBuffer->seekBytes(16);      // skip hash
			}

			if (bitBuffer->readBoolean()) {
				bitBuffer->seekBytes(32);      // reserved data
			}
		}

		// Consistency list (indices of resources to force consistency upon)
		if (bitBuffer->readBoolean()) {
			while (bitBuffer->readBoolean()) {
				int nBits = bitBuffer->readBoolean() ? 5 : 10;
				bitBuffer->seekBits(nBits);    // skip consistency index
			}
		}

		bitBuffer->skipRemainingBits();        // align to byte boundary
		bitBuffer->setEndian(EndianType::Little);
	}

	void DemoParser::MessageSpawnBaseline() {
		while (true) {
			uint32_t entityIndex = bitBuffer->readUnsignedBits(11);

			if (entityIndex == (1u << 11) - 1) {  // all 1's
				break;
			}

			uint32_t entityType = bitBuffer->readUnsignedBits(2);
			std::string entityTypeString;

			if ((entityType & 1) != 0) {  // is bit 1 set?
				if (entityIndex > 0 && entityIndex <= maxClients) {
					entityTypeString = "entity_state_player_t";
				} else {
					entityTypeString = "entity_state_t";
				}
			} else {
				entityTypeString = "custom_entity_state_t";
			}

			GetDeltaStructure(entityTypeString)->readDelta(*bitBuffer);
		}

		uint32_t footer = bitBuffer->readUnsignedBits(5);  // should be all 1's
		if (footer != (1u << 5) - 1) {
			throw std::runtime_error("Bad svc_spawnbaseline footer.");
		}

		uint32_t nExtraData = bitBuffer->readUnsignedBits(6);
		for (int32_t i = 0; i < static_cast<int32_t>(nExtraData); ++i) {
			GetDeltaStructure("entity_state_t")->readDelta(*bitBuffer);
		}

		bitBuffer->setEndian(EndianType::Little);
		bitBuffer->skipRemainingBits();
	}

	void DemoParser::MessageVoiceInit()
	{
		bitBuffer->readString();
		Seek(1);
	}

	void DemoParser::MessageLightStyle()
    {
        Seek(1);
        bitBuffer->readString();
    }

	void DemoParser::MessageCustomization()
	{
		Seek(2);
		bitBuffer->readString();
		Seek(23);
	}

	void DemoParser::MessageVoiceData()
    {
        // byte: client id/slot?
        // short: data length
        // length bytes: data
        Seek(1);
        uint16_t length = bitBuffer->readUInt16();
        bitBuffer->seekBytes(length);
    }

    void DemoParser::MessageSendExtraInfo()
    {
        // string: "com_clientfallback", always seems to be null
        // byte: sv_cheats
        // NOTE: had this backwards before, shouldn't matter
        bitBuffer->readString();
        Seek(1);
    }
	
    void DemoParser::MessageResourceLocation()
    {
        // string: location?
        bitBuffer->readString();
    }

    void DemoParser::MessageSendCvarValue()
    {
        bitBuffer->readString(); // The cvar.
    }

    void DemoParser::MessageSendCvarValue2()
    {
        Seek(4); // unsigned int
        bitBuffer->readString(); // The cvar.
    }
	void DemoParser::MessagePacketEntities() 
	{
		// Skip num entities (16 bits, not reliable)
		bitBuffer->seekBits(16);

		uint32_t entityNumber = 0;

		// Begin entity parsing
		while (true) {
			uint16_t footer = bitBuffer->readUInt16();

			if (footer == 0) {
				break;
			}

			bitBuffer->seekBits(-16);  // rewind 16 bits

			bool entityNumberIncrement = bitBuffer->readBoolean();

			if (!entityNumberIncrement) {
				// is the following entity number absolute or relative?
				bool absoluteEntityNumber = bitBuffer->readBoolean();

				if (absoluteEntityNumber) {
					entityNumber = bitBuffer->readUnsignedBits(11);
				} else {
					entityNumber += bitBuffer->readUnsignedBits(6);
				}
			} else {
				entityNumber++;
			}

			bool custom = bitBuffer->readBoolean();
			bool useBaseline = bitBuffer->readBoolean();

			if (useBaseline) {
				bitBuffer->seekBits(6);  // baseline index
			}

			std::string entityType = "entity_state_t";

			if (entityNumber > 0 && entityNumber <= maxClients) 
			{
				entityType = "entity_state_player_t";

			} else if (custom) {
				entityType = "custom_entity_state_t";
			}

			auto delta_structure = GetDeltaStructure(entityType);

			HalfLifeDelta halflifedelta = delta_structure->createDelta();

			delta_structure->readDelta(*bitBuffer, &halflifedelta);

			if (entityNumber > 0 && entityNumber <= maxClients) 
			{
				EntityStatePlayer entityStatePlayer = toEntityStatePlayer(halflifedelta);
				if(OnPackedPlayerEntity)
					OnPackedPlayerEntity(entityStatePlayer);

			} else if (custom) {
				CustomEntityState customEntityState = toCustomEntityState(halflifedelta);
				if(OnPackedCustomEntity)
					OnPackedCustomEntity(customEntityState);
			}
		}

		bitBuffer->skipRemainingBits();
		bitBuffer->setEndian(EndianType::Little);
	}

	void DemoParser::MessageTempEntity() 
	{
		uint8_t type = bitBuffer->readByte();

		switch (type) {
			// obsolete
			case 16: // TE_BEAM
			case 26: // TE_BEAMHOSE
				break;

			// simple coord format messages
			case 2:  // TE_GUNSHOT
			case 4:  // TE_TAREXPLOSION
			case 9:  // TE_SPARKS
			case 10: // TE_LAVASPLASH
			case 11: // TE_TELEPORT
				Seek(6);
				break;

			case 0:  // TE_BEAMPOINTS
				Seek(24);
				break;

			case 1:  // TE_BEAMENTPOINT
				Seek(20);
				break;

			case 3:  // TE_EXPLOSION
				Seek(11);
				break;

			case 5:  // TE_SMOKE
				Seek(10);
				break;

			case 6:  // TE_TRACER
				Seek(12);
				break;

			case 7:  // TE_LIGHTNING
				Seek(17);
				break;

			case 8:  // TE_BEAMENTS
				Seek(16);
				break;

			case 12: // TE_EXPLOSION2
				Seek(8);
				break;

			case 13: { // TE_BSPDECAL
				Seek(8);
				uint16_t entityIndex = bitBuffer->readUInt16();
				if (entityIndex != 0) {
					Seek(2);
				}
				break;
			}

			case 14: // TE_IMPLOSION
				Seek(9);
				break;

			case 15: // TE_SPRITETRAIL
				Seek(19);
				break;

			case 17: // TE_SPRITE
				Seek(10);
				break;

			case 18: // TE_BEAMSPRITE
				Seek(16);
				break;

			case 19: // TE_BEAMTORUS
			case 20: // TE_BEAMDISK
			case 21: // TE_BEAMCYLINDER
				Seek(24);
				break;

			case 22: // TE_BEAMFOLLOW
				Seek(10);
				break;

			case 23: // TE_GLOWSPRITE
				Seek(11);
				break;

			case 24: // TE_BEAMRING
				Seek(16);
				break;

			case 25: // TE_STREAK_SPLASH
				Seek(19);
				break;

			case 27: // TE_DLIGHT
				Seek(12);
				break;

			case 28: // TE_ELIGHT
				Seek(16);
				break;

			case 29: { // TE_TEXTMESSAGE
				Seek(5);
				uint8_t textParmsEffect = bitBuffer->readByte();
				Seek(14);
				if (textParmsEffect == 2) {
					Seek(2);
				}
				bitBuffer->readString(); // capped to 512 bytes
				break;
			}

			case 30: // TE_LINE
			case 31: // TE_BOX
				Seek(17);
				break;

			case 99:  // TE_KILLBEAM
				Seek(2);
				break;

			case 100: // TE_LARGEFUNNEL
				Seek(10);
				break;

			case 101: // TE_BLOODSTREAM
				Seek(14);
				break;

			case 102: // TE_SHOWLINE
				Seek(12);
				break;

			case 103: // TE_BLOOD
				Seek(14);
				break;

			case 104: // TE_DECAL
				Seek(9);
				break;

			case 105: // TE_FIZZ
				Seek(5);
				break;

			case 106: // TE_MODEL
				Seek(17);
				break;

			case 107: // TE_EXPLODEMODEL
				Seek(13);
				break;

			case 108: // TE_BREAKMODEL
				Seek(24);
				break;

			case 109: // TE_GUNSHOTDECAL
				Seek(9);
				break;

			case 110: // TE_SPRITE_SPRAY
				Seek(17);
				break;

			case 111: // TE_ARMOR_RICOCHET
				Seek(7);
				break;

			case 112: // TE_PLAYERDECAL
				Seek(10);
				break;

			case 113: // TE_BUBBLES
			case 114: // TE_BUBBLETRAIL
				Seek(19);
				break;

			case 115: // TE_BLOODSPRITE
				Seek(12);
				break;

			case 116: // TE_WORLDDECAL
			case 117: // TE_WORLDDECALHIGH
				Seek(7);
				break;

			case 118: // TE_DECALHIGH
				Seek(9);
				break;

			case 119: // TE_PROJECTILE
				Seek(16);
				break;

			case 120: // TE_SPRAY
				Seek(18);
				break;

			case 121: // TE_PLAYERSPRITES
				Seek(5);
				break;

			case 122: // TE_PARTICLEBURST
				Seek(10);
				break;

			case 123: // TE_FIREFIELD
				Seek(9);
				break;

			case 124: // TE_PLAYERATTACHMENT
				Seek(7);
				break;

			case 125: // TE_KILLPLAYERATTACHMENTS
				Seek(1);
				break;

			case 126: // TE_MULTIGUNSHOT
				Seek(18);
				break;

			case 127: // TE_USERTRACER
				Seek(15);
				break;

			default: {
				std::ostringstream oss;
				oss << "Unknown tempentity type \"" << static_cast<int>(type) << "\".";
				throw std::runtime_error(oss.str());
			}
		}
	}

	void DemoParser::MessageDeltaPacketEntities() 
	{
		// Skip num entities (16 bits) and delta sequence number (8 bits)
		bitBuffer->seekBits(16);
		bitBuffer->seekBits(8);

		uint32_t entityNumber = 0;

		while (true) {
			uint16_t footer = bitBuffer->readUInt16();

			if (footer == 0) {
				break;
			}

			bitBuffer->seekBits(-16);  // rewind 16 bits

			bool removeEntity = bitBuffer->readBoolean();
			bool absoluteEntityNumber = bitBuffer->readBoolean();

			if (absoluteEntityNumber) {
				entityNumber = bitBuffer->readUnsignedBits(11);
			} else {
				entityNumber += bitBuffer->readUnsignedBits(6);
			}

			if (!removeEntity) {
				bool custom = bitBuffer->readBoolean();

				std::string entityType = "entity_state_t";

				if (entityNumber > 0 && entityNumber <= maxClients) {
					entityType = "entity_state_player_t";
				} else if (custom) {
					entityType = "custom_entity_state_t";
				}

				auto delta_structure = GetDeltaStructure(entityType);

				HalfLifeDelta halflifedelta = delta_structure->createDelta();

				delta_structure->readDelta(*bitBuffer, &halflifedelta);

				if (entityNumber > 0 && entityNumber <= maxClients) 
				{
					EntityStatePlayer entityStatePlayer = toEntityStatePlayer(halflifedelta);
					if(OnDeltaPackedPlayerEntity)
						OnDeltaPackedPlayerEntity(entityStatePlayer);

				} else if (custom) {
					CustomEntityState customEntityState = toCustomEntityState(halflifedelta);
					if(OnDeltaPackedCustomEntity)
						OnDeltaPackedCustomEntity(customEntityState);
				}

				
			}
		}

		bitBuffer->skipRemainingBits();
		bitBuffer->setEndian(EndianType::Little);
	}

	void DemoParser::MessageSound() 
	{
		uint32_t flags = bitBuffer->readUnsignedBits(9);

		// Volume
		if (flags & (1 << 0)) {
			bitBuffer->seekBits(8);
		}

		// Attenuation * 64
		if (flags & (1 << 1)) {
			bitBuffer->seekBits(8);
		}

		// Channel
		bitBuffer->seekBits(3);

		// Edict number
		bitBuffer->seekBits(11);

		// Sound index
		if (flags & (1 << 2)) {  // short
			bitBuffer->seekBits(16);
		} else {                 // byte
			bitBuffer->seekBits(8);
		}

		// Position
		bitBuffer->readVectorCoord();

		// Pitch
		if (flags & (1 << 3)) {
			bitBuffer->seekBits(8);
		}

		bitBuffer->skipRemainingBits();
		bitBuffer->setEndian(EndianType::Little);
	}

	void DemoParser::MessagePing() {

		while (bitBuffer->readBoolean())
        {
            bitBuffer->seekBits(24); // int32 each: slot, ping, loss
        }

        bitBuffer->skipRemainingBits();
		bitBuffer->setEndian(EndianType::Little);
	}

	void DemoParser::MessageTime()
	{
		float time = bitBuffer->readFloat();

		if(OnTimeTick)
			OnTimeTick(time);
	}

	void DemoParser::MessageUserDefault()
	{
		uint8_t length = bitBuffer->readByte();
		bitBuffer->seekBytes(length);
	}

	void DemoParser::Seek(std::streamoff offset, std::ios_base::seekdir origin) 
	{
		if (readingGameData) {
            if (offset > std::numeric_limits<int32_t>::max())
                throw std::runtime_error("Offset too large for BitBuffer seek");
            bitBuffer->seekBytes(static_cast<int32_t>(offset), origin);
        } else {
            file.seekg(offset, origin);
        }
	}

}