// This file is part of BOINC.
// http://boinc.berkeley.edu
// Copyright (C) 2008 University of California
//
// BOINC is free software; you can redistribute it and/or modify it
// under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation,
// either version 3 of the License, or (at your option) any later version.
//
// BOINC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with BOINC.  If not, see <http://www.gnu.org/licenses/>.
// 

#ifndef __SCREENSAVER_H__
#define __SCREENSAVER_H__


#define GFX_CHANGE_PERIOD 600 /* if > 1 CPUs, change screensaver every 600 secs */


//-----------------------------------------------------------------------------
// Error / status codes
//-----------------------------------------------------------------------------

#define SCRAPPERR_BOINCNOTDETECTED                          0x82000001
#define SCRAPPERR_BOINCNOTDETECTEDSTARTUP                   0x82000002
#define SCRAPPERR_BOINCSUSPENDED                            0x82000003
#define SCRAPPERR_BOINCNOTGRAPHICSCAPABLE                   0x82000004
#define SCRAPPERR_BOINCNOAPPSEXECUTING                      0x82000005
#define SCRAPPERR_BOINCNOPROJECTSDETECTED                   0x82000006
#define SCRAPPERR_BOINCNOGRAPHICSAPPSEXECUTING              0x82000007  
#define SCRAPPERR_BOINCSCREENSAVERLOADING                   0x82000008
#define SCRAPPERR_BOINCAPPFOUNDGRAPHICSLOADING              0x82000009
#define SCRAPPERR_BOINCSHUTDOWNEVENT                        0x8200000a
#define SCRAPPERR_NOPREVIEW                                 0x8200000f
#define SCRAPPERR_DAEMONALLOWSNOGRAPHICS                    0x82000010
#define SCRAPPERR_SCREENSAVERRUNNING                        0x82000011
#define SCRAPPERR_SCREENSAVERBLANKED                        0x82000012
#define SCRAPPERR_QUITSCREENSAVERREQUESTED                  0x82000013

#endif
