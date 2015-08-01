/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *	  $Id$
 *
 *****************************************************************************/

#include "config.h"

#include "GamePlayState.h"
#include "GameStateManager.h"

#include "GameLoadState.h"
#include "logger.h"
#include "integers.h"

#include "DarkCamera.h"

#include "TextureAtlas.h"

#include <OgreRenderWindow.h>
#include <OgreOverlayElement.h>
#include <OgreStringConverter.h>

using namespace Ogre;

namespace Opde {

	template<> GamePlayState* Singleton<GamePlayState>::ms_Singleton = 0;

	GamePlayState::GamePlayState() : mSceneMgr(NULL), mToLoadScreen(true), mDebugOverlay(NULL), mPortalOverlay(NULL) {
		/// Register as a command listener, so we can load different levels
		Opde::ConsoleBackend::getSingleton().registerCommandListener("load", dynamic_cast<ConsoleCommandListener*>(this));
		Opde::ConsoleBackend::getSingleton().setCommandHint("load", "Loads a specified mission file");
		Opde::ConsoleBackend::getSingleton().registerCommandListener("fps", dynamic_cast<ConsoleCommandListener*>(this));
		Opde::ConsoleBackend::getSingleton().setCommandHint("fps", "Dump FPS stats");

		mRotateSpeed = 500;
		mMoveSpeed = 25;
		mRotateYFactor = 1;

		mShadows = false;
		mSceneDisplay = false;

		// Try to remap the parameters with those listed in the configuration
		mConfigService = GET_SERVICE(ConfigService);

		if (mConfigService->hasParam("move_speed"))
			mMoveSpeed = mConfigService->getParam("move_speed").toFloat();

		if (mConfigService->hasParam("mouse_speed"))
			mRotateSpeed = mConfigService->getParam("mouse_speed").toFloat();

		if (mConfigService->hasParam("mouse_invert"))
			mRotateYFactor = mConfigService->getParam("mouse_invert").toFloat();

		mTranslateVector = Vector3::ZERO;
		mRotX = 0;
		mRotY = 0;

		mForward = false;
		mBackward = false;
		mLeft = false;
		mRight = false;

		mScreenShot = false;
		mSceneDisplay = false;
		mPortalDisplay = false;
		mDebug = false;

		mSceneDetailIndex = 0;
		mNumScreenShots = 0;

		mRoot = Ogre::Root::getSingletonPtr();
		mOverlayMgr = OverlayManager::getSingletonPtr();

		/*mConsole = new ConsoleFrontend();
		mConsole->setActive(false);*/

		mDebugOverlay = OverlayManager::getSingleton().getByName("Opde/DebugOverlay");

		// Portal stats overlay
		mPortalOverlay = OverlayManager::getSingleton().getByName("Opde/OpdeDebugOverlay");

		mShadows = true;

		StartingPointObjID = 0;
	}

	GamePlayState::~GamePlayState() {
		// delete mConsole;
	}

