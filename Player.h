#include <irrlicht.h>
#include "q3factory.h"
#include "sound.h"

/*
	Game Data is used to hold Data which is needed to drive the game
*/
int time,eventtime=0,once=0,health=100;
bool mapload=false,drawn=false;
int enemyx=-90,enemyy=-15,enemyz=-140;
vector3df player;
scene::IAnimatedMeshSceneNode* modelNode ;
/*vector3df( -90, -15, -140)*/
vector3df update()
{return player;}
struct GameData
{
	GameData ( const path &startupDir) : retVal(0), StartupDir(startupDir), createExDevice(0), Device(0)
	{
		setDefault ();
	}


	void setDefault ();
//	s32 save ( const path &filename );
	//s32 load ( const path &filename );

	s32 debugState;
	s32 gravityState;
	s32 flyTroughState;
	s32 wireFrame;
	s32 guiActive;
	s32 guiInputActive;
	f32 GammaValue;
	s32 retVal;
	s32 sound;

	path StartupDir;
	stringw CurrentMapName;
	array<path> CurrentArchiveList;

	vector3df PlayerPosition;
	vector3df PlayerRotation;

	tQ3EntityList Variable;

	Q3LevelLoadParameter loadParam;
	SIrrlichtCreationParameters deviceParam;
	funcptr_createDeviceEx createExDevice;
	IrrlichtDevice *Device;
};

/*
	set default settings
*/
IAnimatedMeshSceneNode* WeaponNode;
IAnimatedMeshSceneNode* Enemy=0;

void GameData::setDefault ()
{
	debugState = EDS_OFF;
	gravityState = 1;
	flyTroughState = 0;
	wireFrame = 0;
	guiActive = 1;
	guiInputActive = 0;
	GammaValue = 0.f;

	// default deviceParam;
#if defined ( _IRR_WINDOWS_ )
	deviceParam.DriverType = EDT_DIRECT3D9;
#else
	deviceParam.DriverType = EDT_OPENGL;
#endif
	deviceParam.WindowSize.Width = 800;
	deviceParam.WindowSize.Height = 600;
	deviceParam.Fullscreen = false;
	deviceParam.Bits = 24;
	deviceParam.ZBufferBits = 32;
	deviceParam.Vsync = false;
	deviceParam.AntiAlias = false;
	

	// default Quake3 loadParam
	loadParam.defaultLightMapMaterial = EMT_LIGHTMAP;
	loadParam.defaultModulate = EMFN_MODULATE_1X;
	loadParam.defaultFilter = EMF_ANISOTROPIC_FILTER;
	loadParam.verbose = 2;
	loadParam.mergeShaderBuffer = 1;		// merge meshbuffers with same material
	loadParam.cleanUnResolvedMeshes = 1;	// should unresolved meshes be cleaned. otherwise blue texture
	loadParam.loadAllShaders = 1;			// load all scripts in the script directory
	loadParam.loadSkyShader = 0;			// load sky Shader
	loadParam.alpharef = 1;

	sound = 0;

	CurrentMapName = "";
	CurrentArchiveList.clear ();

	// Explorer Media directory
	//CurrentArchiveList.push_back ( StartupDir );

	// Add the original quake3 files before you load your custom map
	// Most mods are using the original shaders, models&items&weapons
	

	
}


/*
	Representing a player
*/
struct Q3Player : public IAnimationEndCallBack
{
	Q3Player ()
	: Device(0), MapParent(0), Mesh(0),  StartPositionCurrent(0)
	{
		animation[0] = 0;
		memset(Anim, 0, sizeof(TimeFire)*4);
	}

	virtual void OnAnimationEnd(IAnimatedMeshSceneNode* node);

	void create (	IrrlichtDevice *device,
					IQ3LevelMesh* mesh,
					ISceneNode *mapNode,
					IMetaTriangleSelector *meta
				);
	void shutdown ();
	void setAnim ( const c8 *name );
	void respawn ();
	void setpos ( const vector3df &pos, const vector3df& rotation );

	ISceneNodeAnimatorCollisionResponse * cam() { return camCollisionResponse ( Device ); }

	IrrlichtDevice *Device;
	ISceneNode* MapParent;
	IQ3LevelMesh* Mesh;
	
	s32 StartPositionCurrent;
	TimeFire Anim[4];
	c8 animation[64];
	c8 buf[64];
};


/* End player
*/
void Q3Player::shutdown ()
{
	setAnim ( 0 );

	dropElement (WeaponNode);

	if ( Device )
	{
		ICameraSceneNode* camera = Device->getSceneManager()->getActiveCamera();
		dropElement ( camera );
		Device = 0;
	}

	MapParent = 0;
	Mesh = 0;
}


