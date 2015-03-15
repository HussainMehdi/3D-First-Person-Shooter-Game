
#include "mainmenu.h"
#include "q3factory.h"

	int IRRCALLCONV main(int argc, char* argv[])
{
	 
	//mainmenu();
	
	path prgname(argv[0]);
	GameData game ( deletePathFromPath ( prgname, 1 ) );

	// dynamically load irrlicht
	const c8 * dllName ="irrlicht.dll";
	game.createExDevice = load_createDeviceEx ( dllName );
	if ( 0 == game.createExDevice )
	{
		game.retVal = 3;
		return game.retVal; // could not load dll
	}

	// start without asking for driver
	game.retVal = 1;
	
	  // ... initialize Irrlicht and a font

	
	while(game.retVal!=9){	
		mainmenu(&game);
		runGame ( &game );
		//game.retVal=0;
	}


	}
	

