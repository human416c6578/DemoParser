#include <BitBuffer.h>
#include <HalfLifeDeltas.h>

#include <cstdint>
#include <fstream>
#include <functional>
#include <ios>
#include <iostream>
#include <memory>
#include <vector>

namespace demo_analyser
{
    enum class SVCMessage : uint8_t
	{
		SVC_BAD                     =  0,
		SVC_NOP                     =  1,
		SVC_DISCONNECT              =  2,
		SVC_EVENT                   =  3,
		SVC_VERSION                 =  4,
		SVC_SETVIEW                 =  5,
		SVC_SOUND                   =  6,
		SVC_TIME                    =  7,
		SVC_PRINT                   =  8,
		SVC_STUFFTEXT               =  9,
		SVC_SETANGLE                = 10,
		SVC_SERVERINFO              = 11,
		SVC_LIGHTSTYLE              = 12,
		SVC_UPDATEUSERINFO          = 13,
		SVC_DELTADESCRIPTION        = 14,
		SVC_CLIENTDATA              = 15,
		SVC_STOPSOUND               = 16,
		SVC_PINGS                   = 17,
		SVC_PARTICLE                = 18,
		SVC_DAMAGE                  = 19,
		SVC_SPAWN                   = 20,
		SVC_EVENT_RELIABLE          = 21,
		SVC_SPAWNBASELINE           = 22,
		SVC_TEMPENTITY              = 23,
		SVC_SETPAUSE                = 24,
		SVC_SIGNONNUM               = 25,
		SVC_CENTERPRINT             = 26,
		SVC_KILLEDMONSTER           = 27,
		SVC_FOUNDSECRET             = 28,
		SVC_SPAWNSTATICSOUND        = 29,
		SVC_INTERMISSION            = 30,
		SVC_FINALE                  = 31,
		SVC_CDTRACK                 = 32,
		SVC_RESTORE                 = 33,
		SVC_CUTSCENE                = 34,
		SVC_WEAPONANIM              = 35,
		SVC_DECALNAME               = 36,
		SVC_ROOMTYPE                = 37,
		SVC_ADDANGLE                = 38,
		SVC_NEWUSERMSG              = 39,
		SVC_PACKETENTITIES          = 40,
		SVC_DELTAPACKETENTITIES     = 41,
		SVC_CHOKE                   = 42,
		SVC_RESOURCELIST            = 43,
		SVC_NEWMOVEVARS             = 44,
		SVC_RESOURCEREQUEST         = 45,
		SVC_CUSTOMIZATION           = 46,
		SVC_CROSSHAIRANGLE          = 47,
		SVC_SOUNDFADE               = 48,
		SVC_FILETXFERFAILED         = 49,
		SVC_HLTV                    = 50,
		SVC_DIRECTOR                = 51,
		SVC_VOICEINIT               = 52,
		SVC_VOICEDATA               = 53,
		SVC_SENDEXTRAINFO           = 54,
		SVC_TIMESCALE               = 55,
		SVC_RESOURCELOCATION        = 56,
		SVC_SENDCVARVALUE2          = 58
	};

