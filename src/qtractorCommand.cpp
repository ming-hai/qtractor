// qtractorCommand.cpp
//
/****************************************************************************
   Copyright (C) 2005-2006, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*****************************************************************************/

#include "qtractorAbout.h"
#include "qtractorCommand.h"

#include "qtractorMainForm.h"

#include "qtractorSession.h"
#include "qtractorTracks.h"


//----------------------------------------------------------------------
// class qtractorCommand - implementation.
//

// Constructor.
qtractorCommand::qtractorCommand ( qtractorMainForm *pMainForm,
	const QString& sName )
{
	m_pMainForm   = pMainForm;
	m_sName       = sName;
	m_bAutoDelete = false;
}


// Destructor.
qtractorCommand::~qtractorCommand (void)
{
}


//----------------------------------------------------------------------
// class qtractorCommandList - declaration.
//

// Constructor.
qtractorCommandList::qtractorCommandList ( qtractorMainForm *pMainForm )
{
	m_pMainForm    = pMainForm;
	m_pLastCommand = NULL;

	m_commands.setAutoDelete(true);
}

// Destructor.
qtractorCommandList::~qtractorCommandList (void)
{
	clear();
}


// Command stack cleaner.
void qtractorCommandList::clear (void)
{
	m_commands.clear();

	m_pLastCommand = NULL;
}


// Command chain accessors.
qtractorCommand *qtractorCommandList::lastCommand (void) const
{
	return m_pLastCommand;
}

qtractorCommand *qtractorCommandList::nextCommand (void) const
{
	if (m_pLastCommand) {
		return m_pLastCommand->next();
	} else {
		return m_commands.first();
	}
}


// Cannonical command methods.
bool qtractorCommandList::exec ( qtractorCommand *pCommand )
{
	bool bResult = false;

	// Trim the command list from current last command...
	qtractorCommand *pNextCommand = nextCommand();
	while (pNextCommand) {
		qtractorCommand *pLateCommand = pNextCommand->next();
		m_commands.remove(pNextCommand);
		pNextCommand = pLateCommand;
	}

	if (pCommand) {
		m_commands.append(pCommand);
		m_pLastCommand = m_commands.last();
		if (m_pLastCommand) {
			// Execute operation...
			bResult = m_pLastCommand->redo();
			// Log this operation.
			if (bResult) {
				m_pMainForm->appendMessages(
					QObject::tr("Command (%1) succeeded.")
						.arg(m_pLastCommand->name()));
			} else {
				m_pMainForm->appendMessagesError(
					QObject::tr("Command (%1) failed.")
						.arg(m_pLastCommand->name()));
			}
			// Notify commanders...
			update();
		}
	}

	return bResult;
}

bool qtractorCommandList::undo (void)
{
	bool bResult = false;

	if (m_pLastCommand) {
		// Undo operation...
		bResult = m_pLastCommand->undo();
		// Log this operation.
		if (bResult) {
			m_pMainForm->appendMessages(
				QObject::tr("Undo (%1) succeeded.")
					.arg(m_pLastCommand->name()));
		} else {
			m_pMainForm->appendMessagesError(
				QObject::tr("Undo (%1) failed.")
					.arg(m_pLastCommand->name()));
		}
		// Backward one command...
		m_pLastCommand = m_pLastCommand->prev();
		// Notify commanders...
		update();
	}

	return bResult;
}

bool qtractorCommandList::redo (void)
{
	bool bResult = false;

	// Forward one command...
	m_pLastCommand = nextCommand();
	if (m_pLastCommand) {
		// Redo operation...
		bResult = m_pLastCommand->redo();
		// Log this operation.
		if (bResult) {
			m_pMainForm->appendMessages(
				QObject::tr("Redo (%1) succeeded.")
					.arg(m_pLastCommand->name()));
		} else {
			m_pMainForm->appendMessagesError(
				QObject::tr("Redo (%1) failed.")
					.arg(m_pLastCommand->name()));
		}
		// Notify commanders...
		update();
	}

	return bResult;
}


// Command update helper.
void qtractorCommandList::update (void) const
{
	qtractorSession *pSession = m_pMainForm->session();
	if (pSession == NULL)
		return;

	// Maybe, just maybe, we've made things larger...
	pSession->updateTimeScale();
	pSession->updateSessionLength();

	qtractorTracks *pTracks = m_pMainForm->tracks();
	if (pTracks == NULL)
		return;

	// Refresh view.
	pTracks->updateContents(true);
	// Notify who's watching...
	pTracks->contentsChangeNotify();
}


// end of qtractorCommand.cpp