/* create a new player
*/
void Q3Player::create ( IrrlichtDevice *device, IQ3LevelMesh* mesh, ISceneNode *mapNode, IMetaTriangleSelector *meta )
{
	setTimeFire ( Anim + 0, 200, FIRED );
	setTimeFire ( Anim + 1, 5000 );

	if (!device)
		return;
	// load FPS weapon to Camera
	Device = device;
	Mesh = mesh;
	MapParent = mapNode;

	ISceneManager *smgr = device->getSceneManager ();
	IVideoDriver * driver = device->getVideoDriver();

	device->getGUIEnvironment()->addImage(
		driver->getTexture("crosshair.png"),
		core::position2d<s32>(300,200));
	device->getGUIEnvironment()->addImage(
		driver->getTexture("health.png"),
		core::position2d<s32>(15,510));

	ICameraSceneNode* camera = 0;

	SKeyMap keyMap[10];
	keyMap[0].Action = EKA_MOVE_FORWARD;
	keyMap[0].KeyCode = KEY_UP;
	keyMap[1].Action = EKA_MOVE_FORWARD;
	keyMap[1].KeyCode = KEY_KEY_W;

	keyMap[2].Action = EKA_MOVE_BACKWARD;
	keyMap[2].KeyCode = KEY_DOWN;
	keyMap[3].Action = EKA_MOVE_BACKWARD;
	keyMap[3].KeyCode = KEY_KEY_S;

	keyMap[4].Action = EKA_STRAFE_LEFT;
	keyMap[4].KeyCode = KEY_LEFT;
	keyMap[5].Action = EKA_STRAFE_LEFT;
	keyMap[5].KeyCode = KEY_KEY_A;

	keyMap[6].Action = EKA_STRAFE_RIGHT;
	keyMap[6].KeyCode = KEY_RIGHT;
	keyMap[7].Action = EKA_STRAFE_RIGHT;
	keyMap[7].KeyCode = KEY_KEY_D;

	keyMap[8].Action = EKA_JUMP_UP;
	keyMap[8].KeyCode = KEY_LSHIFT;

	keyMap[9].Action = EKA_CROUCH;
	keyMap[9].KeyCode = KEY_KEY_C;

	camera = smgr->addCameraSceneNodeFPS(0, 100.0f, 0.21f, -1, keyMap, 9, false, 9.0f);
	
	//camera->setFOV ( 100.f * core::DEGTORAD );giu
	camera->setFarValue( 20000.f );
	
	

	/*video::SMaterial material;
	Enemy = smgr->addAnimatedMeshSceneNode(smgr->getMesh("faerie.md2"),0, 0);
	Enemy->setPosition(core::vector3df(-90,-15,-140)); // Put its feet on the floor.
	Enemy->setScale(core::vector3df(1.6f)); // Make it appear realistically scaled
	Enemy->setMD2Animation(scene::EMAT_ATTACK);
	Enemy->setAnimationSpeed(20.f);
	material.setTexture(0, driver->getTexture("faerie2.bmp"));*/
	// ENEMY ENDS



	IAnimatedMeshMD2* weaponMesh = (IAnimatedMeshMD2*) smgr->getMesh("m16.md2"); //Can also change with gun.md2
	if ( 0 == weaponMesh )
		return;

	if ( weaponMesh->getMeshType() == EAMT_MD2 )
	{
		s32 count = weaponMesh->getAnimationCount();
		for ( s32 i = 0; i != count; ++i )
		{
			snprintf ( buf, 64, "Animation: %s", weaponMesh->getAnimationName(i) );
			device->getLogger()->log(buf, ELL_INFORMATION);
		}
	}

	WeaponNode = smgr->addAnimatedMeshSceneNode(
						weaponMesh,
						smgr->getActiveCamera(),
						10,
						vector3df( 0, 0, 0),
						vector3df(-90,-90,90)
						);
	WeaponNode->setMaterialFlag(EMF_LIGHTING, false);
	WeaponNode->setMaterialTexture(0, driver->getTexture( "m16.png"));//Can also change with gun.jpg
	//WeaponNode->setFrameLoop(50,51);
	WeaponNode->setMD2Animation ( "stand" );
	//WeaponNode->setAnimationSpeed(2);
	WeaponNode->setLoopMode ( true );



	//create a collision auto response animator
	ISceneNodeAnimator* anim =
		smgr->createCollisionResponseAnimator( meta, camera,
			vector3df(30,45,30),
			getGravity ( "earth" ),
			vector3df(0,40,0),
			0.0005f
		);
	camera->addAnimator( anim );
	anim->drop();
	if ( meta )
	{
		meta->drop ();
	}

	respawn ();
	setAnim ( "idle" );
}

/*
	so we need a good starting Position in the level.
	we can ask the Quake3 Loader for all entities with class_name "info_player_deathmatch"
*/
void Q3Player::respawn ()
{
	if (!Device)
		return;
	ICameraSceneNode* camera = Device->getSceneManager()->getActiveCamera();

	

	if ( StartPositionCurrent >= Q3StartPosition (
			Mesh, camera,StartPositionCurrent++,
			cam ()->getEllipsoidTranslation() )
		)
	{
		StartPositionCurrent = 0;
	}
}

/*
	set Player position from saved coordinates
*/
void Q3Player::setpos ( const vector3df &pos, const vector3df &rotation )
{
	if (!Device)
		return;
	ICameraSceneNode* camera = Device->getSceneManager()->getActiveCamera();
	if ( camera )
	{
		camera->setPosition ( pos );
		camera->setRotation ( rotation );
		//! New. FPSCamera and animators catches reset on animate 0
		camera->OnAnimate ( 0 );
	}
}

/* set the Animation of the player and weapon
*/
void Q3Player::setAnim ( const c8 *name )
{
	if ( name )
	{
		if ( WeaponNode )
		{
			WeaponNode->setAnimationEndCallback ( this );
			WeaponNode->setMD2Animation ( animation );
		}
	}
	else
	{
		animation[0] = 0;
		if ( WeaponNode )
		{
			WeaponNode->setAnimationEndCallback ( 0 );
		}
	}
}


// Callback
void Q3Player::OnAnimationEnd(IAnimatedMeshSceneNode* node)
{
	setAnim ( 0 );
}



/* GUI Elements
*/
struct GUI
{
	GUI ()
	{
		memset ( this, 0, sizeof ( *this ) );
	}

	void drop()
	{
		dropElement ( Window );

	}

	
	IGUIButton* SetVideoMode;

	
	IGUIScrollBar* Gamma;
	
	
	IGUITable* ArchiveList;
	
	
	IGUIListBox* MapList;
	IGUIStaticText* StatusLine;
	IGUIWindow* Window;
};


/*
	CQuake3EventHandler controls the game
*/
class CQuake3EventHandler : public IEventReceiver
{
public:
	CQuake3EventHandler( GameData *gameData );
	virtual ~CQuake3EventHandler ();

	void Animate();
	void Render();