	inline const char* SVCMessageName(uint8_t id)
	{
		switch (static_cast<SVCMessage>(id))
		{
			case SVCMessage::SVC_BAD:                 return "SVC_BAD";
			case SVCMessage::SVC_NOP:                 return "SVC_NOP";
			case SVCMessage::SVC_DISCONNECT:          return "SVC_DISCONNECT";
			case SVCMessage::SVC_EVENT:               return "SVC_EVENT";
			case SVCMessage::SVC_VERSION:             return "SVC_VERSION";
			case SVCMessage::SVC_SETVIEW:             return "SVC_SETVIEW";
			case SVCMessage::SVC_SOUND:               return "SVC_SOUND";
			case SVCMessage::SVC_TIME:                return "SVC_TIME";
			case SVCMessage::SVC_PRINT:               return "SVC_PRINT";
			case SVCMessage::SVC_STUFFTEXT:           return "SVC_STUFFTEXT";
			case SVCMessage::SVC_SETANGLE:            return "SVC_SETANGLE";
			case SVCMessage::SVC_SERVERINFO:          return "SVC_SERVERINFO";
			case SVCMessage::SVC_LIGHTSTYLE:          return "SVC_LIGHTSTYLE";
			case SVCMessage::SVC_UPDATEUSERINFO:      return "SVC_UPDATEUSERINFO";
			case SVCMessage::SVC_DELTADESCRIPTION:    return "SVC_DELTADESCRIPTION";
			case SVCMessage::SVC_CLIENTDATA:          return "SVC_CLIENTDATA";
			case SVCMessage::SVC_STOPSOUND:           return "SVC_STOPSOUND";
			case SVCMessage::SVC_PINGS:               return "SVC_PINGS";
			case SVCMessage::SVC_PARTICLE:            return "SVC_PARTICLE";
			case SVCMessage::SVC_DAMAGE:              return "SVC_DAMAGE";
			case SVCMessage::SVC_SPAWN:               return "SVC_SPAWN";
			case SVCMessage::SVC_EVENT_RELIABLE:      return "SVC_EVENT_RELIABLE";
			case SVCMessage::SVC_SPAWNBASELINE:       return "SVC_SPAWNBASELINE";
			case SVCMessage::SVC_TEMPENTITY:          return "SVC_TEMPENTITY";
			case SVCMessage::SVC_SETPAUSE:            return "SVC_SETPAUSE";
			case SVCMessage::SVC_SIGNONNUM:           return "SVC_SIGNONNUM";
			case SVCMessage::SVC_CENTERPRINT:         return "SVC_CENTERPRINT";
			case SVCMessage::SVC_KILLEDMONSTER:       return "SVC_KILLEDMONSTER";
			case SVCMessage::SVC_FOUNDSECRET:         return "SVC_FOUNDSECRET";
			case SVCMessage::SVC_SPAWNSTATICSOUND:    return "SVC_SPAWNSTATICSOUND";
			case SVCMessage::SVC_INTERMISSION:        return "SVC_INTERMISSION";
			case SVCMessage::SVC_FINALE:              return "SVC_FINALE";
			case SVCMessage::SVC_CDTRACK:             return "SVC_CDTRACK";
			case SVCMessage::SVC_RESTORE:             return "SVC_RESTORE";
			case SVCMessage::SVC_CUTSCENE:            return "SVC_CUTSCENE";
			case SVCMessage::SVC_WEAPONANIM:          return "SVC_WEAPONANIM";
			case SVCMessage::SVC_DECALNAME:           return "SVC_DECALNAME";
			case SVCMessage::SVC_ROOMTYPE:            return "SVC_ROOMTYPE";
			case SVCMessage::SVC_ADDANGLE:            return "SVC_ADDANGLE";
			case SVCMessage::SVC_NEWUSERMSG:          return "SVC_NEWUSERMSG";
			case SVCMessage::SVC_PACKETENTITIES:      return "SVC_PACKETENTITIES";
			case SVCMessage::SVC_DELTAPACKETENTITIES: return "SVC_DELTAPACKETENTITIES";
			case SVCMessage::SVC_CHOKE:               return "SVC_CHOKE";
			case SVCMessage::SVC_RESOURCELIST:        return "SVC_RESOURCELIST";
			case SVCMessage::SVC_NEWMOVEVARS:         return "SVC_NEWMOVEVARS";
			case SVCMessage::SVC_RESOURCEREQUEST:     return "SVC_RESOURCEREQUEST";
			case SVCMessage::SVC_CUSTOMIZATION:       return "SVC_CUSTOMIZATION";
			case SVCMessage::SVC_CROSSHAIRANGLE:      return "SVC_CROSSHAIRANGLE";
			case SVCMessage::SVC_SOUNDFADE:           return "SVC_SOUNDFADE";
			case SVCMessage::SVC_FILETXFERFAILED:     return "SVC_FILETXFERFAILED";
			case SVCMessage::SVC_HLTV:                return "SVC_HLTV";
			case SVCMessage::SVC_DIRECTOR:            return "SVC_DIRECTOR";
			case SVCMessage::SVC_VOICEINIT:           return "SVC_VOICEINIT";
			case SVCMessage::SVC_VOICEDATA:           return "SVC_VOICEDATA";
			case SVCMessage::SVC_SENDEXTRAINFO:       return "SVC_SENDEXTRAINFO";
			case SVCMessage::SVC_TIMESCALE:           return "SVC_TIMESCALE";
			case SVCMessage::SVC_RESOURCELOCATION:    return "SVC_RESOURCELOCATION";
			case SVCMessage::SVC_SENDCVARVALUE2:      return "SVC_SENDCVARVALUE2";
		}
		return ""; // empty = likely user message
	}

