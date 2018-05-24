#include "Platform.h"
#include "MoREApp.h"

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow
)
{
	MoREApp app( hInstance );
	if( !app.Init() )
	{
		return -1;
	}

	return app.Run();
}