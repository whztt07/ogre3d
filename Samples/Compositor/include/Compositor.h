/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2013 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

You may use this sample code for anything you like, it is not covered by the
same license as the rest of the engine.
-----------------------------------------------------------------------------
*/

#ifndef _CompositorDemo_H_
#define _CompositorDemo_H_

#include "OgreConfigFile.h"
#include "OgreStringConverter.h"
#include "OgreException.h"

#include "SdkSample.h"
#include "SamplePlugin.h"

#include "Compositor/OgreCompositorNode.h"
#include "Compositor/OgreCompositorNodeDef.h"
#include "Compositor/OgreCompositorWorkspace.h"
#include "Compositor/OgreCompositorWorkspaceDef.h"

using namespace Ogre;
using namespace OgreBites;

#define COMPOSITORS_PER_PAGE 8

class _OgreSampleClassExport Sample_Compositor : public SdkSample
{
public:
	Sample_Compositor();

    void setupContent(void);
    void cleanupContent(void);
    StringVector getRequiredPlugins();

	bool frameRenderingQueued(const FrameEvent& evt);
	
	void checkBoxToggled(OgreBites::CheckBox * box);
	void buttonHit(OgreBites::Button* button);        
	void itemSelected(OgreBites::SelectMenu* menu);

protected:
	virtual void setupCompositor(void);
	
	void setupView(void);
	void setupControls(void);
    void setupScene(void);
    void createEffects(void);
	void createTextures(void);
	void changePage(size_t pageNum);
	
	SceneNode * mSpinny;
	StringVector mCompositorNames;
	size_t mActiveCompositorPage;
	size_t mNumCompositorPages;	

	//Used to unregister compositor logics and free memory
	/*typedef map<String, CompositorLogic*>::type CompositorLogicMap;
	CompositorLogicMap mCompositorLogics;*/

	String mDebugCompositorName;
	SelectMenu* mDebugTextureSelectMenu;
	TextureUnitState* mDebugTextureTUS;

	CompositorWorkspace *mWorkspace;

};

/**
    @file
        Compositor.cpp
    @brief
        Shows OGRE's Compositor feature
	@author
		W.J. :wumpus: van der Laan
			Ogre compositor framework
		Manuel Bua
			Postfilter ideas and original out-of-core implementation
        Jeff (nfz) Doyle
            added gui framework to demo
*/

#include <Ogre.h>

#include "HelperLogics.h"