	void GamePlayState::start() {
		LOG_INFO("GamePlayState: Starting");
		PropertyServicePtr ps = GET_SERVICE(PropertyService);
		Property* posPG = ps->getProperty("Position");

		InputServicePtr inputSrv = GET_SERVICE(InputService);
		inputSrv->processCommand("bind ` show_console");

		if (posPG == NULL)
			OPDE_EXCEPT("Could not get Position property. Not defined. Fatal", "GamePlayState::start");

		LOG_DEBUG("Starting Point object id : %d", StartingPointObjID);

		DVariant spoint;
		posPG->get(StartingPointObjID, "position", spoint);

		// Medsci1.mis position with some damn bad performance under GL
		//Vector3 StartingPoint(-8.41809, -163.39, 1.3465);
		Vector3 StartingPoint(0,0,0);

		if (spoint.type() == DVariant::DV_VECTOR)
			StartingPoint = spoint.toVector();

		LOG_DEBUG("Starting Point position : %f %f %f", StartingPoint.x, StartingPoint.y, StartingPoint.z);

		mSceneMgr = mRoot->getSceneManager( "DarkSceneManager" );
		RenderServicePtr renderSrv = GET_SERVICE(RenderService);

		mCamera = renderSrv->getDefaultCamera();
		mViewport = renderSrv->getDefaultViewport();
		mWindow = renderSrv->getRenderWindow();

		mSceneMgr->clearSpecialCaseRenderQueues();
		mSceneMgr->setSpecialCaseRenderQueueMode(SceneManager::SCRQM_EXCLUDE);

		mCamera->setNearClipDistance(0.5);
		mCamera->setFarClipDistance(4000);

		// Also change position, and set Quake-type orientation
		ViewPoint vp = mSceneMgr->getSuggestedViewpoint(true);
		mCamera->setPosition(vp.position);

		if (StartingPointObjID != 0)
			mCamera->setPosition(StartingPoint);

		mCamera->pitch(Degree(90));
		mCamera->rotate(vp.orientation);

		// Don't yaw along variable axis, causes leaning
		mCamera->setFixedYawAxis(true, Vector3::UNIT_Z);

		// Medsci1.mis Direction with some damn bad performance (in combination with the pos above)
		// mCamera->setDirection(-0.398078, 0.825408, -0.400297);

		// Thiefy FOV
		mCamera->setFOVy(Degree(60)); //  * mCamera->getAspectRatio()

		if (mConfigService->hasParam("debug")) {
			if (mConfigService->getParam("debug") == true) {
				// debug overlay
				mDebugOverlay->show();

				// Portal stats overlay
				mPortalOverlay->show();

				mDebug = true;
			}
		}

		// hidden as default
		// mConsole->setActive(false);

		mWindow->resetStatistics();

		mToLoadScreen = false;
	/*
		mDrawService = GET_SERVICE(DrawService);

		mCamera->setPosition(0,10,-15);
		mCamera->lookAt(0,0,30);
		mCamera->setNearClipDistance(0.5f);
		mSceneMgr->setAmbientLight(ColourValue(1, 1, 1));

		TextureAtlas* ta = mDrawService->createAtlas();
		TextureAtlas* ta1 = mDrawService->createAtlas();

		DrawSource* ds = ta->createDrawSource("access.pcx", "General");
		// testing what happens when a font is loaded (e.g. where it segv's)
		mDrawService->setFontPalette(Ogre::ManualFonFileLoader::ePT_PCX, "fontpal.pcx", "General");
		FontDrawSource* fds = mDrawService->loadFont(ta, "fonts/MAINFONT.FON" , "General");
		FontDrawSource* fds1 = mDrawService->loadFont(ta, "fonts/MAINAA.FON" , "General");
		FontDrawSource* fds2 = mDrawService->loadFont(ta, "fonts/keyfonta.FON" , "General");

		ta->build();
		ta1->build();

		RenderedLabel* rl = mDrawService->createRenderedLabel(fds, "OpenDarkEngine test");

		mRl1 = mDrawService->createRenderedLabel(fds, "TEST 2");
		mRl2 = mDrawService->createRenderedLabel(fds, "ONLY VISIBLE IN CLIPPED AREA\nThis text is a multiline text that is only to be seen\nif inside a clipping rectangle that it is set to be\nviewed through.");

		ClipRect cr = mDrawService->getClipRect(150, 300, 150, 176);
		mRl2->setClipRect(cr);

		RenderedImage* ri = mDrawService->createRenderedImage(ds);
		ri->setPosition(100, 250);



		rl->setPosition(250, 100);
		mRl1->setPosition(0, 0);
		mRl2->setPosition(0, 12);
		rl->setZOrder(2);
		mRl2->setZOrder(2);
		mRl2->setZOrder(2);

		DrawSheet* dsh = mDrawService->createSheet("GameScreen");

		dsh->addDrawOperation(ri);
		dsh->addDrawOperation(rl);
		dsh->addDrawOperation(mRl1);
		dsh->addDrawOperation(mRl2);

		dsh->setVisible(true);

		mSceneMgr->getRootSceneNode()->attachObject(dsh);
		mDrawService->setActiveSheet(dsh);
	*/
		LOG_INFO("GamePlayState: Started");
	}

