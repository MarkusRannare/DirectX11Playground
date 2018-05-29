#include "Platform.h"
#include "Example2App.h"

int CALLBACK WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR     lpCmdLine,
	int       nCmdShow
)
{
	Example2App app( hInstance );
	if( !app.Init() )
	{
		return -1;
	}

	return app.Run();
}