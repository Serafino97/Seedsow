#include "StdAfx.h"

#include "TradeManager.h"
#include "World.h"

TradeManager* TradeManager::RegisterTrade(CPlayerWeenie * initiator, CPlayerWeenie * partner)
{
	return new TradeManager(initiator, partner);
}

TradeManager::TradeManager(CPlayerWeenie *initiator, CPlayerWeenie *partner)
{
	_initiator = initiator;
	_partner = partner;

	BinaryWriter openTrade;
	openTrade.Write<DWORD>(0x1FD);
	openTrade.Write<DWORD>(initiator->GetID());
	openTrade.Write<DWORD>(partner->GetID());
	openTrade.Write<long>(0); // "some kind of stamp"?
	initiator->SendNetMessage(&openTrade, PRIVATE_MSG, TRUE, FALSE);


	BinaryWriter openTradePartner;
	openTradePartner.Write<DWORD>(0x1FD);
	openTradePartner.Write<DWORD>(partner->GetID());
	openTradePartner.Write<DWORD>(initiator->GetID());
	openTradePartner.Write<long>(0); // "some kind of stamp"?
	partner->SendNetMessage(&openTradePartner, PRIVATE_MSG, TRUE, FALSE);
	
	// it's possible for users to have items in the trade window before starting so clear it
	ResetTrade(initiator);
}

void TradeManager::CloseTrade(CPlayerWeenie *playerFrom, DWORD reason)
{
	OnCloseTrade(_initiator, reason);
	OnCloseTrade(_partner, reason);

	Delete();
}

void TradeManager::OnCloseTrade(CPlayerWeenie *player, DWORD reason)
{
	if (player)
	{
		BinaryWriter closeTrade;
		closeTrade.Write<DWORD>(0x1FF);
		closeTrade.Write<DWORD>(reason);
		player->SendNetMessage(&closeTrade, PRIVATE_MSG, TRUE, FALSE);
	}
}

void TradeManager::AddToTrade(CPlayerWeenie *playerFrom, DWORD item)
{
	if (!CheckTrade())
		return;

	CWeenieObject *pItem = g_pWorld->FindWithinPVS(playerFrom, item);

	if (!pItem || pItem->GetWorldTopLevelOwner() != playerFrom || pItem->IsAttunedOrContainsAttuned() || pItem->IsWielded())
	{
		playerFrom->SendText("You cannot trade that item!", LTT_ERROR);
		BinaryWriter cannotTrade;
		cannotTrade.Write<DWORD>(0x207);
		cannotTrade.Write<DWORD>(item);
		cannotTrade.Write<DWORD>(0);
		playerFrom->SendNetMessage(&cannotTrade, PRIVATE_MSG, TRUE, FALSE);
		return;
	}

	std::list<DWORD> *itemList;

	if (playerFrom == _initiator)
	{
		itemList = &m_lInitiatorItems;
	}
	else
	{
		itemList = &m_lPartnerItems;
	}

	itemList->push_back(item);

	m_bInitiatorAccepted = false;
	m_bPartnerAccepted = false;

	CPlayerWeenie *pOther = GetOtherPlayer(playerFrom);

	pOther->MakeAware(pItem, true);

	BinaryWriter addToTrade;
	addToTrade.Write<DWORD>(0x200);
	addToTrade.Write<DWORD>(item);
	addToTrade.Write<DWORD>(0x1);
	addToTrade.Write<DWORD>(0);
	playerFrom->SendNetMessage(&addToTrade, PRIVATE_MSG, TRUE, FALSE);

	BinaryWriter addToTradeOther;
	addToTradeOther.Write<DWORD>(0x200);
	addToTradeOther.Write<DWORD>(item);
	addToTradeOther.Write<DWORD>(0x2);
	addToTradeOther.Write<DWORD>(0);
	pOther->SendNetMessage(&addToTradeOther, PRIVATE_MSG, TRUE, FALSE);
}

void TradeManager::AcceptTrade(CPlayerWeenie *playerFrom)
{
	if (!CheckTrade())
		return;

	if (playerFrom == _initiator)
		m_bInitiatorAccepted = true;
	else
		m_bPartnerAccepted = true;

	if (m_bInitiatorAccepted && m_bPartnerAccepted)
	{
		OnTradeAccepted();
	}
	else
	{
		BinaryWriter acceptTrade;
		acceptTrade.Write<DWORD>(0x202);
		acceptTrade.Write<DWORD>(playerFrom->GetID());
		//playerFrom->SendNetMessage(&acceptTrade, PRIVATE_MSG, TRUE, FALSE);
		GetOtherPlayer(playerFrom)->SendNetMessage(&acceptTrade, PRIVATE_MSG, TRUE, FALSE);
	}
}