	void AddArchive ( const path& archiveName );
	void LoadMap ( const stringw& mapName, s32 collision );
	void CreatePlayers();
	void AddSky( u32 dome, const c8 *texture );
	Q3Player *GetPlayer ( u32 index ) { return &Player[index]; }
	void Enemy();
	void CreateGUI();
	void SetGUIActive( s32 command);

	bool OnEvent(const SEvent& eve);
	Q3Player Player[2];

private:

	GameData *Game;

	IQ3LevelMesh* Mesh;
	ISceneNode* MapParent;
	ISceneNode* ShaderParent;
	ISceneNode* ItemParent;
	ISceneNode* UnresolvedParent;
	ISceneNode* BulletParent;
	ISceneNode* FogParent;
	ISceneNode * SkyNode;
	IMetaTriangleSelector *Meta;
	gui::IGUIFont* font_health ;
	c8 buf[256];

	

	struct SParticleImpact
	{
		u32 when;
		vector3df pos;
		vector3df outVector;
	};
	array<SParticleImpact> Impacts;
	void useItem( Q3Player * player);
	void createParticleImpacts( u32 now );

	void createTextures ();
	void addSceneTreeItem( ISceneNode * parent, IGUITreeViewNode* nodeParent);

	GUI gui;
	void dropMap ();
};
void CQuake3EventHandler:: Enemy()
{

	if (drawn == false)
	{
	ISceneManager* smgr1 = Game->Device->getSceneManager();
	ISceneNode* camera = smgr1->getActiveCamera();
	IVideoDriver * driver1 = Game->Device->getVideoDriver();
	scene::IAnimatedMesh* mesh1 = smgr1->getMesh("dwarf.x");
	modelNode = smgr1->addAnimatedMeshSceneNode(mesh1);
	vector3df start = camera->getPosition();
	player=start;
	
	enemyx=start.X;
	enemyy=start.Y;
	enemyz=start.Z;
	enemyx-=(time)%60;
	enemyy-=(time)%60;
	enemyz-=(time)%60;
	
	if (modelNode)
	{
		modelNode->setPosition( vector3df(enemyx,enemyy,enemyz) );
		modelNode->setMaterialTexture(0, driver1->getTexture("dwarf.jpg"));
		modelNode->setMaterialFlag(video::EMF_LIGHTING, true);
		modelNode->setMD2Animation(scene::EMAT_CROUCH_WALK);
	}
	drawn=true;
	}
	else 	{modelNode->setPosition( vector3df(enemyx,enemyy,enemyz) );}
}

/* Constructor
*/
CQuake3EventHandler::CQuake3EventHandler( GameData *game )
: Game(game), Mesh(0), MapParent(0), ShaderParent(0), ItemParent(0), UnresolvedParent(0),
	BulletParent(0), FogParent(0), SkyNode(0), Meta(0)
{
	buf[0]=0;
	font_health = game->Device->getGUIEnvironment()->getFont("Fonts\\destructo_font.xml"); // Installing Custom font
	// Also use 16 Bit Textures for 16 Bit RenderDevice
	if ( Game->deviceParam.Bits == 16 )
	{
		game->Device->getVideoDriver()->setTextureCreationFlag(ETCF_ALWAYS_16_BIT, true);
	}

	// Quake3 Shader controls Z-Writing
	game->Device->getSceneManager()->getParameters()->setAttribute(scene::ALLOW_ZWRITE_ON_TRANSPARENT, true);

	// create internal textures
	createTextures ();

	sound_init ( game->Device );

	Game->Device->setEventReceiver ( this );
}


// destructor
CQuake3EventHandler::~CQuake3EventHandler ()
{
	Player[0].shutdown ();
	sound_shutdown ();


	Game->Device->drop();
}


// create runtime textures smog, fog
void CQuake3EventHandler::createTextures()
{
	IVideoDriver * driver = Game->Device->getVideoDriver();

	dimension2du dim(64, 64);

	video::IImage* image;
	u32 i;
	u32 x;
	u32 y;
	u32 * data;
	for ( i = 0; i != 8; ++i )
	{
		image = driver->createImage ( video::ECF_A8R8G8B8, dim);
		data = (u32*) image->lock ();
		for ( y = 0; y != dim.Height; ++y )
		{
			for ( x = 0; x != dim.Width; ++x )
			{
				data [x] = 0xFFFFFFFF;
			}
			data = (u32*) ( (u8*) data + image->getPitch() );
		}
		image->unlock();
		driver->addTexture( buf, image );
		image->drop ();
	}
	
}


