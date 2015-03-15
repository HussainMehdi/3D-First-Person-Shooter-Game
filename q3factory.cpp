#include <irrlicht.h>
#include "q3factory.h"
#include "sound.h"

using namespace irr;
using namespace scene;
using namespace gui;
using namespace video;
using namespace core;
using namespace quake3;


/*!
	Takes the mesh buffers and creates scenenodes for their associated shaders
*/
void Q3ShaderFactory (	Q3LevelLoadParameter &loadParam,
						IrrlichtDevice *device,
						IQ3LevelMesh* mesh,
						eQ3MeshIndex meshIndex,
						ISceneNode *parent,
						IMetaTriangleSelector *meta,
						bool showShaderName )
{
	if ( 0 == mesh || 0 == device )
		return;

	IMeshSceneNode* node = 0;
	ISceneManager* smgr = device->getSceneManager();
	ITriangleSelector * selector = 0;

	// the additional mesh can be quite huge and is unoptimized
	// Save to cast to SMesh
	SMesh * additional_mesh = (SMesh*) mesh->getMesh ( meshIndex );
	if ( 0 == additional_mesh || additional_mesh->getMeshBufferCount() == 0)
		return;

	char buf[128];
	
	IGUIFont *font = 0;
	if ( showShaderName )
		font = device->getGUIEnvironment()->getFont("Fonts\\fontlucida.png");
	IVideoDriver *driver = device->getVideoDriver();
	s32 sceneNodeID = 0;
	for ( u32 i = 0; i!= additional_mesh->getMeshBufferCount (); ++i )
	{
		IMeshBuffer *meshBuffer = additional_mesh->getMeshBuffer ( i );
		const SMaterial &material = meshBuffer->getMaterial();

		//! The ShaderIndex is stored in the second material parameter
		s32 shaderIndex = (s32) material.MaterialTypeParam2;

		// the meshbuffer can be rendered without additional support, or it has no shader
		IShader *shader = (IShader *) mesh->getShader ( shaderIndex );

			// create sceneNode
			node = smgr->addQuake3SceneNode ( meshBuffer, shader, parent, sceneNodeID );
			node->setAutomaticCulling ( scene::EAC_FRUSTUM_BOX );
			sceneNodeID += 1;
	}


}


/*!
	create Items from Entity
*/
void Q3ModelFactory (	Q3LevelLoadParameter &loadParam,
						IrrlichtDevice *device,
						IQ3LevelMesh* masterMesh,
						ISceneNode *parent,
						bool showShaderName
						)
{
	if ( 0 == masterMesh )
		return;

	tQ3EntityList &entity = masterMesh->getEntityList ();
	ISceneManager* smgr = device->getSceneManager();


	const SVarGroup *group;
	IEntity search;
	s32 index;
	s32 lastIndex;
	IAnimatedMeshMD3* model;
	SMD3Mesh * mesh;
	const SMD3MeshBuffer *meshBuffer;
	IMeshSceneNode* node;
	ISceneNodeAnimator* anim;
	const IShader *shader;
	u32 pos;
	vector3df p;
	u32 nodeCount = 0;
	tTexArray textureArray;

	IGUIFont *font = 0;
	if ( showShaderName )
		font = device->getGUIEnvironment()->getFont("fontlucida.png");

}

/*!
	so we need a good starting Position in the level.
	we can ask the Quake3 Loader for all entities with class_name "info_player_deathmatch"
*/
s32 Q3StartPosition (	IQ3LevelMesh* mesh,
						ICameraSceneNode* camera,
						s32 startposIndex,
						const vector3df &translation
					)
{
	if ( 0 == mesh )
		return 0;

	tQ3EntityList &entityList = mesh->getEntityList ();

	IEntity search;
	search.name = "info_player_start";	// "info_player_deathmatch";

	// find all entities in the multi-list
	s32 lastIndex;
	s32 index = entityList.binary_search_multi ( search, lastIndex );

	if ( index < 0 )
	{
		search.name = "info_player_deathmatch";
		index = entityList.binary_search_multi ( search, lastIndex );
	}

	if ( index < 0 )
		return 0;

	index += core::clamp ( startposIndex, 0, lastIndex - index );

	u32 parsepos;

	const SVarGroup *group;
	group = entityList[ index ].getGroup(1);

	parsepos = 0;
	vector3df pos = getAsVector3df ( group->get ( "origin" ), parsepos );
	pos += translation;

	parsepos = 0;
	f32 angle = getAsFloat ( group->get ( "angle"), parsepos );

	vector3df target ( 0.f, 0.f, 1.f );
	target.rotateXZBy ( angle - 90.f, vector3df () );

	if ( camera )
	{
		camera->setPosition ( pos );
		camera->setTarget ( pos + target );
		//FPSCamera and animators catches reset on animate 0
		camera->OnAnimate ( 0 );
	}
	return lastIndex - index + 1;
}


/*!
	gets a accumulated force on a given surface
*/
vector3df getGravity ( const c8 * surface )
{
	if ( 0 == strcmp ( surface, "earth" ) ) return vector3df ( 0.f, -60.f, 0.f );
	if ( 0 == strcmp ( surface, "moon" ) ) return vector3df ( 0.f, -6.f / 100.f, 0.f );
	if ( 0 == strcmp ( surface, "water" ) ) return vector3df ( 0.1f / 100.f, -2.f / 100.f, 0.f );
	if ( 0 == strcmp ( surface, "ice" ) ) return vector3df ( 0.2f / 100.f, -9.f / 100.f, 0.3f / 100.f );

	return vector3df ( 0.f, 0.f, 0.f );
}



/*
	Dynamically load the Irrlicht Library
*/

#if defined(_IRR_WINDOWS_API_)
#ifdef _MSC_VER
#pragma comment(lib, "Irrlicht.lib")
#endif

#include <windows.h>

funcptr_createDevice load_createDevice ( const c8 * filename)
{
	return (funcptr_createDevice) GetProcAddress ( LoadLibrary ( filename ), "createDevice" );
}

funcptr_createDeviceEx load_createDeviceEx ( const c8 * filename)
{
	return (funcptr_createDeviceEx) GetProcAddress ( LoadLibrary ( filename ), "createDeviceEx" );
}


#endif

/*
	get the current collision response camera animator
*/
ISceneNodeAnimatorCollisionResponse* camCollisionResponse( IrrlichtDevice * device )
{
	ICameraSceneNode *camera = device->getSceneManager()->getActiveCamera();
	ISceneNodeAnimatorCollisionResponse *a = 0;

	list<ISceneNodeAnimator*>::ConstIterator it = camera->getAnimators().begin();
	for (; it != camera->getAnimators().end(); ++it)
	{
		a = (ISceneNodeAnimatorCollisionResponse*) (*it);
		if ( a->getType() == ESNAT_COLLISION_RESPONSE )
			return a;
	}

	return 0;
}



// Script from irrlicht.sourceforge.net
//! internal Animation
void setTimeFire ( TimeFire *t, u32 delta, u32 flags )
{
	t->flags = flags;
	t->next = 0;
	t->delta = delta;
}


void checkTimeFire ( TimeFire *t, u32 listSize, u32 now )
{
	u32 i;
	for ( i = 0; i < listSize; ++i )
	{
		if ( now < t[i].next )
			continue;

		t[i].next = core::max_ ( now + t[i].delta, t[i].next + t[i].delta );
		t[i].flags |= FIRED;
	}
}
// Script ends