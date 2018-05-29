#include "Platform.h"
#include "Example1App.h"

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow
)
{
	Example1App app( hInstance );
	if( !app.Init() )
	{
		return -1;
	}

	return app.Run();
}