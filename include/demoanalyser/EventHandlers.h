#pragma once

#include <demoanalyser/DemoStructs.h>

#include <string>

extern void OnReadHeader(const DemoHeader demoHeader) __attribute__((weak));
extern void OnConsoleCommand(const std::string& command) __attribute__((weak)); // FRAME TYPE 3
extern void OnPlayerState(PlayerState& playerState) __attribute__((weak));      // FRAME TYPE 4
extern void OnEventFrame(const EventFrame& eventFrame) __attribute__((weak));   // FRAME TYPE 6


// EVENTS
extern void OnTimeTick(float Time) __attribute__((weak));
extern void OnMessagePrint(const std::string& message) __attribute__((weak));
extern void OnClientData(ClientData& clientData) __attribute__((weak));
extern void OnNewMoveVars(MoveVars& moveVars) __attribute__((weak));
extern void OnUpdateUserInfo(UpdateUserInfo& updateUserInfo) __attribute__((weak));
extern void OnServerInfo(ServerInfo& serverInfo) __attribute__((weak));

extern void OnSetAngle(Angle angle) __attribute__((weak));
extern void OnPackedPlayerEntity(EntityStatePlayer entityStatePlayer)  __attribute__((weak));
extern void OnPackedCustomEntity(CustomEntityState customEntityState)  __attribute__((weak));
extern void OnDeltaPackedPlayerEntity(EntityStatePlayer entityStatePlayer)  __attribute__((weak));
extern void OnDeltaPackedCustomEntity(CustomEntityState customEntityState)  __attribute__((weak));


