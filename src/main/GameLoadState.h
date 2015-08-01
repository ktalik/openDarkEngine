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


#ifndef __GAMELOADSTATE_H
#define __GAMELOADSTATE_H

#include "config.h"

#include "GameState.h"
#include "OpdeSingleton.h"
#include "FileGroup.h"
#include "WorldRepService.h"
#include "OpdeServiceManager.h"
#include "ManualFonFileLoader.h"

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <Overlay/OgreOverlayManager.h>


namespace Opde {

	class GameLoadState : public Singleton<GameLoadState>, public GameState {
		public:
			GameLoadState();
			virtual ~GameLoadState();

			virtual void start();
			virtual void exit();
			virtual void suspend();
			virtual void resume();

			virtual void update(unsigned long timePassed);

			bool keyPressed(const SDL_KeyboardEvent &e);
			bool keyReleased(const SDL_KeyboardEvent &e);

			bool mouseMoved(const SDL_MouseMotionEvent &e);
			bool mousePressed(const SDL_MouseButtonEvent &e);
			bool mouseReleased(const SDL_MouseButtonEvent &e);

			static GameLoadState& getSingleton();
			static GameLoadState* getSingletonPtr();
		protected:
			void createTestFontOverlays();

			Ogre::Root *mRoot;
			Ogre::SceneManager *mSceneMgr;
			Ogre::OverlayManager *mOverlayMgr;
			Ogre::Camera *mCamera;
			Ogre::Viewport *mViewport;

			ServiceManager* mServiceMgr;

			bool mFirstTime;
			bool mLoaded;

			Ogre::Overlay* mLoadingOverlay;

			// config service
			ConfigServicePtr mConfigService;
	};
}

#endif
