#include "Initialize.h"
#include "player.h"
#include "server.h"
#include "client.h"
int mainmenu(GameData *game){
bool menu[3]={true,false,false};
	Keystroke keys; 
	int eventtime=0, res_sel=0;
// deviceType , windowSize , bits , fullscreen , stencilbuffer, vsync , eventReceiver
	game->Device =
		createDevice( video::EDT_DIRECT3D9, dimension2d<u32>(800, 600), 32, false, false, true, &keys);
	IrrlichtDevice *device=game->Device;
	if (!device)
		return 1;

	ISoundEngine* sound=createIrrKlangDevice();
	ISoundEngine* sfx=createIrrKlangDevice();
	device->setWindowCaption(L"Destructo Beam");
	IVideoDriver* driver = device->getVideoDriver();
	ISceneManager* smgr = device->getSceneManager();
	IGUIEnvironment* guienv = device->getGUIEnvironment();
	gui::IGUIEnvironment* gui = device->getGUIEnvironment();
	
   // gui->addImage(driver->getTexture("load.jpg"),core::position2d<s32>(0, 0));
	video::ITexture* images = driver->getTexture("load.jpg");
	
	//driver->makeColorKeyTexture(images, core::position2d<s32>(0,0)); // Making texture color transparent
	
	gui::IGUIFont* font = device->getGUIEnvironment()->getFont("Fonts\\destructo_font.xml"); // Installing Custom font
	float music=100,sfx_vol=100;
	sound->play2D("sounds\\gamestartup.mp3",true);
	
	//guienv->addStaticText(L"Sample Text!",rect<s32>(10,10,260,22), true);
	int  choice=0,color[4]={255,180,180,180};
	while(device->run())
	{
		sound->setSoundVolume(music/100);
		sfx->setSoundVolume(sfx_vol/100);
		int time = device->getTimer()->getTime();
		driver->beginScene(true, true, SColor(0,0,0,0));
		driver->draw2DImage(images, core::position2d<s32>(0,0),
                core::rect<s32>(0,0,800,600), 0,
                video::SColor(255,255,255,255), true);
		if(!font) font=device->getGUIEnvironment()->getBuiltInFont();
		if (font && menu[0]==true)
		 {
				
				if(keys.IsKeyDown(irr::KEY_DOWN) && (time-eventtime>170) ){
					if (choice==0){color[0]=180;color[1]=255;choice++;}
					else if (choice==1){color[1]=180;color[2]=255;choice++;}
					else if (choice==2){color[2]=180;color[3]=255;choice++;}
					else if (choice==3){color[3]=180;color[0]=255;choice=0;}
					sfx->play2D("sounds\\move.wav");
					eventtime=time;
				}
				if(keys.IsKeyDown(irr::KEY_UP) && (time-eventtime>170) ){
					if (choice==0){color[0]=180;color[3]=255;choice=3;}
					else if (choice==1){color[1]=180;color[0]=255;choice--;}
					else if (choice==2){color[2]=180;color[1]=255;choice--;}
					else if (choice==3){color[3]=180;color[2]=255;choice--;}
					sfx->play2D("sounds\\move.wav");
					eventtime=time;
				}
				font->draw(L"Start Singleplayer",
					core::rect<s32>( 30 /* x */ , 275 /* y */,0,0),
					video::SColor(color[0],255,255,255));

				font->draw(L"Start Multiplayer",
					core::rect<s32>( 30 /* x */ , 315 /* y */,0,0),
					video::SColor(color[1],255,250,250));

				font->draw(L"Options",
					core::rect<s32>( 30 /* x */ , 355 /* y */,0,0),
					video::SColor(color[2],255,250,250));

				font->draw(L"Exit",
					core::rect<s32>( 30 /* x */ , 395 /* y */,0,0),
					video::SColor(color[3],255,250,250));
				
				if(keys.IsKeyDown(irr::KEY_RETURN) ) {
					if (choice==0){break;}
					if (choice==1){start_server();}
					if (choice==2){menu[2]=true; menu[0]=false; }
					if (choice==3){	 
						game->retVal=9;

						game->Device->closeDevice(); 
						
					}
					sfx->play2D("sounds\\click.wav");
					
				}
		 }
				if (menu[2]==true)
				{   choice=0; color[0]=255;color[1]=180;color[2]=180;color[3]=180;
					static int colr[4]={255,180,180,180} ,  choice1=0;
					if(keys.IsKeyDown(irr::KEY_DOWN) && (time-eventtime>170) ){
					if (choice1==0){colr[0]=180;colr[1]=255;choice1++;}
					else if (choice1==1){colr[1]=180;colr[2]=255;choice1++;}
					else if (choice1==2){colr[2]=180;colr[3]=255;choice1++;}
					else if (choice1==3){colr[3]=180;colr[0]=255;choice1=0;}
					sfx->play2D("sounds\\move.wav");
					eventtime=time;
				}
				if(keys.IsKeyDown(irr::KEY_UP) && (time-eventtime>170) ){
					if (choice1==0){colr[0]=180;colr[3]=255;choice1=3;}
					else if (choice1==1){colr[1]=180;colr[0]=255;choice1--;}
					else if (choice1==2){colr[2]=180;colr[1]=255;choice1--;}
					else if (choice1==3){colr[3]=180;colr[2]=255;choice1--;}
					sfx->play2D("sounds\\move.wav");
					eventtime=time;
				}

					std::string res[10]={"640 x 480", "720 x 480", "800 x 480" , "800 x 600" , "1024 x 480" ,
						                 "1024 x 600" , "1024 x 768" , "1152 x 864" , "1280 x 720" , "1440 x 900"};
					menu[0]=false;
					
					font->draw("Screen Resolution",
					core::rect<s32>( 130 /* x */ , 175 /* y */,0,0),
					video::SColor(colr[0],255,255,255));

					font->draw(res[res_sel].c_str(),
					core::rect<s32>( 380 /* x */ , 175 /* y */,0,0),
					video::SColor(colr[0],255,255,255));

					font->draw("Music",
					core::rect<s32>( 130 /* x */ , 210 /* y */,0,0),
					video::SColor(colr[1],255,255,255));
					std::string sfx_str , music_str ;
					
					std::stringstream buffer_music , buffer_sfx;
					buffer_music<<music;
					buffer_music>>music_str;
					font->draw(music_str.c_str(),
					core::rect<s32>( 380 /* x */ , 210 /* y */,0,0),
					video::SColor(colr[1],255,255,255));

					font->draw("Sfx",
					core::rect<s32>( 130 /* x */ , 245 /* y */,0,0),
					video::SColor(colr[2],255,255,255));
					buffer_sfx<<sfx_vol;
					buffer_sfx>>sfx_str;
					font->draw(sfx_str.c_str(),
					core::rect<s32>( 380 /* x */ , 245 /* y */,0,0),
					video::SColor(colr[2],255,255,255));

					font->draw("Back",
					core::rect<s32>( 275 /* x */ , 290 /* y */,0,0),
					video::SColor(colr[3],255,255,255));

					if (keys.IsKeyDown(irr::KEY_RIGHT)  && time-eventtime>170 )
					{  
					   
					   if (choice1==0 && res_sel<9) {res_sel++; eventtime=time;}
					   else if (choice1==1 && music<100) { eventtime=time; music+=10; }
					   else if (choice1==2 && sfx_vol<100 ) { eventtime=time; sfx_vol+=10; }
					   sfx->play2D("sounds\\move.wav");
					}

					if (keys.IsKeyDown(irr::KEY_LEFT)  && time-eventtime>170 )
					{ 
					   
					   if (choice1==0 && res_sel>0 ) {res_sel--; eventtime=time;}
					   else if (choice1==1 && music>=10 ) { eventtime=time; music-=10; }
					   else if (choice1==2 && sfx_vol>=10 ) { eventtime=time; sfx_vol-=10; }
					   sfx->play2D("sounds\\move.wav");
					}

					if ( keys.IsKeyDown(irr::KEY_RETURN )  )
					 {
						 if (choice1==3) {menu[0]=true; menu[2]=false;choice1=0; colr[0]=255;colr[1]=180;colr[2]=180;colr[3]=180;   }
					 }

					if(keys.IsKeyDown(irr::KEY_ESCAPE) ) {menu[0]=true; menu[2]=false;}

				}

				smgr->drawAll();
		guienv->drawAll();
		driver->endScene();
	}
	//device->drop();	
}