#include <HalfLifeDeltas.h>
#include "EventHandlers.h"

inline ClientData toClientData(const HalfLifeDelta& delta)
{
    ClientData data{};
    
    auto getFloat = [&](const char* name) -> float {
        if (const DeltaValue* val = delta.findEntryValue(name)) {
            return toFloat(*val);
        }
        return 0.0f;
    };

    auto getInt = [&](const char* name) -> int {
        if (const DeltaValue* val = delta.findEntryValue(name)) {
            if (auto ptr = std::get_if<int32_t>(val)) return *ptr;
            if (auto ptr = std::get_if<uint32_t>(val)) return static_cast<int>(*ptr);
        }
        return 0;
    };

    auto getVec3 = [&](const char* baseName, float out[3]) {
        out[0] = getFloat((std::string(baseName) + "[0]").c_str());
        out[1] = getFloat((std::string(baseName) + "[1]").c_str());
        out[2] = getFloat((std::string(baseName) + "[2]").c_str());
    };

    // Vectors
    getVec3("origin", data.origin);
    getVec3("velocity", data.velocity);
    getVec3("punchangle", data.punchangle);
    getVec3("view_ofs", data.view_ofs);
    getVec3("vuser1", data.vuser1);
    getVec3("vuser2", data.vuser2);
    getVec3("vuser3", data.vuser3);
    getVec3("vuser4", data.vuser4);

    // Scalars
    data.viewmodel = getInt("viewmodel");
    data.flags = getInt("flags");
    data.waterlevel = getInt("waterlevel");
    data.watertype = getInt("watertype");
    data.health = getFloat("health");
    data.bInDuck = getInt("bInDuck");
    data.weapons = getInt("weapons");
    data.flTimeStepSound = getInt("flTimeStepSound");
    data.flDuckTime = getInt("flDuckTime");
    data.flSwimTime = getInt("flSwimTime");
    data.waterjumptime = getInt("waterjumptime");
    data.maxspeed = getFloat("maxspeed");
    data.fov = getFloat("fov");
    data.weaponanim = getInt("weaponanim");
    data.m_iId = getInt("m_iId");
    data.ammo_shells = getInt("ammo_shells");
    data.ammo_nails = getInt("ammo_nails");
    data.ammo_cells = getInt("ammo_cells");
    data.ammo_rockets = getInt("ammo_rockets");
    data.m_flNextAttack = getFloat("m_flNextAttack");
    data.tfstate = getInt("tfstate");
    data.pushmsec = getInt("pushmsec");
    data.deadflag = getInt("deadflag");

    // physinfo string
    if (const DeltaValue* physinfoVal = delta.findEntryValue("physinfo")) {
        if (auto ptr = std::get_if<std::string>(physinfoVal)) {
            std::strncpy(data.physinfo, ptr->c_str(), sizeof(data.physinfo));
            data.physinfo[sizeof(data.physinfo)-1] = '\0';
        }
    }

    // Mod-specific scalars
    data.iuser1 = getInt("iuser1");
    data.iuser2 = getInt("iuser2");
    data.iuser3 = getInt("iuser3");
    data.iuser4 = getInt("iuser4");
    data.fuser1 = getFloat("fuser1");
    data.fuser2 = getFloat("fuser2");
    data.fuser3 = getFloat("fuser3");
    data.fuser4 = getFloat("fuser4");

    return data;
}