	void GamePlayState::exit() {
		LOG_INFO("GamePlayState: Exiting");

		// mConsole->setActive(false);

		// Debugging pos/dir writes. For performance profiling
		// std::cerr << mCamera->getPosition() << std::endl;
		// std::cerr << mCamera->getDirection() << std::endl;

		if (mPortalOverlay)
			mPortalOverlay->hide();

		if (mDebugOverlay)
			mDebugOverlay->hide();
		/*mConsole->setActive(false);
		mConsole->update(1); // to hide the console while exiting
		*/

		if (mToLoadScreen) {
			pushState(GameLoadState::getSingletonPtr());
			mToLoadScreen = false;
		}

		LOG_INFO("GamePlayState: Exited");
	}

	void GamePlayState::suspend() {
		LOG_INFO("GamePlayState: Suspend?!");
		mToLoadScreen = false;
	}

	void GamePlayState::resume() {
		LOG_INFO("GamePlayState: Resume?!");
   		mToLoadScreen = false;
	}

	void GamePlayState::update(unsigned long timePassed) {
        mMoveScale = mMoveSpeed * timePassed / 1000000.0f;
        mRotScale = mRotateSpeed * timePassed / 1000000.0f;

		// Quick hack. Let the camera move:
		if (mForward)
			mTranslateVector.z = -mMoveScale;

		if (mBackward)
			mTranslateVector.z =  mMoveScale;

		if (mLeft)
			mTranslateVector.x = -mMoveScale;

		if (mRight)
			mTranslateVector.x =  mMoveScale;

		mCamera->yaw(mRotX * mRotScale);
		mCamera->pitch(mRotY * mRotScale);
		mCamera->moveRelative(mTranslateVector);

		mTranslateVector = Vector3::ZERO;
		mRotX = 0;
		mRotY = 0;

		if (mSceneDisplay) {
			mSceneDetailIndex = (mSceneDetailIndex+1)%2 ; // I Do not need points for now
			switch(mSceneDetailIndex) {
				case 0 : mCamera->setPolygonMode(PM_SOLID) ; break ;
				case 1 : mCamera->setPolygonMode(PM_WIREFRAME) ; break ;
				//case 2 : mCamera->setPolygonMode(PM_POINTS) ; break ;
			}
			mSceneDisplay = false;
		}

		if (mPortalDisplay) {
			// reuse
			mSceneMgr->getOption("ShowPortals", &mPortalDisplay);
			mPortalDisplay = !mPortalDisplay;
			mSceneMgr->setOption("ShowPortals", &mPortalDisplay);

			mPortalDisplay = false;
		}

		if (mScreenShot) {
			char tmp[20];
			sprintf(tmp, "screenshot_%d.png", ++mNumScreenShots);
			RenderWindow* w = Ogre::Root::getSingleton().getAutoCreatedWindow();

			w->writeContentsToFile(tmp);

			mScreenShot = false;
		}

		// mConsole->update(timePassed);

		if (mDebug) {
			// update stats when necessary
			try {

				// Temporary: Debug Overlay
				static String currFps = "Current FPS: ";
				static String avgFps = "Average FPS: ";
				static String bestFps = "Best FPS: ";
				static String worstFps = "Worst FPS: ";
				static String tris = "Triangle Count: ";
				static String batches = "Batch Count: ";

				OverlayElement* guiAvg = OverlayManager::getSingleton().getOverlayElement("Opde/AverageFps");
				OverlayElement* guiCurr = OverlayManager::getSingleton().getOverlayElement("Opde/CurrFps");
				OverlayElement* guiBest = OverlayManager::getSingleton().getOverlayElement("Opde/BestFps");
				OverlayElement* guiWorst = OverlayManager::getSingleton().getOverlayElement("Opde/WorstFps");

				const RenderTarget::FrameStats& stats = mWindow->getStatistics();

				guiAvg->setCaption(avgFps + StringConverter::toString(stats.avgFPS));
				guiCurr->setCaption(currFps + StringConverter::toString(stats.lastFPS));
				guiBest->setCaption(bestFps + StringConverter::toString(stats.bestFPS)
				+" "+StringConverter::toString(stats.bestFrameTime)+" ms");
				guiWorst->setCaption(worstFps + StringConverter::toString(stats.worstFPS)
				+" "+StringConverter::toString(stats.worstFrameTime)+" ms");

				OverlayElement* guiTris = OverlayManager::getSingleton().getOverlayElement("Opde/NumTris");
				guiTris->setCaption(tris + StringConverter::toString(stats.triangleCount));

				OverlayElement* guiBatches = OverlayManager::getSingleton().getOverlayElement("Opde/NumBatches");
				guiBatches->setCaption(batches + StringConverter::toString(stats.batchCount));

				// OverlayElement* guiDbg = OverlayManager::getSingleton().getOverlayElement("Core/DebugText");
			}
			catch(...)
			{
				// ignore
			}

			// update the portal statistics
			try {
				// Volca: I've disabled the timing reports, they need a patch of SM to work
				OverlayElement* guibc = OverlayManager::getSingleton().getOverlayElement("Opde/BackCulls");
				OverlayElement* guiep = OverlayManager::getSingleton().getOverlayElement("Opde/EvalPorts");
				OverlayElement* guirc = OverlayManager::getSingleton().getOverlayElement("Opde/RendCells");
				OverlayElement* guitt = OverlayManager::getSingleton().getOverlayElement("Opde/TravTime");
				OverlayElement* guisr = OverlayManager::getSingleton().getOverlayElement("Opde/StaticRenderTime");
				OverlayElement* guivo = OverlayManager::getSingleton().getOverlayElement("Opde/VisibleObjectsTime");
				OverlayElement* guill = OverlayManager::getSingleton().getOverlayElement("Opde/LightListTime");
				OverlayElement* guilc = OverlayManager::getSingleton().getOverlayElement("Opde/LightCount");
				OverlayElement* guisg = OverlayManager::getSingleton().getOverlayElement("Opde/SceneGraphTime");

				// Temporary: Debug Overlay
				static String sbc = "Backface culls: ";
				static String sep = "Evaluated portals: ";
				static String src = "Rendered cells: ";
				static String stt = "Traversal Time: ";
				static String ssr = "Static Build Time: ";
				static String vot = "Visible obj. Time: ";
				static String llt = "Light list. Time: ";
				static String lcs = "Light count : ";
				static String sgt = "Scene graph Time: ";

				uint bculls = 0, eports = 0, rendc = 0, travtm = 0;

				unsigned long statbt, fvot, lltime, sgtime, lcnt;

				mSceneMgr->getOption("BackfaceCulls", &bculls);

				// mSceneMgr->getOption("CellsRendered", &rendc);
				mSceneMgr->getOption("EvaluatedPortals", &eports);
				// mSceneMgr->getOption("TraversalTime", &travtm);
				mSceneMgr->getOption("StaticBuildTime", &statbt);
				mSceneMgr->getOption("FindVisibleObjectsTime", &fvot);
				mSceneMgr->getOption("LightListTime", &lltime);
				mSceneMgr->getOption("LightCount", &lcnt);
				mSceneMgr->getOption("SceneGraphTime", &sgtime);

				travtm = static_cast<DarkCamera*>(mCamera)->getTraversalTime();
				rendc = static_cast<DarkCamera*>(mCamera)->getVisibleCellCount();

				guibc->setCaption(sbc + StringConverter::toString(bculls));
				guiep->setCaption(sep + StringConverter::toString(eports));
				guirc->setCaption(src + StringConverter::toString(rendc));
				guitt->setCaption(stt + StringConverter::toString(travtm) + " ms");
				guisr->setCaption(ssr + StringConverter::toString(statbt) + " ms");
				guivo->setCaption(vot + StringConverter::toString(fvot) + " ms");
				guill->setCaption(llt + StringConverter::toString(lltime) + " ms");
				guilc->setCaption(lcs + StringConverter::toString(lcnt));
				guisg->setCaption(sgt + StringConverter::toString(sgtime) + " ms");
			}
			catch(...)
			{
				// ignore
			}
		}
	}