/*
	create the GUI
*/
void CQuake3EventHandler::CreateGUI()
{

	IGUIEnvironment *env = Game->Device->getGUIEnvironment();
	IVideoDriver * driver = Game->Device->getVideoDriver();

	gui.drop();

	// set skin font
	IGUIFont* font = env->getFont("fontlucida.png");
	if (font)
		env->getSkin()->setFont(font);
	env->getSkin()->setColor ( EGDC_BUTTON_TEXT, video::SColor(240,0xAA,0xAA,0xAA) );
	env->getSkin()->setColor ( EGDC_3D_HIGH_LIGHT, video::SColor(240,0x22,0x22,0x22) );
	env->getSkin()->setColor ( EGDC_3D_FACE, video::SColor(240,0x44,0x44,0x44) );
	env->getSkin()->setColor ( EGDC_EDITABLE, video::SColor(240,0x44,0x44,0x44) );
	env->getSkin()->setColor ( EGDC_FOCUSED_EDITABLE, video::SColor(240,0x54,0x54,0x54) );
	env->getSkin()->setColor ( EGDC_WINDOW, video::SColor(240,0x66,0x66,0x66) );

	
	dimension2d<u32> dim ( 400, 600 );
	dimension2d<u32> vdim ( Game->Device->getVideoDriver()->getScreenSize() );


	gui.Window = env->addWindow ( rect<s32> ( 0, 0, dim.Width, dim.Height ), false, L"Destructo Beam" );
	gui.Window->getCloseButton()->setToolTipText ( L"Quit Destructo Beam" );

	// add a status line help text
	

	env->addStaticText ( L"Gamma:", rect<s32>( dim.Width - 400, 104, dim.Width - 310, 120 ),false, false, gui.Window, -1, false );
	gui.Gamma = env->addScrollBar( true, rect<s32>( dim.Width - 300, 104, dim.Width - 10, 120 ), gui.Window,-1 );
	gui.Gamma->setMin ( 50 );
	gui.Gamma->setMax ( 350 );
	gui.Gamma->setSmallStep ( 1 );
	gui.Gamma->setLargeStep ( 10 );
	gui.Gamma->setPos ( core::floor32 ( Game->GammaValue * 100.f ) );
	gui.Gamma->setToolTipText ( L"Adjust Gamma Ramp" );
	Game->Device->setGammaRamp ( Game->GammaValue, Game->GammaValue, Game->GammaValue, 0.f, 0.f );


	

	//Respawn = env->addButton ( rect<s32>( dim.Width - 260, 90, dim.Width - 10, 106 ), 0,-1, L"Respawn" );

	
	env->addStaticText ( L" Maps:\n                                   (Double-Click the Map to start the level)",
		rect<s32>( dim.Width - 400, dim.Height - 433, dim.Width - 5,dim.Height - 190 ),false, false, gui.Window, -1, false );
	gui.MapList = env->addListBox ( rect<s32>( 5,dim.Height - 400, dim.Width - 5,dim.Height - 40  ), gui.Window, -1, true  );
	gui.MapList->setToolTipText ( L"Double-Click the Map to start the level" );

	ifstream file1("maps\\maps.txt");
	std::string get;
	while(!file1.eof())
	{
		
		file1>>get;
		get="maps\\"+get;
		AddArchive ( get.c_str() );
	}
}


/*
	Add an Archive to the FileSystems and updates the GUI
*/
void CQuake3EventHandler::AddArchive ( const path& archiveName )
{
	IFileSystem *fs = Game->Device->getFileSystem();
	u32 i;

	if ( archiveName.size () )
	{
		bool exists = false;
		for ( i = 0; i != fs->getFileArchiveCount(); ++i )
		{
			if ( fs->getFileArchive(i)->getFileList()->getPath() == archiveName )
			{
				exists = true;
				break;
			}
		}

		if (!exists)
		{
			fs->addFileArchive(archiveName, true, false);
		}
	}

	// store the current archives in game data
	// show the attached Archive in proper order
	if ( gui.ArchiveList )
	{
		gui.ArchiveList->clearRows();

		for ( i = 0; i != fs->getFileArchiveCount(); ++i )
		{
			IFileArchive * archive = fs->getFileArchive ( i );

			u32 index = gui.ArchiveList->addRow(i);

			core::stringw typeName;
			switch(archive->getType())
			{
			case io::EFAT_ZIP:
				typeName = "ZIP";
				break;
			case io::EFAT_GZIP:
				typeName = "gzip";
				break;
			case io::EFAT_FOLDER:
				typeName = "Mount";
				break;
			case io::EFAT_PAK:
				typeName = "PAK";
				break;
			case io::EFAT_TAR:
				typeName = "TAR";
				break;
			default:
				typeName = "archive";
			}

			gui.ArchiveList->setCellText ( index, 0, typeName );
			gui.ArchiveList->setCellText ( index, 1, archive->getFileList()->getPath() );
		}
	}


	// browse the archives for maps
	if ( gui.MapList )
	{
		gui.MapList->clear();

		IGUISpriteBank *bank = Game->Device->getGUIEnvironment()->getSpriteBank("sprite_q3map");
		if ( 0 == bank )
			bank = Game->Device->getGUIEnvironment()->addEmptySpriteBank("sprite_q3map");

		SGUISprite sprite;
		SGUISpriteFrame frame;
		core::rect<s32> r;

		bank->getSprites().clear();
		bank->getPositions().clear ();
		gui.MapList->setSpriteBank ( bank );

		u32 g = 0;
		core::stringw s;

		// browse the attached file system
		fs->setFileListSystem ( FILESYSTEM_VIRTUAL );
		fs->changeWorkingDirectoryTo ( "/maps/" );
		IFileList *fileList = fs->createFileList ();
		fs->setFileListSystem ( FILESYSTEM_NATIVE );

		for ( i=0; i< fileList->getFileCount(); ++i)
		{
			s = fileList->getFullFileName(i);
			if ( s.find ( ".bsp" ) >= 0 )
			{
				// get level screenshot. reformat texture to 128x128
				path c ( s );
				deletePathFromFilename ( c );
				cutFilenameExtension ( c, c );
				c = path ( "levelshots/" ) + c;

				dimension2du dim ( 128, 128 );
				IVideoDriver * driver = Game->Device->getVideoDriver();
				IImage* image = 0;
				ITexture *tex = 0;
				path filename;

				filename = c + ".jpg";
				if ( fs->existFile ( filename ) )
					image = driver->createImageFromFile( filename );
				if ( 0 == image )
				{
					filename = c + ".tga";
					if ( fs->existFile ( filename ) )
						image = driver->createImageFromFile( filename );
				}

				if ( image )
				{
					IImage* filter = driver->createImage ( video::ECF_R8G8B8, dim );
					image->copyToScalingBoxFilter ( filter, 0 );
					image->drop ();
					image = filter;
				}

				if ( image )
				{
					tex = driver->addTexture ( filename, image );
					image->drop ();
				}


				bank->setTexture ( g, tex );

				r.LowerRightCorner.X = dim.Width;
				r.LowerRightCorner.Y = dim.Height;
				gui.MapList->setItemHeight ( r.LowerRightCorner.Y + 4 );
				frame.rectNumber = bank->getPositions().size();
				frame.textureNumber = g;

				bank->getPositions().push_back(r);

				sprite.Frames.set_used ( 0 );
				sprite.Frames.push_back(frame);
				sprite.frameTime = 0;
				bank->getSprites().push_back(sprite);

				gui.MapList->addItem ( s.c_str (), g );
				g += 1;
			}
		}
		fileList->drop ();

		gui.MapList->setSelected ( -1 );
		IGUIScrollBar * bar = (IGUIScrollBar*)gui.MapList->getElementFromId( 0 );
		if ( bar )
			bar->setPos ( 0 );

	}

}