inline EntityStatePlayer toEntityStatePlayer(const HalfLifeDelta& delta) 
{
    EntityStatePlayer e{};

    auto getFloat = [&](const char* name){ return toFloat(*delta.findEntryValue(name)); };
    auto getInt = [&](const char* name){ 
        if (const DeltaValue* val = delta.findEntryValue(name)) {
            if (auto p = std::get_if<int32_t>(val)) return *p;
            if (auto p = std::get_if<uint32_t>(val)) return static_cast<int>(*p);
        }
        return 0;
    };
    auto getVec3 = [&](const char* base, float out[3]){
        out[0] = getFloat((std::string(base) + "[0]").c_str());
        out[1] = getFloat((std::string(base) + "[1]").c_str());
        out[2] = getFloat((std::string(base) + "[2]").c_str());
    };

    e.animtime = getFloat("animtime");
    e.frame = getFloat("frame");
    getVec3("origin", e.origin);
    getVec3("angles", e.angles);
    getVec3("mins", e.mins);
    getVec3("maxs", e.maxs);
    getVec3("basevelocity", e.basevelocity);

    e.gaitsequence = getInt("gaitsequence");
    e.sequence = getInt("sequence");
    e.modelindex = getInt("modelindex");
    e.movetype = getInt("movetype");
    e.solid = getInt("solid");
    e.weaponmodel = getInt("weaponmodel");
    e.team = getInt("team");
    e.playerclass = getInt("playerclass");
    e.owner = getInt("owner");
    e.effects = getInt("effects");
    e.framerate = getFloat("framerate");
    e.skin = getInt("skin");
    e.body = getInt("body");
    e.rendermode = getInt("rendermode");
    e.renderamt = getInt("renderamt");
    e.renderfx = getInt("renderfx");
    e.scale = getFloat("scale");
    e.friction = getFloat("friction");
    e.usehull = getInt("usehull");
    e.gravity = getFloat("gravity");
    e.aiment = getInt("aiment");
    e.spectator = getInt("spectator");

    // controller and blending
    for(int i = 0; i < 4; ++i) e.controller[i] = static_cast<uint8_t>(getInt(("controller" + std::to_string(i)).c_str()));
    for(int i = 0; i < 2; ++i) e.blending[i] = static_cast<uint8_t>(getInt(("blending" + std::to_string(i)).c_str()));

    e.rendercolor.r = static_cast<uint8_t>(getInt("rendercolor.r"));
    e.rendercolor.g = static_cast<uint8_t>(getInt("rendercolor.g"));
    e.rendercolor.b = static_cast<uint8_t>(getInt("rendercolor.b"));

    return e;
}

inline CustomEntityState toCustomEntityState(const HalfLifeDelta& delta)
{
    CustomEntityState e{};

    auto getFloat = [&](const char* name) -> float {
        if (const DeltaValue* val = delta.findEntryValue(name)) {
            return toFloat(*val);
        }
        return 0.0f;
    };

    auto getInt = [&](const char* name) -> int {
        if (const DeltaValue* val = delta.findEntryValue(name)) {
            if (auto ptr = std::get_if<int32_t>(val)) return *ptr;
            if (auto ptr = std::get_if<uint32_t>(val)) return static_cast<int>(*ptr);
        }
        return 0;
    };

    auto getVec3 = [&](const char* baseName, float out[3]) {
        out[0] = getFloat((std::string(baseName) + "[0]").c_str());
        out[1] = getFloat((std::string(baseName) + "[1]").c_str());
        out[2] = getFloat((std::string(baseName) + "[2]").c_str());
    };

    // Populate vectors
    getVec3("origin", e.origin);
    getVec3("angles", e.angles);

    // Scalars
    e.rendermode = getInt("rendermode");
    e.sequence = getInt("sequence");
    e.skin = getInt("skin");
    e.modelindex = getInt("modelindex");
    e.scale = getFloat("scale"); // apply post multiplier if needed
    e.body = getInt("body");
    e.renderfx = getInt("renderfx");
    e.renderamt = getInt("renderamt");
    e.frame = getFloat("frame");
    e.animtime = getFloat("animtime"); // apply post multiplier if needed

    // Color
    e.rendercolor.r = static_cast<uint8_t>(getInt("rendercolor.r"));
    e.rendercolor.g = static_cast<uint8_t>(getInt("rendercolor.g"));
    e.rendercolor.b = static_cast<uint8_t>(getInt("rendercolor.b"));

    return e;
}

inline EventFrame ParseEventFrame(const std::vector<uint8_t>& data)
{
    BitBuffer bitBuffer(data);

    EventFrame frame;

    // Top-level event frame fields
    frame.flags = bitBuffer.readInt32();
    frame.index = bitBuffer.readInt32();
    frame.delay = bitBuffer.readFloat();

    EventFrame::EventArgs& args = frame.args;

    // Nested args
    args.flags = bitBuffer.readInt32();
    args.entityIndex = bitBuffer.readInt32();

    // origin[3]
    args.origin[0] = bitBuffer.readFloat();
    args.origin[1] = bitBuffer.readFloat();
    args.origin[2] = bitBuffer.readFloat();

    // angles[3]
    args.angles[0] = bitBuffer.readFloat();
    args.angles[1] = bitBuffer.readFloat();
    args.angles[2] = bitBuffer.readFloat();

    // velocity[3]
    args.velocity[0] = bitBuffer.readFloat();
    args.velocity[1] = bitBuffer.readFloat();
    args.velocity[2] = bitBuffer.readFloat();

    args.ducking = bitBuffer.readInt32();

    args.fparam1 = bitBuffer.readFloat();
    args.fparam2 = bitBuffer.readFloat();

    args.iparam1 = bitBuffer.readInt32();
    args.iparam2 = bitBuffer.readInt32();

    args.bparam1 = bitBuffer.readInt32();
    args.bparam2 = bitBuffer.readInt32();

    return frame;
}