	bool GamePlayState::keyPressed(const SDL_KeyboardEvent &e) {
        if(e.keysym.sym == SDLK_w) {
            mForward = true;
            return true;
        } else if(e.keysym.sym == SDLK_s) {
            mBackward = true;
            return true;
        } else if(e.keysym.sym == SDLK_a) {
            mLeft = true;
            return true;
        } else if(e.keysym.sym == SDLK_d) {
            mRight = true;
            return true;
        } else if (e.keysym.sym == SDLK_PRINTSCREEN || e.keysym.sym == SDLK_F5) {
            mScreenShot = true;
            return true;
        } else if (e.keysym.sym == SDLK_o) {
            mSceneDisplay = true;
            return true;
        } else if (e.keysym.sym == SDLK_p) {
            mPortalDisplay = true;
            return true;
        } else return true;
	}

    bool GamePlayState::keyReleased(const SDL_KeyboardEvent &e) {
        if(e.keysym.sym == SDLK_w) {
            mForward = false;
            return true;
        } else if(e.keysym.sym == SDLK_s) {
            mBackward = false;
            return true;
        } else if(e.keysym.sym == SDLK_a) {
            mLeft = false;
            return true;
        } else if(e.keysym.sym == SDLK_d) {
            mRight = false;
            return true;
        } else if(e.keysym.sym == SDLK_ESCAPE) {
            requestTermination();
            return true;
        } else if (e.keysym.sym == SDLK_i) {
            mShadows = !mShadows;

            if (mShadows)
                mSceneMgr->setShadowTechnique(SHADOWTYPE_TEXTURE_MODULATIVE);
            else
                mSceneMgr->setShadowTechnique(SHADOWTYPE_NONE);
        }

        return true;
    }

