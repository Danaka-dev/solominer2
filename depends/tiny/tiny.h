#pragma once

/****************************************************** //
//              tiny-for-c++ v3 library                 //
//              -----------------------                 //
//   Copyright (c) 2016-2024 | NEXTWave Technologies    //
//      <http://www.nextwave-techs.com/>                //
// ******************************************************/

//! Check if your project qualifies for a free license
//!   at http://nextwave-techs.com/license

//! THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//! IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//! FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//!        AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//! LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//! OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
//! SOFTWARE.

//////////////////////////////////////////////////////////////////////////////
#ifndef TINY_H
#define TINY_H

//////////////////////////////////////////////////////////////////////////////
//! @file "tiny.h"
//! @brief tiny-for-c++ main header
//! @author the NEXTWave developers
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! tiny for C

#define TINY_VERSION		"3.01.2018"
#define TINY_VERSIONNUM		3012018
#define TINY_DEVCYCLE		"BETA"

//! BOSS - Basic OS Service
#include "tiny-os.h"

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//! tiny for C++

#ifdef __cplusplus

//////////////////////////////////////////////////////////////////////////////
//! tiny foundation

//! Basic definitions
#include "tiny-defs.h"

//! Basic C++ types implementation
#ifdef TINY_TYPES_TINY
 #include "tiny-types.hpp" //! in defs
#endif

//! Basic C++ object & interface
#include "tiny-object.hpp"

//! Basic methodology & helpers
#include "tiny-base.hpp"

//! Basic C++ BOSS wrappers
#include "tiny-core.hpp"

//! Gui C++ BOSS wrappers
#ifndef TINY_NO_GUI
 #include "tiny-gui.hpp"
#else
 #define TINY_NO_XGUI
#endif

//////////////////////////////////////////////////////////////////////////////
//! tiny extensions

#include "common/guid.h"
#include "common/text.h"
#include "common/time.h"
#include "common/patterns.h"
#include "common/memory.h"

//! x-app (all)
#ifndef TINY_NO_XAPP
 #include "x-app/app-config.h"
 #include "x-app/app-test.h"
 #include "x-app/app-service.h"
 // #include "x-app/app-sim.h"
#endif

 //! x-data (all)
#ifndef TINY_NO_XDATA
 #include "x-data/data-config.h"
#endif

//! x-gui (all)
#ifndef TINY_NO_XGUI
 //TODO gui -> gui-core ... gui.h pack include below

 #include "x-gui/gui.h"
 // #include "x-gui/gui-core.h"
 #include "x-gui/gui-assets.h"
 #include "x-gui/gui-theme.h"

 #include "x-gui/controls/gui-controls.h"
 #include "x-gui/controls/gui-edit.h"
 #include "x-gui/controls/gui-grid.h"
 #include "x-gui/controls/gui-graph.h"
 #include "x-gui/controls/gui-plot.h"

 #include "x-gui/gui-editor.h"
#endif

//! x-math
 //...

//! x-www
 //...

//////////////////////////////////////////////////////////////////////////////

#endif // __cplusplus

//////////////////////////////////////////////////////////////////////////////
#endif // TINY_H