/*
	clears the Map in Memory
*/
void CQuake3EventHandler::dropMap ()
{
	IVideoDriver * driver = Game->Device->getVideoDriver();

	driver->removeAllHardwareBuffers ();
	driver->removeAllTextures ();

	Player[0].shutdown ();


	dropElement ( ItemParent );
	dropElement ( ShaderParent );
	dropElement ( UnresolvedParent );
	dropElement ( FogParent );
	dropElement ( BulletParent );


	Impacts.clear();

	if ( Meta )
	{
		Meta = 0;
	}

	dropElement ( MapParent );
	dropElement ( SkyNode );

	// clean out meshes, because textures are invalid
	// TODO: better texture handling;-)
	IMeshCache *cache = Game->Device->getSceneManager ()->getMeshCache();
	cache->clear ();
	Mesh = 0;
}

/* Load new map
*/
void CQuake3EventHandler::LoadMap ( const stringw &mapName, s32 collision )
{
	if ( 0 == mapName.size() )
		return;

	dropMap ();

	IFileSystem *fs = Game->Device->getFileSystem();
	ISceneManager *smgr = Game->Device->getSceneManager ();

	IReadFile* file = fs->createMemoryReadFile(&Game->loadParam,
				sizeof(Game->loadParam), L"levelparameter.cfg", false);

	// load cfg file
	smgr->getMesh( file );
	file->drop ();

	// load the actual map
	Mesh = (IQ3LevelMesh*) smgr->getMesh(mapName);
	if ( 0 == Mesh )
		return;

	/*
		add the geometry mesh to the Scene ( polygon & patches )
		The Geometry mesh is optimised for faster drawing
	*/

	IMesh *geometry = Mesh->getMesh(E_Q3_MESH_GEOMETRY);
	if ( 0 == geometry || geometry->getMeshBufferCount() == 0)
		return;

	Game->CurrentMapName = mapName;

	//create a collision list
	Meta = 0;

	ITriangleSelector * selector = 0;
	if (collision)
		Meta = smgr->createMetaTriangleSelector();

	//IMeshBuffer *b0 = geometry->getMeshBuffer(0);
	//s32 minimalNodes = b0 ? core::s32_max ( 2048, b0->getVertexCount() / 32 ) : 2048;
	s32 minimalNodes = 2048;

	MapParent = smgr->addOctreeSceneNode(geometry, 0, -1, minimalNodes);
	MapParent->setName ( mapName );
	if ( Meta )
	{
		selector = smgr->createOctreeTriangleSelector( geometry,MapParent, minimalNodes);
		//selector = smgr->createTriangleSelector ( geometry, MapParent );
		Meta->addTriangleSelector( selector);
		selector->drop ();
	}

	// logical parent for the items
	ItemParent = smgr->addEmptySceneNode();


	ShaderParent = smgr->addEmptySceneNode();
	

	UnresolvedParent = smgr->addEmptySceneNode();
	

	FogParent = smgr->addEmptySceneNode();
	

	// logical parent for the bullets
	BulletParent = smgr->addEmptySceneNode();
	

	/*
		now construct SceneNodes for each Shader
		The Objects are stored in the quake mesh E_Q3_MESH_ITEMS
		and the Shader ID is stored in the MaterialParameters
		mostly dark looking skulls and moving lava.. or green flashing tubes?
	*/
	Q3ShaderFactory ( Game->loadParam, Game->Device, Mesh, E_Q3_MESH_ITEMS,ShaderParent, Meta, false );
	Q3ShaderFactory ( Game->loadParam, Game->Device, Mesh, E_Q3_MESH_FOG,FogParent, 0, false );
	Q3ShaderFactory ( Game->loadParam, Game->Device, Mesh, E_Q3_MESH_UNRESOLVED,UnresolvedParent, Meta, true );

	/*
		Now construct Models from Entity List
	*/
	Q3ModelFactory ( Game->loadParam, Game->Device, Mesh, ItemParent, false );
}

/*
	Adds a SceneNode with an icon to the Scene Tree
*/
void CQuake3EventHandler::addSceneTreeItem( ISceneNode * parent, IGUITreeViewNode* nodeParent)
{
	IGUITreeViewNode* node;
	wchar_t msg[128];

	s32 imageIndex;
	list<ISceneNode*>::ConstIterator it = parent->getChildren().begin();
	for (; it != parent->getChildren().end(); ++it)
	{
		switch ( (*it)->getType () )
		{
			case ESNT_Q3SHADER_SCENE_NODE: imageIndex = 0; break;
			case ESNT_CAMERA: imageIndex = 1; break;
			case ESNT_EMPTY: imageIndex = 2; break;
			case ESNT_MESH: imageIndex = 3; break;
			case ESNT_OCTREE: imageIndex = 3; break;
			case ESNT_ANIMATED_MESH: imageIndex = 4; break;
			case ESNT_SKY_BOX: imageIndex = 5; break;
			case ESNT_BILLBOARD: imageIndex = 6; break;
			case ESNT_PARTICLE_SYSTEM: imageIndex = 7; break;
			case ESNT_TEXT: imageIndex = 8; break;
			default:imageIndex = -1; break;
		}
		node = nodeParent->addChildBack( msg, 0, imageIndex );

		// Add all Animators
		list<ISceneNodeAnimator*>::ConstIterator ait = (*it)->getAnimators().begin();
		for (; ait != (*it)->getAnimators().end(); ++ait)
		{
			imageIndex = -1;
			switch ( (*ait)->getType () )
			{
				case ESNAT_FLY_CIRCLE:
				case ESNAT_FLY_STRAIGHT:
				case ESNAT_FOLLOW_SPLINE:
				case ESNAT_ROTATION:
				case ESNAT_TEXTURE:
				case ESNAT_DELETION:
				case ESNAT_COLLISION_RESPONSE:
				case ESNAT_CAMERA_FPS:
				case ESNAT_CAMERA_MAYA:
				default:
					break;
			}
			node->addChildBack( msg, 0, imageIndex );
		}

		addSceneTreeItem ( *it, node );
	}
}


