#include <cstdint>
#include <string>

struct DemoHeader {
	uint32_t networkProtocol = 0;
    uint32_t demoProtocol = 0;
    std::string mapName;
    std::string gameFolderName;
    uint32_t mapChecksum = 0;
    int32_t directoryOffset;
		
	struct DemoDirectoryEntry {
		int32_t type;
		std::string description;
		int32_t flags;
		int32_t CDTrack;
		float trackTime;
		int32_t frameCount;
		int32_t offset;
		int32_t fileLength;
	} demoDirectory[2];
};

struct ServerInfo
{
    uint32_t Protocol;
    uint32_t SpawnCount;
    uint32_t MapCRC;

    uint8_t ClientDLLHash[16];

    uint8_t MaxPlayers;
    uint8_t PlayerIndex;
    uint8_t IsDeathmatch;

    std::string GameDir;
    std::string Hostname;
    std::string MapFileName;
    std::string Mapcycle;

    uint8_t Zero;
};


struct UpdateUserInfo
{
    uint8_t 	    ClientIndex;
    uint32_t 	    ClientUserID;
    std::string 	ClientUserInfo;
    uint8_t         ClientCDKeyHash[16];;
};

struct PlayerState {
    float position[3];
    float rotation[3];
    uint32_t weaponFlags;
    float fov;
};

#define MAX_PHYSINFO_STRING 256

struct ClientData {
    float origin[3];
    float velocity[3];

    int viewmodel;
    float punchangle[3];
    int flags;
    int waterlevel;
    int watertype;
    float view_ofs[3];
    float health;

    int bInDuck;

    int weapons; // optional, can remove if unused

    int flTimeStepSound;
    int flDuckTime;
    int flSwimTime;
    int waterjumptime;

    float maxspeed;

    float fov;
    int weaponanim;

    int m_iId;
    int ammo_shells;
    int ammo_nails;
    int ammo_cells;
    int ammo_rockets;
    float m_flNextAttack;

    int tfstate;
    int pushmsec;
    int deadflag;

    char physinfo[MAX_PHYSINFO_STRING]; // fixed-size string

    // Mod-specific fields
    int iuser1;
    int iuser2;
    int iuser3;
    int iuser4;
    float fuser1;
    float fuser2;
    float fuser3;
    float fuser4;
    float vuser1[3];
    float vuser2[3];
    float vuser3[3];
    float vuser4[3];
};

struct EventFrame {
	int flags;
	int index;
	float delay;

	struct EventArgs{
		int flags;
		int32_t entityIndex;
		float origin[3];
		float angles[3];
		float velocity[3];
		int32_t ducking;
		float fparam1;
		float fparam2;
		int32_t iparam1;
		int32_t iparam2;
		int32_t bparam1;
		int32_t bparam2;
	} args;
};

struct Color {
    uint8_t r, g, b;
};

struct EntityStatePlayer {
    float animtime;
    float frame;
    
    float origin[3];
    float angles[3];
    
    int gaitsequence;
    int sequence;
    int modelindex;
    int movetype;
    int solid;
    
    float mins[3];
    float maxs[3];
    
    int weaponmodel;
    int team;
    int playerclass;
    int owner;
    int effects;
    
    float framerate;
    int skin;
    
    uint8_t controller[4];
    uint8_t blending[2];
    
    int body;
    int rendermode;
    int renderamt;
    int renderfx;
    float scale;
    
    Color rendercolor;
    
    float friction;
    int usehull;
    float gravity;
    int aiment;
    
    float basevelocity[3];
    int spectator;
};

struct CustomEntityState {
    int rendermode;
    float origin[3];
    float angles[3];
    int sequence;
    int skin;
    int modelindex;
    
    float scale; // note: post multiplier applied in decode
    
    int body;
    Color rendercolor;
    
    int renderfx;
    int renderamt;
    
    float frame;
    float animtime; // note: post multiplier applied
};

struct MoveVars{ 
	float	gravity;           // Gravity for map
	float	stopspeed;         // Deceleration when not moving
	float	maxspeed;          // Max allowed speed
	float	spectatormaxspeed;
	float	accelerate;        // Acceleration factor
	float	airaccelerate;     // Same for when in open air
	float	wateraccelerate;   // Same for when in water
	float	friction;          
	float   edgefriction;	   // Extra friction near dropofs 
	float	waterfriction;     // Less in water
	float	entgravity;        // 1.0
	float   bounce;            // Wall bounce value. 1.0
	float   stepsize;          // sv_stepsize;
	float   maxvelocity;       // maximum server velocity.
	float	zmax;			   // Max z-buffer range (for GL)
	float	waveHeight;		   // Water wave height (for GL)
	uint32_t    footsteps;        // Play footstep sounds
	float	rollangle;
	float	rollspeed;
	float	skycolor_r;			// Sky color
	float	skycolor_g;			// 
	float	skycolor_b;			//
	float	skyvec_x;			// Sky vector
	float	skyvec_y;			// 
	float	skyvec_z;			//
    std::string	skyName;	   // Name of the sky map
};

struct Angle {
    int16_t pitch;
    int16_t yaw;
    int16_t roll;
};