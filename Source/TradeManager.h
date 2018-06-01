#pragma once

#include "Player.h"

class TradeManager
{
public:
	static TradeManager* RegisterTrade(CPlayerWeenie *initiator, CPlayerWeenie *partner);

	void CloseTrade(CPlayerWeenie *playerFrom, DWORD reason = 0x51);
	void OnCloseTrade(CPlayerWeenie *player, DWORD reason = 0);

	void AddToTrade(CPlayerWeenie *playerFrom, DWORD item);
	//void RemoveFromTrade(CPlayerWeenie *playerFrom, DWORD item);

	void AcceptTrade(CPlayerWeenie *playerFrom);
	bool OnTradeAccepted();

	void DeclineTrade(CPlayerWeenie *playerFrom);

	void ResetTrade(CPlayerWeenie *playerFrom);

	CPlayerWeenie* GetOtherPlayer(CPlayerWeenie *player);

	void CheckDistance();
private:
	// prevent anyone from initiating this outside of OpenTrade
	TradeManager(CPlayerWeenie *initiator, CPlayerWeenie *partner);

	void Delete();

	CPlayerWeenie *_initiator;
	CPlayerWeenie *_partner;

	bool m_bInitiatorAccepted = false;
	bool m_bPartnerAccepted = false;

	std::list<DWORD> m_lInitiatorItems;
	std::list<DWORD> m_lPartnerItems;

	// double check the trade is still legit
	bool CheckTrade();
};