// Adds life!
void CQuake3EventHandler::CreatePlayers()
{
	Player[0].create ( Game->Device, Mesh, MapParent, Meta );
	//Player[1].create ( Game->Device, Mesh, MapParent, Meta );
}


// Adds a skydome to the scene
void CQuake3EventHandler::AddSky( u32 dome, const c8 *texture)
{
	ISceneManager *smgr = Game->Device->getSceneManager ();
	IVideoDriver * driver = Game->Device->getVideoDriver();
	bool oldMipMapState = driver->getTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS);
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, false);
	if ( 0 == dome )
	{
		u32 i = 0;
		SkyNode = smgr->addSkyBoxSceneNode( driver->getTexture ( buf ), 0, 0, 0, 0, 0 );

		if (SkyNode)
		{
			for ( i = 0; i < 6; ++i )
			{
				SkyNode->getMaterial(i).setTexture ( 0, driver->getTexture ( buf ) );
			}
		}
	}
	else
	if ( 1 == dome )
	{
		SkyNode = smgr->addSkyDomeSceneNode(
				driver->getTexture( buf ), 32,32,
				1.f, 1.f, 1000.f, 0, 11);
	}
	else
	if ( 2 == dome )
	{
		SkyNode = smgr->addSkyDomeSceneNode(
				driver->getTexture( buf ), 16,8,
				0.95f, 2.f, 1000.f, 0, 11);
	}

	
	driver->setTextureCreationFlag(video::ETCF_CREATE_MIP_MAPS, oldMipMapState);
}


// enable GUI elements
void CQuake3EventHandler::SetGUIActive( s32 command)
{
	bool inputState = false;

	ICameraSceneNode * camera = Game->Device->getSceneManager()->getActiveCamera ();

	switch ( command )
	{
		case 0: Game->guiActive = 0; inputState = !Game->guiActive; break;
		case 1: Game->guiActive = 1; inputState = !Game->guiActive;;break;
		case 2: Game->guiActive ^= 1; inputState = !Game->guiActive;break;
		case 3:
			if ( camera )
				inputState = !camera->isInputReceiverEnabled();
			break;
	}

	if ( camera )
	{
		camera->setInputReceiverEnabled ( inputState );
		Game->Device->getCursorControl()->setVisible( !inputState );
	}

	if ( gui.Window )
	{
		gui.Window->setVisible ( Game->guiActive != 0 );
	}
	
	Game->Device->getGUIEnvironment()->setFocus ( Game->guiActive ? gui.Window: 0 );
}


