#ifndef __ISPF_H__
#define __ISPF_H__

#include <emulight/ispf/msgid.h>
#include <emulight/ispf/panel.h>


/******************************************************************************/
/* Common ISPF routines                                                       */
/******************************************************************************/
void	SPFInit(SPF* spf);
void	SPFDraw(SPF* spf);
void	SPFUpdate(SPF* spf);
void	SPFProcess(SPF* spf);
void	SPFRepaint(SPF* spf);


/******************************************************************************/
/* Message box routines                                                       */
/******************************************************************************/
void	SPFShowMessage(SPF* spf, const char* title, const char* message);
void	SPFShowError(SPF* spf, const char* message);


/******************************************************************************/
/* Message box routines                                                       */
/******************************************************************************/
void	SPFCreatePanel(SPF* spf, const SPFPanelDefinition* panel);
void	SPFDrawPanel(SPFPanel* panel);
void	SPFUpdatePanel(SPFPanel* panel);
void	SPFProcessPanel(SPFPanel* panel, u16 key);
void	SPFDestroyPanel(SPFPanel* panel);


/******************************************************************************/
/* Menu routines                                                              */
/******************************************************************************/
void	SPFSetMenuBar(SPF* spf, const SPFMenuData* data,
		SPFMenuHandler handler);
void	SPFDrawMenu(SPF* spf);
void	SPFDrawMenuBar(SPF* spf);
void	SPFProcessMenu(SPF* spf, u16 key);
BOOL	SPFProcessMenuHotkeys(SPF* spf, u16 key);
void	SPFShowMenu(SPF* spf);
void	SPFHideMenu(SPF* spf);


/******************************************************************************/
/* Status Bar routines                                                        */
/******************************************************************************/
void	SPFInitStatusBar(SPF* spf);
void	SPFDrawStatusBar(SPF* spf);
void	SPFUpdateStatusBar(SPF* spf);
void	SPFSetStatusBarDrawCallback(SPF* spf, SPFStatusBarHandler handler);
void	SPFSetStatusBarUpdateCallback(SPF* spf, SPFStatusBarHandler handler);

#endif