	bool GamePlayState::mouseMoved(const SDL_MouseMotionEvent &e) {
		mRotX -= Degree( e.xrel );
		// use Y axis invert
		mRotY -= Degree( e.yrel * mRotateYFactor);
		return false;
	}

	bool GamePlayState::mousePressed(const SDL_MouseButtonEvent &e) {
		return false;
	}

	bool GamePlayState::mouseReleased(const SDL_MouseButtonEvent &e) {
		return false;
	}

	void GamePlayState::commandExecuted(std::string command, std::string parameters) {
		std::cerr << "command " << command  << " " << parameters << std::endl;

		if (command == "load") {
			// specify the mission file to load by the load state, then switch to the load state
			mConfigService->setParam("mission", parameters);
			mToLoadScreen = true;
			popState();
		} else if (command == "fps") {
			const RenderTarget::FrameStats& stats = mWindow->getStatistics();

			LOG_INFO("Average FPS : %10.2f", stats.avgFPS);
			LOG_INFO("Last FPS    : %10.2f", stats.lastFPS);
			LOG_INFO("Worst FPS   : %10.2f", stats.worstFPS);
		}
	}

	void GamePlayState::onLinkPlayerFactoryMsg(const LinkChangeMsg& msg) {
		if (msg.change == LNK_ADDED) {
			LOG_INFO("GamePlayState: Found StartingPoint");
			// get the Link ref.
			LinkPtr l = mPlayerFactoryRelation->getLink(msg.linkID);
			StartingPointObjID = l->src();
		}
	}

	void GamePlayState::bootstrapFinished() {
		mLinkService = GET_SERVICE(LinkService);
		Relation::ListenerPtr metaPropCallback(
			new ClassCallback<LinkChangeMsg, GamePlayState>(this, &GamePlayState::onLinkPlayerFactoryMsg));

		mPlayerFactoryRelation = mLinkService->getRelation("PlayerFactory");

		if (mPlayerFactoryRelation.isNull())
			OPDE_EXCEPT("PlayerFactory relation not found. Fatal.", "GamePlayState::bootstrapFinished");

		mPlayerFactoryListenerID = mPlayerFactoryRelation->registerListener(metaPropCallback);
		LOG_INFO("GamePlayState::bootstrapFinished() - done");
	}

	GamePlayState& GamePlayState::getSingleton() {
		assert(ms_Singleton); return *ms_Singleton;
	}

	GamePlayState* GamePlayState::getSingletonPtr() {
		return ms_Singleton;
	}

}