/*
	Handle game input
*/
bool CQuake3EventHandler::OnEvent(const SEvent& eve)
{
	if ( eve.EventType == EET_LOG_TEXT_EVENT )
	{
		return false;
	}

	if ( Game->guiActive && eve.EventType == EET_GUI_EVENT )
	{
		if ( eve.GUIEvent.Caller == gui.MapList && eve.GUIEvent.EventType == gui::EGET_LISTBOX_SELECTED_AGAIN )
		{
			s32 selected = gui.MapList->getSelected();
			if ( selected >= 0 )
			{
				stringw loadMap = gui.MapList->getListItem ( selected );
				if ( 0 == MapParent || loadMap != Game->CurrentMapName )
				{
					LoadMap ( loadMap , 1 );
					mapload=true;
					if ( 0 == Game->loadParam.loadSkyShader )
					{
						AddSky ( 1, "skydome2" );
					}
					CreatePlayers ();
					CreateGUI ();
					SetGUIActive ( 0 );
					return true;
				}
			}
		}
		else
		
		
		if ( eve.GUIEvent.Caller == gui.Gamma && eve.GUIEvent.EventType == gui::EGET_SCROLL_BAR_CHANGED )
		{
			Game->GammaValue = gui.Gamma->getPos () * 0.01f;
			Game->Device->setGammaRamp ( Game->GammaValue, Game->GammaValue, Game->GammaValue, 0.f, 0.f );
		}
		else
		if ( eve.GUIEvent.Caller == gui.SetVideoMode && eve.GUIEvent.EventType == gui::EGET_BUTTON_CLICKED )
		{
			//Game->retVal = 2;
			//Game->Device->closeDevice();

			//CLOSE BUTTON
		}
		else
		if ( eve.GUIEvent.Caller == gui.Window && eve.GUIEvent.EventType == gui::EGET_ELEMENT_CLOSED )
		{
			Game->Device->closeDevice();
		}
		else
	
		return false;
	}

	// fire
	if (
		(eve.EventType == EET_MOUSE_INPUT_EVENT && eve.MouseInput.Event == EMIE_LMOUSE_LEFT_UP)
	   )
	{
		if (mapload==true){
		ICameraSceneNode * camera = Game->Device->getSceneManager()->getActiveCamera ();
		ISoundEngine* shoot=createIrrKlangDevice();
		shoot->play2D("sounds\\shoot.wav");
		if ( camera && camera->isInputReceiverEnabled () )
		{
			useItem( Player + 0 );
		}
		}
	}

	// gui active
	if ((eve.EventType == EET_KEY_INPUT_EVENT && eve.KeyInput.Key == KEY_ESCAPE && eve.KeyInput.PressedDown==false ) )
	{
		SetGUIActive ( 2 );
	}
	if (eve.KeyInput.Key == KEY_KEY_W || eve.KeyInput.Key == KEY_KEY_A ||
		eve.KeyInput.Key == KEY_KEY_S || eve.KeyInput.Key == KEY_KEY_D ||
		eve.KeyInput.Key == KEY_LEFT || eve.KeyInput.Key == KEY_UP ||
		eve.KeyInput.Key == KEY_DOWN || eve.KeyInput.Key == KEY_RIGHT 
	   )
		{
		if  (time-eventtime>270){
		ISoundEngine* walk=createIrrKlangDevice();
		walk->play2D("sounds\\foot.mp3");
		eventtime=time;
		}
		
		if (once==0 && mapload==true){
			//WeaponNode->setFrameLoop(54,70);
			WeaponNode->setMD2Animation ( "walk" );
			once=1;
		}	
		
		//WeaponNode->setFrameLoop(50,51);
		}
	// check if user presses the key
	if ( eve.EventType == EET_KEY_INPUT_EVENT && eve.KeyInput.PressedDown == false)
	{
		if (mapload==true)
		
		{WeaponNode->setMD2Animation ( "stand" );once=0;}
		
		if (eve.KeyInput.Key == KEY_F2)
		{
			Player[0].respawn ();
		}
		if (eve.KeyInput.Key == KEY_F4)
		{   Enemy();
			enemyx+=5;
			enemyy+=5;
			enemyz+=5;
		}
		
	}

	// check if user presses the key C ( for crouch)
	if ( eve.EventType == EET_KEY_INPUT_EVENT && eve.KeyInput.Key == KEY_KEY_C )
	{
		// crouch
		ISceneNodeAnimatorCollisionResponse *anim = Player[0].cam ();
		if ( anim && 0 == Game->flyTroughState )
		{
			if ( false == eve.KeyInput.PressedDown )
			{
				// stand up
				anim->setEllipsoidRadius (  vector3df(30,45,30) );
				anim->setEllipsoidTranslation ( vector3df(0,40,0));

			}
			else
			{
				// on your knees
				anim->setEllipsoidRadius (  vector3df(30,20,30) );
				anim->setEllipsoidTranslation ( vector3df(0,20,0));
			}
			return true;
		}
	}
	return false;
}



/*
	useItem
*/
void CQuake3EventHandler::useItem( Q3Player * player)
{
	ISceneManager* smgr = Game->Device->getSceneManager();
	ICameraSceneNode* camera = smgr->getActiveCamera();

	if (!camera)
		return;

	SParticleImpact imp;
	imp.when = 0;

	// get line of camera

	vector3df start = camera->getPosition();

	if (WeaponNode )
	{
		start.X += 0.f;
		start.Y += 0.f;
		start.Z += 0.f;
	}

	vector3df end = (camera->getTarget() - start);
	end.normalize();
	start += end*20.0f;

	end = start + (end * camera->getFarValue());

	triangle3df triangle;
	line3d<f32> line(start, end);

	// get intersection point with map
	scene::ISceneNode* hitNode;
	if (smgr->getSceneCollisionManager()->getCollisionPoint(
		line, Meta, end, triangle,hitNode))
	{
		// collides with wall
		vector3df out = triangle.getNormal();
		out.setLength(0.03f);
		imp.when = 1;
		imp.outVector = out;
		imp.pos = end;
	}
	// create fire ball
	ISceneNode* node = 0;
	node = smgr->addBillboardSceneNode( BulletParent,dimension2d<f32>(10,10), start);
	node->setMaterialFlag(EMF_LIGHTING, false);
	node->setMaterialTexture(0, Game->Device->getVideoDriver()->getTexture("shalow1.bmp"));
	node->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
	node->setMaterialType(EMT_TRANSPARENT_ADD_COLOR);
	
	f32 length = (f32)(end - start).getLength();
	const f32 speed = 5.8f;
	u32 time = (u32)(length / speed);

	ISceneNodeAnimator* anim = 0;

	// set flight line

	anim = smgr->createFlyStraightAnimator(start, end, time);
	node->addAnimator(anim);
	anim->drop();

	snprintf ( buf, 64, "bullet: %s on %.1f,%1.f,%1.f",
				imp.when ? "hit" : "nohit", end.X, end.Y, end.Z );
	node->setName ( buf );


	anim = smgr->createDeleteAnimator(time);
	node->addAnimator(anim);
	anim->drop();

	if (imp.when)
	{
		imp.when = Game->Device->getTimer()->getTime() +
			(time + (s32) ( ( 1.f + Noiser::get() ) * 250.f ));
		Impacts.push_back(imp);
	}

}