/*************************************************************************
	                    Sample_Compositor Methods
*************************************************************************/
Sample_Compositor::Sample_Compositor()
{
	mInfo["Title"] = "Compositor";
	mInfo["Description"] = "A demo of Ogre's post-processing framework.";
	mInfo["Thumbnail"] = "thumb_comp.png";
	mInfo["Category"] = "Effects";
}
//--------------------------------------------------------------------------
void Sample_Compositor::setupView()
{
	SdkSample::setupView();
    mCamera->setPosition(Ogre::Vector3(0,0,0));
    mCamera->lookAt(Ogre::Vector3(0,0,-300));
    mCamera->setNearClipDistance(1);
}
//-----------------------------------------------------------------------------------
void Sample_Compositor::setupCompositor(void)
{
	/*Ogre::CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

	const Ogre::IdString workspaceName( "CompositorSampleWorkspace" );
	CompositorWorkspaceDef *workspaceDef = compositorManager->getWorkspaceDefinition( workspaceName );
	workspaceDef->connect( "Bloom", "Glass" );
	workspaceDef->connect( "Glass", 0, "FinalComposition", 1 );*/

	CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

	const Ogre::IdString workspaceName( "CompositorSampleWorkspace" );
	CompositorWorkspaceDef *workspaceDef = compositorManager->getWorkspaceDefinition( workspaceName );

	//Clear the definition made with scripts as example
	workspaceDef->clearAll();

	CompositorManager2::CompositorNodeDefMap nodeDefs = compositorManager->getNodeDefinitions();

	//iterate through Compositor Managers resources and add name keys to menu
	CompositorManager2::CompositorNodeDefMap::const_iterator itor = nodeDefs.begin();
	CompositorManager2::CompositorNodeDefMap::const_iterator end  = nodeDefs.end();

	IdString compositorId = "Ogre/Compositor";

	// Add all compositor resources to the view container
	while( itor != end )
	{
		if( itor->second->mCustomIdentifier == compositorId )
		{
			mCompositorNames.push_back(itor->second->getNameStr());

			//Manually disable the node and add it to the workspace without any connection
			itor->second->setStartEnabled( false );
			workspaceDef->addNodeAlias( itor->first, itor->first );
		}

		++itor;
	}

	workspaceDef->connect( "CompositorSampleStdRenderer", 0, "FinalComposition", 1 );
	workspaceDef->connectOutput( "FinalComposition", 0 );

	mWorkspace = compositorManager->addWorkspace( mSceneMgr, mWindow, mCamera, workspaceName, true );

	mNumCompositorPages = (mCompositorNames.size() / COMPOSITORS_PER_PAGE) +
		((mCompositorNames.size() % COMPOSITORS_PER_PAGE == 0) ? 0 : 1);
}
//-----------------------------------------------------------------------------------
void Sample_Compositor::setupContent(void)
{
	// Register the compositor logics
	// See comment in beginning of HelperLogics.h for explanation
	/*Ogre::CompositorManager& compMgr = Ogre::CompositorManager::getSingleton();
	mCompositorLogics["GaussianBlur"]	= new GaussianBlurLogic;
	mCompositorLogics["HDR"]			= new HDRLogic;
	mCompositorLogics["HeatVision"]		= new HeatVisionLogic;
	compMgr.registerCompositorLogic("GaussianBlur", mCompositorLogics["GaussianBlur"]);
	compMgr.registerCompositorLogic("HDR", mCompositorLogics["HDR"]);
	compMgr.registerCompositorLogic("HeatVision", mCompositorLogics["HeatVision"]);*/
	
#if 0
	createTextures();
#endif
    /// Create a couple of hard coded postfilter effects as an example of how to do it
	/// but the preferred method is to use compositor scripts.
	createEffects();

	setupScene();

	setupControls();

#if OGRE_PLATFORM != OGRE_PLATFORM_APPLE_IOS
	setDragLook(true);
#endif
}
StringVector Sample_Compositor::getRequiredPlugins()
{
    StringVector names;
    if (!GpuProgramManager::getSingleton().isSyntaxSupported("glsles") && !GpuProgramManager::getSingleton().isSyntaxSupported("glsl150"))
        names.push_back("Cg Program Manager");
    return names;
}
//-----------------------------------------------------------------------------------
void Sample_Compositor::changePage(size_t pageNum)
{
	assert(pageNum < mNumCompositorPages);
	
	mActiveCompositorPage = pageNum;

	size_t maxCompositorsInPage = mCompositorNames.size() - (pageNum * COMPOSITORS_PER_PAGE);
	for (size_t i=0; i < COMPOSITORS_PER_PAGE; i++)
	{
		String checkBoxName = "Compositor_" + Ogre::StringConverter::toString(i);
		CheckBox* cb = static_cast<CheckBox*>(mTrayMgr->getWidget(TL_TOPLEFT, checkBoxName));
		if (i < maxCompositorsInPage)
		{
			String compositorName = mCompositorNames[pageNum * COMPOSITORS_PER_PAGE + i];
			CompositorNode *node = mWorkspace->findNode( compositorName );

			cb->setCaption(compositorName);

			if( node ) //Shouldn't be null, but just in case...
			{
				cb->setChecked( node->getEnabled(), false );
				cb->show();
			}
			else
			{
				cb->setChecked( false, false );
				cb->hide();
			}

		}
		else
		{
			cb->hide();
		}
	}

	OgreBites::Button* pageButton = static_cast<OgreBites::Button*>(mTrayMgr->getWidget(TL_TOPLEFT, "PageButton"));
	Ogre::StringStream ss;
	ss << "Compositors " << pageNum + 1 << "/" << mNumCompositorPages;
	pageButton->setCaption(ss.str());
}
//-----------------------------------------------------------------------------------
void Sample_Compositor::cleanupContent(void)
{
	mDebugTextureTUS->setContentType(TextureUnitState::CONTENT_NAMED);
	mCompositorNames.clear();

    TextureManager::getSingleton().remove("DitherTex");
    TextureManager::getSingleton().remove("HalftoneVolume");

#if 0
	Ogre::CompositorManager& compMgr = Ogre::CompositorManager::getSingleton();
	CompositorLogicMap::const_iterator itor = mCompositorLogics.begin();
	CompositorLogicMap::const_iterator end  = mCompositorLogics.end();
	while( itor != end )
	{
		compMgr.unregisterCompositorLogic( itor->first );
		delete itor->second;
		++itor;
	}
	mCompositorLogics.clear();
#endif
    MeshManager::getSingleton().remove("Myplane");
}
//-----------------------------------------------------------------------------------
void Sample_Compositor::setupControls(void) 
{
	mTrayMgr->createButton(TL_TOPLEFT, "PageButton", "Compositors", 175);

	for (size_t i=0; i < COMPOSITORS_PER_PAGE; i++)
	{
		String checkBoxName = "Compositor_" + Ogre::StringConverter::toString(i);
		CheckBox* cb = mTrayMgr->createCheckBox(TL_TOPLEFT, checkBoxName, "Compositor", 175);
		cb->hide();
	}

	changePage(0);
	
	mDebugTextureSelectMenu = mTrayMgr->createThickSelectMenu(TL_TOPRIGHT, "DebugRTTSelectMenu", "Debug RTT", 180, 5);
	mDebugTextureSelectMenu->addItem("None");

	mTrayMgr->createSeparator(TL_TOPRIGHT, "DebugRTTSep1");  // this is a hack to give the debug RTT a bit more room

	DecorWidget* debugRTTPanel = mTrayMgr->createDecorWidget(TL_NONE, "DebugRTTPanel", "SdkTrays/Picture");
	OverlayContainer* debugRTTContainer = (OverlayContainer*)debugRTTPanel->getOverlayElement();
	mDebugTextureTUS = debugRTTContainer->getMaterial()->getBestTechnique()->getPass(0)->getTextureUnitState(0);
	//mDebugTextureTUS->setTextureName("CompositorDemo/DebugView");
	debugRTTContainer->setDimensions(128, 128);
	debugRTTContainer->getChild("DebugRTTPanel/PictureFrame")->setDimensions(144, 144);
	debugRTTPanel->hide();

	mTrayMgr->createSeparator(TL_TOPRIGHT, "DebugRTTSep2");  // this is a hack to give the debug RTT a bit more room

	mTrayMgr->showCursor();
	mTrayMgr->showLogo(TL_BOTTOMLEFT);
	mTrayMgr->toggleAdvancedFrameStats();
}
//-----------------------------------------------------------------------------------
void Sample_Compositor::checkBoxToggled(OgreBites::CheckBox * box)
{
	if (Ogre::StringUtil::startsWith(box->getName(), "Compositor_", false))
	{
		String nodeName = box->getCaption();

		String activeTex = mDebugTextureSelectMenu->getSelectedItem();

		if (!box->isChecked())
		{
			//Remove the items from the debug menu and remove debug texture if from disabled compositor
			bool debuggingRemovedTex = StringUtil::startsWith(activeTex, nodeName, false);
			if (debuggingRemovedTex)
			{
				mDebugTextureTUS->setContentType(TextureUnitState::CONTENT_NAMED);
				mDebugTextureSelectMenu->selectItem(0, true);
			}
			for (unsigned int i = 1; i < mDebugTextureSelectMenu->getNumItems(); i++)
			{
				if (StringUtil::startsWith(mDebugTextureSelectMenu->getItems()[i], nodeName, false))
				{
					mDebugTextureSelectMenu->removeItem(i);
					i--;
				}
			}
			if (!debuggingRemovedTex)
			{
				//Selection clears itself when removing items. Restore.
				mDebugTextureSelectMenu->selectItem(activeTex, false);
			}
		}

#if 0
		CompositorManager::getSingleton().setCompositorEnabled(mViewport, nodeName, box->isChecked());
#endif
		const Ogre::IdString workspaceName( "CompositorSampleWorkspace" );
		CompositorManager2 *compositorManager = mRoot->getCompositorManager2();

		//Disable/Enable the node (it's already instantiated in setupCompositor())
		CompositorNode *node = mWorkspace->findNode( nodeName );
		node->setEnabled( box->isChecked() );

		//The workspace instance can't return a non-const version of its definition.
		CompositorWorkspaceDef *workspaceDef = compositorManager->getWorkspaceDefinition( workspaceName );

		//----------------------------------------------------------------------------------------------
		//
		//	METHOD 1 (the masochist way, 'cos it's hard, prone to bugs, but very flexible):
		//		When enabling: Interleave the node between the 1st and 2nd node.
		//		When disabling: Manually disconnect the node in the middle, then fix broken connections.
		//		Normally this method is not recommended, but if you're looking to make a GUI node
		//		editor, the knowledge from this code is very useful.
		//
		//----------------------------------------------------------------------------------------------
		/*CompositorWorkspaceDef::ChannelRouteList &channelRouteList = workspaceDef->_getChannelRoutes();
		if( box->isChecked() )
		{
			//Enabling
			if( channelRouteList.size() == 1 )
			{
				//No compositor node yet activated
				channelRouteList.pop_back();
				workspaceDef->connect( "CompositorSampleStdRenderer", node->getName() );
				workspaceDef->connect( node->getName(), 0, "FinalComposition", 1 );
			}
			else
			{
				//There is at least one compositor active already, interleave

				//We always define in advance "CompositorSampleStdRenderer" to be the first one,
				//otherwise we would have to look for those entries with
				//channelRouteList[n].outNode == CompositorSampleStdRenderer
				CompositorWorkspaceDef::ChannelRouteList::iterator it = channelRouteList.begin();
				CompositorWorkspaceDef::ChannelRoute &rendererChannel0 = *it++;
				CompositorWorkspaceDef::ChannelRoute &rendererChannel1 = *it;

				IdString old2ndNode = rendererChannel0.inNode; //Will now become the 3rd node

				rendererChannel0.inNode		= node->getName();
				//rendererChannel0.inChannel= 0 //Channel stays the same
				rendererChannel1.inNode		= node->getName();
				//rendererChannel1.inChannel= 1 //Channel stays the same

				workspaceDef->connect( node->getName(), old2ndNode );
			}
		}
		else
		{
			//Disabling
			if( channelRouteList.size() == 3 )
			{
				//After disabling us, there will be no more compositors active
				channelRouteList.clear();
				workspaceDef->connect( "CompositorSampleStdRenderer", 0, "FinalComposition", 1 );
			}
			else
			{
				//Find our channel route
				CompositorWorkspaceDef::ChannelRouteList::iterator it = channelRouteList.begin();
				CompositorWorkspaceDef::ChannelRouteList::iterator en = channelRouteList.end();

				IdString currentNode = node->getName();
				IdString prevNode; //We assume all inputs are coming from the same node
				IdString nextNode; //We assume our node doesn't output to more than one node simultaneously

				while( it != en )
				{
					if( it->inNode == currentNode )
					{
						prevNode = it->outNode;
						it = channelRouteList.erase( it );
					}
					else if( it->outNode == currentNode )
					{
						nextNode = it->inNode;
						it = channelRouteList.erase( it );
					}
					else
					{
						++it;
					}
				}

				if( nextNode == "FinalComposition" )
					workspaceDef->connect( prevNode, 0, nextNode, 1 );
				else
					workspaceDef->connect( prevNode, nextNode );
			}
		}*/

		//----------------------------------------------------------------------------------------------
		//
		//	METHOD 2 (the easy way):
		//		Reconstruct the whole connection from scratch based on a copy (be it a cloned, untouched
		//		workspace definition, a custom file, or the very own workspace instance) but leaving the
		//		node we're disabling unplugged.
		//		This method is much safer and easier, the **recommended** way for most usage scenarios
		//		involving toggling compositors on and off frequently. With a few tweaks, it can easily
		//		be adapted to complex compositors too.
		//
		//----------------------------------------------------------------------------------------------
		workspaceDef->clearAllInterNodeConnections();

		IdString finalCompositionId = "FinalComposition";
		const CompositorNodeVec &nodes = mWorkspace->getNodeSequence();

		IdString lastInNode;
		CompositorNodeVec::const_iterator it = nodes.begin();
		CompositorNodeVec::const_iterator en = nodes.end();

		while( it != en )
		{
			CompositorNode *outNode = *it;

			if( outNode->getEnabled() && outNode->getName() != finalCompositionId )
			{
				//Look for the next enabled node we can connect to
				CompositorNodeVec::const_iterator it2 = it + 1;
				while( it2 != en && (!(*it2)->getEnabled() || (*it2)->getName() == finalCompositionId) )
					++it2;

				if( it2 != en )
				{
					lastInNode = (*it2)->getName();
					workspaceDef->connect( outNode->getName(), lastInNode );
				}

				it = it2 - 1;
			}

			++it;
		}

		if( lastInNode == IdString() )
			lastInNode = "CompositorSampleStdRenderer";

		workspaceDef->connect( lastInNode, 0, "FinalComposition", 1 );

		//Not needed unless we'd called workspaceDef->clearOutputConnections
		//workspaceDef->connectOutput( "FinalComposition", 0 );

		//Now that we're done, tell the instance to update itself.
		mWorkspace->reconnectAllNodes();
		
		if (box->isChecked())
		{
#if 0
			//Add the items to the selectable texture menu
			CompositorInstance* instance = CompositorManager::getSingleton().getCompositorChain(mViewport)->getCompositor(compositorName);
			if (instance)
			{
				CompositionTechnique::TextureDefinitionIterator it = instance->getTechnique()->getTextureDefinitionIterator();
				while (it.hasMoreElements())
				{
					CompositionTechnique::TextureDefinition* texDef = it.getNext();
					size_t numTextures = texDef->formatList.size();
					if (numTextures > 1)
					{
						for (size_t i=0; i<numTextures; i++)
						{
							//Dirty string composition. NOT ROBUST!
							mDebugTextureSelectMenu->addItem(compositorName + ";" + texDef->name + ";" + 
								Ogre::StringConverter::toString((Ogre::uint32)i));
						}
					}
					else
					{
						mDebugTextureSelectMenu->addItem(compositorName + ";" + texDef->name);
					}
				}
				mDebugTextureSelectMenu->selectItem(activeTex, false);
			}
#endif
		}
	}
}
//-----------------------------------------------------------------------------------
void Sample_Compositor::buttonHit(OgreBites::Button* button)
{
	size_t nextPage = (mActiveCompositorPage + 1) % mNumCompositorPages;
	changePage(nextPage);
}
//-----------------------------------------------------------------------------------
void Sample_Compositor::itemSelected(OgreBites::SelectMenu* menu)
{
	if (menu->getSelectionIndex() == 0)
	{
		mDebugTextureTUS->setContentType(TextureUnitState::CONTENT_NAMED);
		mTrayMgr->getWidget("DebugRTTPanel")->hide();
		mTrayMgr->removeWidgetFromTray("DebugRTTPanel");
		return;
	}

	mTrayMgr->getWidget("DebugRTTPanel")->show();
	mTrayMgr->moveWidgetToTray("DebugRTTPanel", TL_TOPRIGHT, static_cast<unsigned int>(mTrayMgr->getNumWidgets(TL_TOPRIGHT) - 1));
	StringVector parts = StringUtil::split(menu->getSelectedItem(), ";");
	mDebugTextureTUS->setContentType(TextureUnitState::CONTENT_COMPOSITOR);

	if (parts.size() == 2)
	{
		mDebugTextureTUS->setCompositorReference(parts[0], parts[1]);
	}
	else
	{
		mDebugTextureTUS->setCompositorReference(parts[0], parts[1], 
			StringConverter::parseUnsignedInt(parts[2]));
	}
}
//-----------------------------------------------------------------------------------
void Sample_Compositor::setupScene(void)
{
	Ogre::MovableObject::setDefaultVisibilityFlags(0x00000001);

	// Set ambient light
	mSceneMgr->setAmbientLight(Ogre::ColourValue(0.3, 0.3, 0.2));

	Ogre::Light* l = mSceneMgr->createLight();
	mSceneMgr->createSceneNode()->attachObject( l );
	Ogre::Vector3 dir(-1,-1,0);
	dir.normalise();
	l->setType(Ogre::Light::LT_DIRECTIONAL);
	l->setDirection(dir);
	l->setDiffuseColour(1, 1, 0.8);
	l->setSpecularColour(1, 1, 1);


	Ogre::Entity* pEnt;

	// House
	pEnt = mSceneMgr->createEntity( "tudorhouse.mesh", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );
	Ogre::SceneNode* n1 = mSceneMgr->getRootSceneNode()->createChildSceneNode(SCENE_STATIC, Ogre::Vector3(350, 450, -200));
	n1->attachObject( pEnt );

	pEnt = mSceneMgr->createEntity( "tudorhouse.mesh", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME, SCENE_STATIC );
	Ogre::SceneNode* n2 = mSceneMgr->getRootSceneNode()->createChildSceneNode(SCENE_STATIC, Ogre::Vector3(-350, 450, -200));
	n2->attachObject( pEnt );

	pEnt = mSceneMgr->createEntity( "knot.mesh", ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME );
	mSpinny = mSceneMgr->getRootSceneNode()->createChildSceneNode(SCENE_DYNAMIC, Ogre::Vector3(0, 0, 300));
	mSpinny->attachObject( pEnt );
	pEnt->setMaterialName("Examples/MorningCubeMap");

	mSceneMgr->setSkyBox(true, "Examples/MorningSkyBox");


	Ogre::Plane plane;
	plane.normal = Ogre::Vector3::UNIT_Y;
	plane.d = 100;
	MeshPtr planMesh = Ogre::MeshManager::getSingleton().createPlane("Myplane",
		Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, plane,
		1500, 1500, 10, 10, true, 1, 5, 5, Ogre::Vector3::UNIT_Z);
	Ogre::Entity* pPlaneEnt = mSceneMgr->createEntity( planMesh, SCENE_STATIC );
	pPlaneEnt->setMaterialName("Examples/Rockwall");
	pPlaneEnt->setCastShadows(false);
	mSceneMgr->getRootSceneNode()->createChildSceneNode( SCENE_STATIC )->attachObject(pPlaneEnt);

	mCamera->setPosition(-400, 50, 900);
	mCamera->lookAt(0,80,0);
}
//-----------------------------------------------------------------------------------
bool Sample_Compositor::frameRenderingQueued(const FrameEvent& evt)
{
	mSpinny->yaw(Ogre::Degree(10 * evt.timeSinceLastFrame));
	return SdkSample::frameRenderingQueued(evt);
}
//-----------------------------------------------------------------------------------
/// Create the hard coded postfilter effects
void Sample_Compositor::createEffects(void)
{
	    // Bloom compositor is loaded from script but here is the hard coded equivalent
//		CompositorPtr comp = CompositorManager::getSingleton().create(
//				"Bloom", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
//			);
//		{
//			CompositionTechnique *t = comp->createTechnique();
//			{
//				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt0");
//				def->width = 128;
//				def->height = 128;
//				def->format = PF_A8R8G8B8;
//			}
//			{
//				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt1");
//				def->width = 128;
//				def->height = 128;
//				def->format = PF_A8R8G8B8;
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
//				tp->setOutputName("rt1");
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_NONE);
//				tp->setOutputName("rt0");
//				CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/Blur0");
//				pass->setInput(0, "rt1");
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_NONE);
//				tp->setOutputName("rt1");
//				CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/Blur1");
//				pass->setInput(0, "rt0");
//			}
//			{
//				CompositionTargetPass *tp = t->getOutputTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
//				{ CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/BloomBlend");
//				pass->setInput(0, "rt1");
//				}
//			}
//		}
	    // Glass compositor is loaded from script but here is the hard coded equivalent
		/// Glass effect