bool TradeManager::OnTradeAccepted()
{
	bool bError = false;
	if (m_lPartnerItems.size() > _initiator->Container_GetNumFreeMainPackSlots())
	{
		_initiator->SendText("You do not have enough pack space to complete this trade!", LTT_ERROR);
		_partner->SendText((_initiator->GetName() = " does not have enough pack space to complete this trade!").c_str(), LTT_ERROR);
		bError = true;
	}
	if (m_lInitiatorItems.size() > _partner->Container_GetNumFreeMainPackSlots())
	{
		_partner->SendText("You do not have enough pack space to complete this trade!", LTT_ERROR);
		_initiator->SendText((_partner->GetName() = " does not have enough pack space to complete this trade!").c_str(), LTT_ERROR);
		bError = true;
	}

	// TODO check if this takes player over 300% burden

	std::list<CWeenieObject*> lpInitiatorItems;
	for (auto it = m_lInitiatorItems.begin(); it != m_lInitiatorItems.end(); ++it)
	{
		CWeenieObject *pItem = g_pWorld->FindWithinPVS(_initiator, *it);
		lpInitiatorItems.push_back(pItem);

		if (!pItem)
		{
			_initiator->SendText("Invalid item in trade!", LTT_ERROR);
			_partner->SendText("Invalid item in trade!", LTT_ERROR);
			bError = true;
			break;
		}
		else if (pItem->GetWorldTopLevelOwner() != _initiator || pItem->IsWielded() || pItem->IsAttunedOrContainsAttuned())
		{
			_initiator->SendText(("You cannot trade " + pItem->GetName() + "!").c_str(), LTT_ERROR);
			_partner->SendText((_initiator->GetName() + " put invalid items in the trade!").c_str(), LTT_ERROR);
			bError = true;
			break;
		}
	}

	std::list<CWeenieObject*> lpPartnerItems;
	for (auto it = m_lPartnerItems.begin(); it != m_lPartnerItems.end(); ++it)
	{
		CWeenieObject *pItem = g_pWorld->FindWithinPVS(_partner, *it);
		lpPartnerItems.push_back(pItem);

		if (!pItem)
		{
			_initiator->SendText("Invalid item in trade!", LTT_ERROR);
			_partner->SendText("Invalid item in trade!", LTT_ERROR);
			bError = true;
			break;
		}
		else if (pItem->GetWorldTopLevelOwner() != _partner || pItem->IsWielded() || pItem->IsAttunedOrContainsAttuned())
		{
			_partner->SendText(("You cannot trade " + pItem->GetName() + "!").c_str(), LTT_ERROR);
			_initiator->SendText((_partner->GetName() + " put invalid items in the trade!").c_str(), LTT_ERROR);
			bError = true;
			break;
		}
	}

	if (!bError)
	{
		// Swap items
		for (auto it = lpInitiatorItems.begin(); it != lpInitiatorItems.end(); ++it)
		{
			_partner->OnReceiveInventoryItem(_initiator, *it, 0);


			BinaryWriter removeItem;
			removeItem.Write<DWORD>(0x24);
			removeItem.Write<DWORD>((*it)->GetID());
			_initiator->SendNetMessage(&removeItem, PRIVATE_MSG, TRUE, FALSE);
		}
		for (auto it = lpPartnerItems.begin(); it != lpPartnerItems.end(); ++it)
		{
			_initiator->OnReceiveInventoryItem(_partner, *it, 0);

			BinaryWriter removeItem;
			removeItem.Write<DWORD>(0x24);
			removeItem.Write<DWORD>((*it)->GetID());
			_partner->SendNetMessage(&removeItem, PRIVATE_MSG, TRUE, FALSE);
		}


		// Trade Complete!
		_initiator->NotifyWeenieError(0x529);
		_partner->NotifyWeenieError(0x529);

		// reset the trade to continue trading
		ResetTrade(_initiator);

		return true;
	}
	

	// Unfortunately, you cannot un-accept a completed trade as the accepting client bugs out
	CloseTrade(_initiator, 0);

	return false;
}

void TradeManager::DeclineTrade(CPlayerWeenie *playerFrom)
{
	if (!CheckTrade())
		return;

	if (playerFrom == _initiator)
		m_bInitiatorAccepted = false;
	else
		m_bPartnerAccepted = false;

	BinaryWriter declineTrade;
	declineTrade.Write<DWORD>(0x203);
	declineTrade.Write<DWORD>(playerFrom->GetID());
	//playerFrom->SendNetMessage(&declineTrade, PRIVATE_MSG, TRUE, FALSE);
	GetOtherPlayer(playerFrom)->SendNetMessage(&declineTrade, PRIVATE_MSG, TRUE, FALSE);
}

void TradeManager::ResetTrade(CPlayerWeenie *playerFrom)
{
	if (!CheckTrade())
		return;

	// remove all items
	m_lInitiatorItems.clear();
	m_lPartnerItems.clear();

	CPlayerWeenie *other = GetOtherPlayer(playerFrom);

	BinaryWriter resetTrade;
	resetTrade.Write<DWORD>(0x205);
	resetTrade.Write<DWORD>(playerFrom->GetID());
	playerFrom->SendNetMessage(&resetTrade, PRIVATE_MSG, TRUE, FALSE);
	GetOtherPlayer(playerFrom)->SendNetMessage(&resetTrade, PRIVATE_MSG, TRUE, FALSE);
}

CPlayerWeenie* TradeManager::GetOtherPlayer(CPlayerWeenie *player)
{
	if (player == _initiator)
		return _partner;

	return _initiator;
};

// Checks whether trade is still legit. True is so.
bool TradeManager::CheckTrade()
{
	// not currently trading
	if (!_initiator || !_partner)
	{
		OnCloseTrade(_initiator);
		OnCloseTrade(_partner);
		Delete();
		return false;
	}

	return true;
}

// Removes references and then removes this from memory
void TradeManager::Delete()
{
	// Delete all references to this
	if (_initiator != NULL)
	{
		_initiator->SetTradeManager(NULL);
	}
	if (_partner != NULL)
	{
		_partner->SetTradeManager(NULL);
	}

	// MUST be the final thing done in this class
	delete this;
}

void TradeManager::CheckDistance()
{
	if (!CheckTrade())
		return;

	if ( _initiator->DistanceTo(_partner, true) >= 1 )
	{
		_initiator->SendText((_partner->GetName() + " is too far away to trade!").c_str(), LTT_ERROR);
		_partner->SendText((_initiator->GetName() + " is too far away to trade!").c_str(), LTT_ERROR);

		CloseTrade(_initiator, 1);
	}
}