// rendered when bullets hit something
void CQuake3EventHandler::createParticleImpacts( u32 now )
{
	ISceneManager* sm = Game->Device->getSceneManager();
	
	struct smokeLayer
	{
		const c8 * texture;
		f32 scale;
		f32 minparticleSize;
		f32 maxparticleSize;
		f32 boxSize;
		u32 minParticle;
		u32 maxParticle;
		u32 fadeout;
		u32 lifetime;
	};

	smokeLayer smoke[] =
	{
		{ "smoke2.jpg", 0.4f, 1.5f, 18.f, 20.f, 20, 50, 2000, 10000 },
		{ "smoke3.jpg", 0.2f, 1.2f, 15.f, 20.f, 10, 30, 1000, 12000 }
	};


	u32 i;
	u32 g;
	s32 factor = 1;

	for ( i=0; i < Impacts.size(); ++i)
	{
		if (now < Impacts[i].when)
			continue;

		// create smoke particle system
		IParticleSystemSceneNode* pas = 0;

		for ( g = 0; g != 2; ++g )
		{
			pas = sm->addParticleSystemSceneNode(false, BulletParent, -1, Impacts[i].pos);

			snprintf ( buf, 64, "bullet impact smoke at %.1f,%.1f,%1.f",
				Impacts[i].pos.X,Impacts[i].pos.Y,Impacts[i].pos.Z);
			pas->setName ( buf );

			// create a flat smoke
			vector3df direction = Impacts[i].outVector;
			direction *= smoke[g].scale;
			IParticleEmitter* em = pas->createBoxEmitter(
				aabbox3d<f32>(-4.f,0.f,-4.f,20.f,smoke[g].minparticleSize,20.f),
				direction,smoke[g].minParticle, smoke[g].maxParticle,
				video::SColor(0,0,0,0),video::SColor(0,128,128,128),
				250,4000, 60);

			em->setMinStartSize (dimension2d<f32>( smoke[g].minparticleSize, smoke[g].minparticleSize));
			em->setMaxStartSize (dimension2d<f32>( smoke[g].maxparticleSize, smoke[g].maxparticleSize));

			pas->setEmitter(em);
			em->drop();

			// particles get invisible
			IParticleAffector* paf = pas->createFadeOutParticleAffector(
				video::SColor ( 0, 0, 0, 0 ), smoke[g].fadeout);
			pas->addAffector(paf);
			paf->drop();

			// particle system life time
			ISceneNodeAnimator* anim = sm->createDeleteAnimator( smoke[g].lifetime);
			pas->addAnimator(anim);
			anim->drop();

			pas->setMaterialFlag(video::EMF_LIGHTING, false);
			pas->setMaterialFlag(video::EMF_ZWRITE_ENABLE, false);
			pas->setMaterialType(video::EMT_TRANSPARENT_ADD_COLOR );
			pas->setMaterialTexture(0, Game->Device->getVideoDriver()->getTexture( smoke[g].texture ));
		}



		Impacts.erase(i);
		i--;
	}
}

/*
	render
*/
void CQuake3EventHandler::Render()
{
	IVideoDriver * driver = Game->Device->getVideoDriver();
	if ( 0 == driver )
		return;
	{
		driver->beginScene(true, true, SColor(0,0,0,0));
		Game->Device->getSceneManager()->drawAll();
	}
	Game->Device->getGUIEnvironment()->drawAll();
	driver->endScene();
}

/*
	update the generic scene node
*/
void CQuake3EventHandler::Animate()
{
	u32 now = Game->Device->getTimer()->getTime();

	Q3Player * player = Player + 0;
	// Query Scene Manager attributes
	if ( player->Anim[0].flags & FIRED )
	{
		ISceneManager *smgr = Game->Device->getSceneManager ();
		wchar_t msg[128];
		IVideoDriver * driver = Game->Device->getVideoDriver();

		IAttributes * attr = smgr->getParameters();
		swprintf ( msg, 128,
			L"Destructo Beam Start");
		Game->Device->setWindowCaption( msg );
	}

	

	createParticleImpacts ( now );

}


/* The main game states
*/
void runGame ( GameData *game )
{
	ISceneManager *smgr = game->Device->getSceneManager ();
	IVideoDriver * driver = game->Device->getVideoDriver();
	if ( game->retVal >= 9 )
		return;	
	if ( game->Device == NULL)
	{
		// could not create selected driver.
		game->retVal = 0;
		return;
	}
	// create an event receiver based on current game data
	CQuake3EventHandler *eventHandler = new CQuake3EventHandler( game );
	
	// add our media directory and archive to the file system
	for ( u32 i = 0; i < game->CurrentArchiveList.size(); ++i )
	{
		eventHandler->AddArchive ( game->CurrentArchiveList[i] );
	}
	
	// Load a Map or startup to the GUI
	if ( game->CurrentMapName.size () )
	{
		eventHandler->LoadMap ( game->CurrentMapName, 1 );
		if ( 0 == game->loadParam.loadSkyShader )
			eventHandler->AddSky ( 1, "skydome2" );
		eventHandler->CreatePlayers ();
	}
	else
	{eventHandler->AddSky ( 1, "skydome2" );
		eventHandler->CreateGUI ();
		eventHandler->SetGUIActive ( 1 );
	}
	
	game->retVal = 3;
	int aikbaar=0;
	while( game->Device->run() )
	{
		if(mapload==true){
		if (health==100)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\100.png"),core::position2d<s32>(80,519));
		if (health==90)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\90.png"),core::position2d<s32>(80,519));
		if (health==80)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\80.png"),core::position2d<s32>(80,519));
		if (health==70)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\70.png"),core::position2d<s32>(80,519));
		if (health==60)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\60.png"),core::position2d<s32>(80,519));
		if (health==50)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\50.png"),core::position2d<s32>(80,519));
		if (health==40)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\40.png"),core::position2d<s32>(80,519));
		if (health==30)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\30.png"),core::position2d<s32>(80,519));
		if (health==20)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\20.png"),core::position2d<s32>(80,519));
		if (health==10)
		game->Device->getGUIEnvironment()->addImage(driver->getTexture("health\\10.png"),core::position2d<s32>(80,519));
//		Enemy->setPosition(vector3df( enemyx, enemyy, enemyz));
	//	eventHandler->Enemy();
		}
		time = game->Device->getTimer()->getTime();
	eventHandler->Animate ();

	eventHandler->Render ();

	if (aikbaar==0 && mapload==true){
	
	
	aikbaar=1;
	}
	}
	
	game->Device->setGammaRamp ( 1.f, 1.f, 1.f, 0.f, 0.f );
	delete eventHandler;
}

#if defined (_IRR_WINDOWS_) && 0
	#pragma comment(linker, "/subsystem:windows /ENTRY:mainCRTStartup")
#endif
