# DemoAnalyser ( Counter Strike 1.6 Demo Parser )

This project is a C++ library for reading and parsing **Counter-Strike 1.6 / GoldSrc**
`.dem` demo files.  
It extracts the full event stream (frames + network events) and exposes it to the
user through a **callback-based interface**, allowing external programs to hook
into demo playback and process the data.

---

## Features

- Parse standard GoldSrc demo file structure
- Read all known frame types:
  - **Type 1** – Demo start / header
  - **Type 2** – Unknown/unused in most demos
  - **Type 3** – `ConsoleCommand`
  - **Type 4** – `ClientData` / player state
  - **Type 5** – Next Section
  - **Type 6** – Event frames
  - **Type 7** – Weapon Anim // TODO
  - **Type 8** – Sound // TODO
  - **Type 9** – Demo Buffer // TODO
- Parse some GoldSrc game events inside network frames:
  - Time ticks
  - Message print events
  - Client data
  - Server info
  - UpdateUserInfo
  - Packed & DeltaPacked entity updates (players + custom entities)

  More can be added quite easily
  
- All handlers are **optional** thanks to weak linking

---

## How It Works

The parser walks through the `.dem` file sequentially and dispatches events in the exact order they appear.

Users can hook into the parser by **defining their own functions** in their application.  
Every function has a default weak implementation:

```cpp
extern void OnReadHeader(const DemoHeader demoHeader) __attribute__((weak));
extern void OnConsoleCommand(const std::string& command) __attribute__((weak));
extern void OnPlayerState(PlayerState& playerState) __attribute__((weak));
extern void OnEventFrame(const EventFrame& eventFrame) __attribute__((weak));
