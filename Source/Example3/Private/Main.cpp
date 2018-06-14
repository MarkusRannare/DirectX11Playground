#include "Platform.h"
#include "Example3App.h"

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow
)
{
	Example3App app( hInstance );
	if( !app.Init() )
	{
		return -1;
	}

	return app.Run();
}