	struct FrameHeader
	{
		uint8_t Type;
		float Timestamp;
		uint32_t Number;
	};

	struct GameDataFrameHeader
	{
		uint32_t ResolutionWidth;
		uint32_t ResolutionHeight;
		uint32_t Length;
	};

	struct MessageHandler 
	{
		int Id;
		std::function<void()> Callback;  // or nullptr
		int Length = -1;                 // -1 means variable length (user message)
	};

	struct UserMessage
	{
		uint8_t Id;
        int8_t Length;
	};

	class DemoParser
    {
		public:
			DemoParser(const std::string& path);

			void parseDemo();

		private:
			std::ifstream file;
			std::unique_ptr<BitBuffer> bitBuffer;
			std::unordered_map<std::string, std::unique_ptr<HalfLifeDeltaStructure>> deltaDecoderTable;
			std::unordered_map<uint8_t, MessageHandler> messageHandlerTable;

			std::unordered_map<std::string, UserMessage> userMessageTable;

			int maxClients;
			int frames = 0;
			bool serverInfoParsed = false;

			bool readingGameData = false;
			void readDemoHeader(std::ifstream &file, const std::vector<uint8_t>& headerData, const uint32_t fileSize);
			FrameHeader ReadFrameHeader();
			GameDataFrameHeader ReadGameDataFrameHeader();
			void ParseGameDataMessages(const std::vector<uint8_t>& frameData);
			
			void MessageClientData();
			void MessageDeltaDescription();
			void MessagePrint();
			void MessageServerInfo();
			void MessageExtraInfo();
			void MessageNewMoveVars();
			void MessageNewUserMsg();
			void MessageUpdateUserInfo();
			void MessageResourceList();
			void MessageSpawnBaseline();
			void MessageLightStyle();
			void MessageVoiceInit();
			void MessageCustomization();
			void MessageVoiceData();
			void MessageSendExtraInfo();
			void MessageResourceLocation();
			void MessageSendCvarValue();
			void MessageSendCvarValue2();
			void MessagePacketEntities();
			void MessageTempEntity();
			void MessageDeltaPacketEntities();
			void MessageSound();
			void MessagePing();
			void MessageTime();
			void MessageSetAngle();

			void MessageUserDefault();

			void AddMessageHandler(uint8_t id, int32_t length, std::function<void()> callback);
			MessageHandler* FindMessageHandler(uint8_t id);

			void AddUserMessage(uint8_t id, int8_t length, const std::string& name);
			std::string FindMessageIdString(uint8_t id);
			

			void AddDeltaStructure(std::unique_ptr<HalfLifeDeltaStructure> structure)
			{
				const std::string& name = structure->getName();

				// Overwrite existing delta structure if it already exists
				deltaDecoderTable[name] = std::move(structure);
			}

			HalfLifeDeltaStructure* GetDeltaStructure(const std::string& name)
			{
				auto it = deltaDecoderTable.find(name);
				if (it == deltaDecoderTable.end())
					throw std::runtime_error("Delta structure \"" + name + "\" not found.");

				return it->second.get();
			}

			void Seek(std::streamoff offset, std::ios_base::seekdir origin = std::ios::cur);
			void SkipFrame(uint8_t frameType);
			int32_t GetFrameLength(uint8_t frameType);
    };

}