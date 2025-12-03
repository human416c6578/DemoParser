#include <demoanalyser/DemoParser.h>
#include <demoanalyser/EventHandlers.h>

void OnReadHeader(const DemoHeader demoHeader)
{
    std::cout<<demoHeader.mapName<<std::endl;

    std::cout<<demoHeader.demoDirectory[1].frameCount<<std::endl;
    std::cout<<demoHeader.demoDirectory[1].trackTime<<std::endl;
}

void OnUpdateUserInfo(UpdateUserInfo &updateUserInfo)
{
    //std::cout<<updateUserInfo.ClientUserInfo<<std::endl;
}

void OnServerInfo(ServerInfo &serverInfo)
{
    std::cout<<serverInfo.Hostname<<std::endl;
}

void OnClientData(ClientData& clientData) 
{
    //std::cout<<clientData.origin[0]<<" "<<clientData.origin[1]<<" "<<clientData.origin[2]<<std::endl;
}

void OnPlayerState(PlayerState& playerState)
{
    //std::cout<<playerState.position[0]<<" "<<playerState.position[1]<<" "<<playerState.position[2]<<std::endl;
}

void OnTimeTick(float Time)
{
    //std::cout<<Time<<std::endl;
}
void OnMessagePrint(const std::string &message)
{
    //std::cout<<message<<std::endl;
}

void OnConsoleCommand(const std::string &command)
{
    //std::cout<<command<<std::endl;
}

void OnEventFrame(const EventFrame &eventFrame)
{
    //std::cout<<eventFrame.index<<std::endl;
}

void OnNewMoveVars(MoveVars &moveVars)
{
    //std::cout<<moveVars.maxspeed<<std::endl;
}
int main(int argc, char* argv[]) 
{
    if (argc < 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    const char* filename = argv[1];

    // Read demo header
    demo_analyser::DemoParser demoParser(filename);
    demoParser.parseDemo();

    //demo_analyser::PrintHeader(demo.header);

    return 0;
}