//		CompositorPtr comp2 = CompositorManager::getSingleton().create(
//				"Glass", ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
//			);
//		{
//			CompositionTechnique *t = comp2->createTechnique();
//			{
//				CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("rt0");
//				def->width = 0;
//				def->height = 0;
//				def->format = PF_R8G8B8;
//			}
//			{
//				CompositionTargetPass *tp = t->createTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_PREVIOUS);
//				tp->setOutputName("rt0");
//			}
//			{
//				CompositionTargetPass *tp = t->getOutputTargetPass();
//				tp->setInputMode(CompositionTargetPass::IM_NONE);
//				{ CompositionPass *pass = tp->createPass();
//				pass->setType(CompositionPass::PT_RENDERQUAD);
//				pass->setMaterialName("Ogre/Compositor/GlassPass");
//				pass->setInput(0, "rt0");
//				}
//			}
//		}
/*		/// Motion blur effect
	Ogre::CompositorPtr comp3 = Ogre::CompositorManager::getSingleton().create(
			"Motion Blur", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
		);
	{
		Ogre::CompositionTechnique *t = comp3->createTechnique();
		{
			Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
			def->width = 0;
			def->height = 0;
			def->formatList.push_back(Ogre::PF_R8G8B8);
		}
		{
			Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("sum");
			def->width = 0;
			def->height = 0;
			def->formatList.push_back(Ogre::PF_R8G8B8);
		}
		{
			Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
			def->width = 0;
			def->height = 0;
			def->formatList.push_back(Ogre::PF_R8G8B8);
		}
		/// Render scene
		{
			Ogre::CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
			tp->setOutputName("scene");
		}
		/// Initialisation pass for sum texture
		{
			Ogre::CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
			tp->setOutputName("sum");
			tp->setOnlyInitial(true);
		}
		/// Do the motion blur
		{
			Ogre::CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
			tp->setOutputName("temp");
			{ Ogre::CompositionPass *pass = tp->createPass();
			pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("Ogre/Compositor/Combine");
			pass->setInput(0, "scene");
			pass->setInput(1, "sum");
			}
		}
		/// Copy back sum texture
		{
			Ogre::CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
			tp->setOutputName("sum");
			{ Ogre::CompositionPass *pass = tp->createPass();
			pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("Ogre/Compositor/Copyback");
			pass->setInput(0, "temp");
			}
		}
		/// Display result
		{
			Ogre::CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
			{ Ogre::CompositionPass *pass = tp->createPass();
			pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
			pass->setMaterialName("Ogre/Compositor/MotionBlur");
			pass->setInput(0, "sum");
			}
		}
	}
	/// Heat vision effect
	Ogre::CompositorPtr comp4 = Ogre::CompositorManager::getSingleton().create(
			"Heat Vision", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME
		);
	{
		Ogre::CompositionTechnique *t = comp4->createTechnique();
		t->setCompositorLogicName("HeatVision");
		{
			Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("scene");
			def->width = 256;
			def->height = 256;
			def->formatList.push_back(Ogre::PF_R8G8B8);
		}
		{
			Ogre::CompositionTechnique::TextureDefinition *def = t->createTextureDefinition("temp");
			def->width = 256;
			def->height = 256;
			def->formatList.push_back(Ogre::PF_R8G8B8);
		}
		/// Render scene
		{
			Ogre::CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(Ogre::CompositionTargetPass::IM_PREVIOUS);
			tp->setOutputName("scene");
		}
		/// Light to heat pass
		{
			Ogre::CompositionTargetPass *tp = t->createTargetPass();
			tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
			tp->setOutputName("temp");
			{
				Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setIdentifier(0xDEADBABE); /// Identify pass for use in listener
				pass->setMaterialName("Fury/HeatVision/LightToHeat");
				pass->setInput(0, "scene");
			}
		}
		/// Display result
		{
			Ogre::CompositionTargetPass *tp = t->getOutputTargetPass();
			tp->setInputMode(Ogre::CompositionTargetPass::IM_NONE);
			{
				Ogre::CompositionPass *pass = tp->createPass();
				pass->setType(Ogre::CompositionPass::PT_RENDERQUAD);
				pass->setMaterialName("Fury/HeatVision/Blur");
				pass->setInput(0, "temp");
			}
		}
	}*/
}
//--------------------------------------------------------------------------
void Sample_Compositor::createTextures(void)
{
	using namespace Ogre;

	TexturePtr tex = TextureManager::getSingleton().createManual(
		"HalftoneVolume",
		"General",
		TEX_TYPE_3D,
		64,64,64,
		0,
		PF_L8, 
		TU_DYNAMIC_WRITE_ONLY
	);

    if(!tex.isNull())
    {
        HardwarePixelBufferSharedPtr ptr = tex->getBuffer(0,0);
        ptr->lock(HardwareBuffer::HBL_DISCARD);
        const PixelBox &pb = ptr->getCurrentLock();
        Ogre::uint8 *data = static_cast<Ogre::uint8*>(pb.data);

        size_t height = pb.getHeight();
        size_t width = pb.getWidth();
        size_t depth = pb.getDepth();
        size_t rowPitch = pb.rowPitch;
        size_t slicePitch = pb.slicePitch;

        for (size_t z = 0; z < depth; ++z)
        {
            for (size_t y = 0; y < height; ++y)
            {
                for(size_t x = 0; x < width; ++x)
                {
                    float fx = 32-(float)x+0.5f;
                    float fy = 32-(float)y+0.5f;
                    float fz = 32-((float)z)/3+0.5f;
                    float distanceSquare = fx*fx+fy*fy+fz*fz;
                    data[slicePitch*z + rowPitch*y + x] =  0x00;
                    if (distanceSquare < 1024.0f)
                        data[slicePitch*z + rowPitch*y + x] +=  0xFF;
                }
            }
        }
        ptr->unlock();
    }
	Ogre::Viewport *vp = mWindow->getViewport(0); 

	TexturePtr tex2 = TextureManager::getSingleton().createManual(
		"DitherTex",
		"General",
		TEX_TYPE_2D,
		vp->getActualWidth(),vp->getActualHeight(),1,
		0,
		PF_L8,
        TU_DYNAMIC_WRITE_ONLY
	);

	HardwarePixelBufferSharedPtr ptr2 = tex2->getBuffer(0,0);
	ptr2->lock(HardwareBuffer::HBL_DISCARD);
	const PixelBox &pb2 = ptr2->getCurrentLock();
	Ogre::uint8 *data2 = static_cast<Ogre::uint8*>(pb2.data);
	
	size_t height2 = pb2.getHeight();
	size_t width2 = pb2.getWidth();
	size_t rowPitch2 = pb2.rowPitch;

	for (size_t y = 0; y < height2; ++y)
	{
		for(size_t x = 0; x < width2; ++x)
		{
			data2[rowPitch2*y + x] = Ogre::Math::RangeRandom(64.0,192);
		}
	}
	
	ptr2->unlock();
}

#endif	// end _CompositorDemo_H_
