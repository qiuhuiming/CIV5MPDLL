/*	-------------------------------------------------------------------------------------------------------
	?1991-2012 Take-Two Interactive Software and its subsidiaries.  Developed by Firaxis Games.  
	Sid Meier's Civilization V, Civ, Civilization, 2K Games, Firaxis Games, Take-Two Interactive Software 
	and their respective logos are all trademarks of Take-Two interactive Software, Inc.  
	All other marks and trademarks are the property of their respective owners.  
	All rights reserved. 
	------------------------------------------------------------------------------------------------------- */
#include "CvGameCoreDLLPCH.h"
#include "CvDllNetMessageHandler.h"
#include "CvDllContext.h"

#include "CvDiplomacyAI.h"
#include "CvTypes.h"
#include "CvGameCoreUtils.h"
#include "ArgContainer.pb.h"
#include "NetworkMessageAdapter.h"

CvDllNetMessageHandler::CvDllNetMessageHandler()
{
}
//------------------------------------------------------------------------------
CvDllNetMessageHandler::~CvDllNetMessageHandler()
{
}
//------------------------------------------------------------------------------
void* CvDllNetMessageHandler::QueryInterface(GUID guidInterface)
{
	if(guidInterface == ICvUnknown::GetInterfaceId() ||
	        guidInterface == ICvNetMessageHandler1::GetInterfaceId() ||
	        guidInterface == ICvNetMessageHandler2::GetInterfaceId() ||
			guidInterface == ICvNetMessageHandler3::GetInterfaceId())
	{
		return this;
	}

	return NULL;
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::Destroy()
{
	// Do nothing.
	// This is a static class whose instance is managed externally.
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::operator delete(void* p)
{
	CvDllGameContext::Free(p);
}
//------------------------------------------------------------------------------
void* CvDllNetMessageHandler::operator new(size_t bytes)
{
	return CvDllGameContext::Allocate(bytes);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseAdvancedStartAction(PlayerTypes ePlayer, AdvancedStartActionTypes eAction, int iX, int iY, int iData, bool bAdd)
{
	GET_PLAYER(ePlayer).doAdvancedStartAction(eAction, iX, iY, iData, bAdd);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseAutoMission(PlayerTypes ePlayer, int iUnitID)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvUnit* pkUnit = kPlayer.getUnit(iUnitID);
	if(pkUnit)
	{
		pkUnit->AutoMission();
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseBarbarianRansom(PlayerTypes ePlayer, int iOptionChosen, int iUnitID)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

	// Pay ransom
	if(iOptionChosen == 0)
	{
		CvTreasury* pkTreasury = kPlayer.GetTreasury();
		int iNumGold = /*100*/ GC.getBARBARIAN_UNIT_GOLD_RANSOM_exp();
		const int iTreasuryGold = pkTreasury->GetGold();
		if(iNumGold > iTreasuryGold)
		{
			iNumGold = iTreasuryGold;
		}

		pkTreasury->ChangeGold(-iNumGold);
	}
	// Abandon Unit
	else if(iOptionChosen == 1)
	{
		CvUnit* pkUnit = kPlayer.getUnit(iUnitID);
		if(pkUnit != NULL)
			pkUnit->kill(true, BARBARIAN_PLAYER);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseChangeWar(PlayerTypes ePlayer, TeamTypes eRivalTeam, bool bWar)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvTeam& kTeam = GET_TEAM(kPlayer.getTeam());
	const TeamTypes eTeam = kPlayer.getTeam();

	FAssert(eTeam != eRivalTeam);

	if(bWar)
	{
#if defined(MOD_EVENTS_WAR_AND_PEACE)
		kTeam.declareWar(eRivalTeam, false, ePlayer);
#else
		kTeam.declareWar(eRivalTeam);
#endif
	}
	else
	{
#if defined(MOD_EVENTS_WAR_AND_PEACE)
		kTeam.makePeace(eRivalTeam, true, false, ePlayer);
#else
		kTeam.makePeace(eRivalTeam);
#endif
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseIgnoreWarning(PlayerTypes ePlayer, TeamTypes eRivalTeam)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvTeam& kTeam = GET_TEAM(kPlayer.getTeam());
	const TeamTypes eTeam = kPlayer.getTeam();
	FAssert(eTeam != eRivalTeam);
	
	kTeam.PushIgnoreWarning(eRivalTeam);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseCityBuyPlot(PlayerTypes ePlayer, int iCityID, int iX, int iY)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pkCity = kPlayer.getCity(iCityID);
	if(pkCity != NULL)
	{
		CvPlot* pkPlot = NULL;

		// (-1,-1) means pick a random plot to buy
		if(iX == -1 && iY == -1)
		{
			pkPlot = pkCity->GetNextBuyablePlot();
		}
		else
		{
			pkPlot = GC.getMap().plot(iX, iY);
		}

		if(pkPlot != NULL)
		{
			if(pkCity->CanBuyPlot(pkPlot->getX(), pkPlot->getY()))
			{
				pkCity->BuyPlot(pkPlot->getX(), pkPlot->getY());
				if(ePlayer == GC.getGame().getActivePlayer() && GC.GetEngineUserInterface()->isCityScreenUp())
				{
					GC.GetEngineUserInterface()->setDirty(CityScreen_DIRTY_BIT, true);
				}
			}
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseCityDoTask(PlayerTypes ePlayer, int iCityID, TaskTypes eTask, int iData1, int iData2, bool bOption, bool bAlt, bool bShift, bool bCtrl)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pkCity = kPlayer.getCity(iCityID);

	if(pkCity != NULL)
	{
		pkCity->doTask(eTask, iData1, iData2, bOption, bAlt, bShift, bCtrl);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseCityPopOrder(PlayerTypes ePlayer, int iCityID, int iNum)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pkCity = kPlayer.getCity(iCityID);
	if(pkCity != NULL)
	{
		pkCity->popOrder(iNum);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseCityPurchase(PlayerTypes ePlayer, int iCityID, UnitTypes eUnitType, BuildingTypes eBuildingType, ProjectTypes eProjectType)
{
	ResponseCityPurchase(ePlayer, iCityID, eUnitType, eBuildingType, eProjectType, NO_YIELD);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseCityPurchase(PlayerTypes ePlayer, int iCityID, UnitTypes eUnitType, BuildingTypes eBuildingType, ProjectTypes eProjectType, int ePurchaseYield)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pkCity = kPlayer.getCity(iCityID);
	if(pkCity && ePurchaseYield >= -1 && ePurchaseYield < NUM_YIELD_TYPES)
	{
		pkCity->Purchase(eUnitType, eBuildingType, eProjectType, static_cast<YieldTypes>(ePurchaseYield));
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseCityPushOrder(PlayerTypes ePlayer, int iCityID, OrderTypes eOrder, int iData, bool bAlt, bool bShift, bool bCtrl)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pkCity = kPlayer.getCity(iCityID);
	if(pkCity != NULL)
	{
		pkCity->pushOrder(eOrder, iData, -1, bAlt, bShift, bCtrl);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseCitySwapOrder(PlayerTypes ePlayer, int iCityID, int iNum)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pkCity = kPlayer.getCity(iCityID);
	if(pkCity != NULL)
	{
		pkCity->swapOrder(iNum);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseChooseElection(PlayerTypes ePlayer, int iSelection, int iVoteId)
{
	// Unused
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseDestroyUnit(PlayerTypes ePlayer, int iUnitID)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvUnit* pkUnit = kPlayer.getUnit(iUnitID);

	if(pkUnit)
	{
		pkUnit->kill(true, ePlayer);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseDiplomacyFromUI(PlayerTypes ePlayer, PlayerTypes eOtherPlayer, FromUIDiploEventTypes eEvent, int iArg1, int iArg2)
{
	GET_PLAYER(eOtherPlayer).GetDiplomacyAI()->DoFromUIDiploEvent(ePlayer, eEvent, iArg1, iArg2);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseDiploVote(PlayerTypes ePlayer, PlayerTypes eVotePlayer)
{
	TeamTypes eVotingTeam = GET_PLAYER(ePlayer).getTeam();
	TeamTypes eVote = GET_PLAYER(eVotePlayer).getTeam();

	GC.getGame().SetVoteCast(eVotingTeam, eVote);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseDoCommand(PlayerTypes ePlayer, int iUnitID, CommandTypes eCommand, int iData1, int iData2, bool bAlt)
{
	PlayerTypes player = ePlayer;

	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvUnit* pkUnit = kPlayer.getUnit(iUnitID);


	if(pkUnit != NULL)
	{
		if(bAlt && GC.getCommandInfo(eCommand)->getAll())
		{
			const UnitTypes eUnitType = pkUnit->getUnitType();

			CvUnit* pkLoopUnit = NULL;
			int iLoop = 0;

			for(pkLoopUnit = kPlayer.firstUnit(&iLoop); pkLoopUnit != NULL; pkLoopUnit = kPlayer.nextUnit(&iLoop))
			{
				if(pkLoopUnit->getUnitType() == eUnitType)
				{
					pkLoopUnit->doCommand(eCommand, iData1, iData2);
				}
			}
		}
		else
		{
			pkUnit->doCommand(eCommand, iData1, iData2);
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseExtendedGame(PlayerTypes ePlayer)
{
	GET_PLAYER(ePlayer).makeExtendedGame();
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseFoundPantheon(PlayerTypes ePlayer, BeliefTypes eBelief)
{
	CvGame& kGame(GC.getGame());
	CvGameReligions* pkGameReligions(kGame.GetGameReligions());
	CvBeliefXMLEntries* pkBeliefs = GC.GetGameBeliefs();
	CvBeliefEntry* pEntry = pkBeliefs->GetEntry((int)eBelief);

	// Pantheon belief, or adding one through Reformation?
	if (pEntry && ePlayer != NO_PLAYER)
	{
		if (pEntry->IsPantheonBelief())
		{
			CvGameReligions::FOUNDING_RESULT eResult = pkGameReligions->CanCreatePantheon(ePlayer, true);
			if(eResult == CvGameReligions::FOUNDING_OK)
			{
#if defined(MOD_TRAITS_ANY_BELIEF)
				if(pkGameReligions->IsPantheonBeliefAvailable(eBelief, ePlayer))
#else
				if(pkGameReligions->IsPantheonBeliefAvailable(eBelief))
#endif
				{
					pkGameReligions->FoundPantheon(ePlayer, eBelief);
				}
				else
				{
					CvGameReligions::NotifyPlayer(ePlayer, CvGameReligions::FOUNDING_BELIEF_IN_USE);
				}
			}
			else
			{
				CvGameReligions::NotifyPlayer(ePlayer, eResult);
			}
		}
		else if (pEntry->IsReformationBelief())
		{
			CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
			if (!pkGameReligions->HasAddedReformationBelief(ePlayer) && kPlayer.GetReligions()->HasCreatedReligion())
			{
				ReligionTypes eReligion = kPlayer.GetReligions()->GetReligionCreatedByPlayer();
				pkGameReligions->AddReformationBelief(ePlayer, eReligion, eBelief);
			}
		}
	}
}

void CvDllNetMessageHandler::TransmissCustomizedOperationFromResponseFoundReligion(
	PlayerTypes ePlayer, 
	int iData1, int iData2, int iData3, int iData4, int iData5, int iData6,
	int customCommandType,
	const char* customMsg) {
	//ArgContainer args;
	//char buffer[1024] = "";
	
	
	//New invoke way: iData1~iData4: Arguments to locate the instance. Like playerID, unitID.
	//iData5: message length, iData6: Invoke id (Avoid repeated execution)
	
	if (customMsg[0] != 'e') {
		NetworkMessageAdapter::StringShiftReverse(NetworkMessageAdapter::ReceiveBuffer, customMsg, iData5);
		if (customCommandType == CUSTOM_OPERATION_UNIT_KILL) return;
		NetworkMessageAdapter::ReceiveArgContainer.ParseFromString(std::string(NetworkMessageAdapter::ReceiveBuffer, iData5));
		string func = NetworkMessageAdapter::ReceiveArgContainer.functiontocall();
		string head = func.substr(0, func.find_first_of(':')) + "::GetArgumentsAndExecute";
		StaticFunctionReflector::ExecuteFunctionWraps<void>(head, &NetworkMessageAdapter::ReceiveArgContainer, iData1, iData2, iData3, iData4);
		NetworkMessageAdapter::ReceiveArgContainer.Clear();
		NetworkMessageAdapter::Clear(NetworkMessageAdapter::ReceiveBuffer, iData5);
		return;
	}
	
	
	if (ReturnValueUtil::container.getReturnValueExist(iData6)) return;
	int realCommandType = customCommandType;
	CvUnit* unit;
	CvCity* city;
	CvPlot* plot;
	//if (ReturnValueUtil::container.getReturnValueExist(iData6)) return;
	switch (realCommandType) {
	case CUSTOM_OPERATION_UNIT_KILL:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: Executer of the command. iData3: bDelay
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->kill(iData2 > 0, (PlayerTypes)iData3);
		}
		break;
	case CUSTOM_OPERATION_UNIT_TELEPORT:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: X. iData3: Y, iData4: boolean flags. iData5: Mark executed
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->setXY(iData2, iData3,
				iData4 & (1 << 0), iData4 & (1 << 1), iData4 & (1 << 2), iData4 & (1 << 3));
		}
		break;
	case CUSTOM_OPERATION_UNIT_SET_DAMAGE:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: iNewValue. iData3: ePlayer. iData4: -1. iData5: bNotifyEntity
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->setDamage(iData2, (PlayerTypes)iData3, iData4, iData5 > 0);
		}
		break;
	case CUSTOM_OPERATION_UNIT_GIVE_PROMOTION:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: promotion. iData3: new value
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->setHasPromotion(PromotionTypes(iData2), iData3 > 0);
		}
		break;
	case CUSTOM_OPERATION_UNIT_SET_LEVEL:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: level.
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->setLevel(iData2);
		}
		break;
	case CUSTOM_OPERATION_UNIT_SET_MOVE:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: New value.
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->setMoves(iData2);
		}
		break;
	case CUSTOM_OPERATION_UNIT_SET_MADEATK:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: New value.
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->setMadeAttack(iData2 > 0);
		}
		break;
	case CUSTOM_OPERATION_UNIT_SET_EXPERIENCE:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: New value. iData3: iMax
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->setExperienceTimes100(iData2, iData3);
		}
		break;
	/*case CUSTOM_OPERATION_UNIT_SET_NAME:
		//ePlayer: Owner of the unit
		//iData1: Unit ID.
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->setName(customMsg);
		}
		break;*/
	case CUSTOM_OPERATION_UNIT_SET_EMBARKED:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: New value.
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->setEmbarked(iData2);
		}
		break;
	case CUSTOM_OPERATION_UNIT_CHANGE_DAMAGE:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: Change value. iData3: operater. iData4: iUnit.
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->changeDamage(iData2, PlayerTypes(iData3), iData4);
		}
		break;
	case CUSTOM_OPERATION_UNIT_CHANGE_EXP:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: Change value. iData3: iMax. iData4: option integers.
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->changeExperienceTimes100(iData2, iData3, iData4 & 1, iData4 & (1 << 1), iData4 & (1 << 2));
		}
		break;
	case CUSTOM_OPERATION_UNIT_JUMP_VALID_PLOT:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: time.
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->jumpToNearestValidPlot();
		}
		break;
	case CUSTOM_OPERATION_UNIT_DO_COMMAND:
		//ePlayer: Owner of the unit
		//iData1: Unit ID. iData2: command type. iData3: data1. iData4: data2
		unit = GET_PLAYER(ePlayer).getUnit(iData1);
		if (unit != NULL) {
			unit->doCommand((CommandTypes)iData2, iData3, iData4);
		}
		break;

	case CUSTOM_OPERATION_PLAYER_INIT_UNIT:
		//ePlayer: The player to give unit
		//iData1: Unit type. iData2: X. iData3: Y. iData4: Unit AI. iData5: Direction. iData6: ID of return value.
		//if the message is sent from local player, execute before sending network message.
		GET_PLAYER(ePlayer).initUnit((UnitTypes)iData1, iData2, iData3, (UnitAITypes)iData4, (DirectionTypes)iData5);
		break;
	case CUSTOM_OPERATION_PLAYER_SET_HAS_POLICY:
		//ePlayer: The player to give policy
		//iData1: Policy index. iData2: bValue. iData3: bFree.
		GET_PLAYER(ePlayer).setHasPolicy(PolicyTypes(iData1), iData2 > 0, iData3 > 0);
		break;
	case CUSTOM_OPERATION_PLAYER_SET_JONSCULTURE:
		//ePlayer: The player to set culture
		//iData1: New value.
		GET_PLAYER(ePlayer).setJONSCulture(iData1);
		break;
	case CUSTOM_OPERATION_PLAYER_SET_ANARCHY:
		//ePlayer: The player to set culture
		//iData1: Anarchy turns.
		GET_PLAYER(ePlayer).SetAnarchyNumTurns(iData1);
		break;
	case CUSTOM_OPERATION_PLAYER_CHANGE_NUM_RES:
		//ePlayer: The player to change res
		//iData1: Resourse ID. iData2: change value. iData3: include import.
		GET_PLAYER(ePlayer).changeNumResourceTotal((ResourceTypes)iData1, iData2, iData3 > 0);
		break;
	case CUSTOM_OPERATION_PLAYER_SET_NUM_FREETECH:
		//ePlayer: The player to change res
		//iData1: Change num.
		GET_PLAYER(ePlayer).SetNumFreeTechs(iData1);
		break;

	case CUSTOM_OPERATION_PLAYER_TREASURY_CHANGE_GOLD:
		//ePlayer: The player to change res
		//iData1: Change num.
		GET_PLAYER(ePlayer).GetTreasury()->ChangeGold(iData1);
		break;

	case CUSTOM_OPERATION_CITY_SET_DAMAGE:
		//ePlayer: Owner of the city
		//iData1: City ID. iData2: New value. iData3: No message.
		city = GET_PLAYER(ePlayer).getCity(iData1);
		if (city != NULL) {
			city->setDamage(iData2, iData3 > 0);
		}
		break;
	case CUSTOM_OPERATION_CITY_SET_PUPPET:
		//ePlayer: Owner of the city
		//iData1: City ID. iData2: New value
		city = GET_PLAYER(ePlayer).getCity(iData1);
		if (city != NULL) {
			city->SetPuppet(iData2 > 0);
		}
		break;
	case CUSTOM_OPERATION_CITY_SET_OCCUPIED:
		//ePlayer: Owner of the city
		//iData1: City ID. iData2: New value
		city = GET_PLAYER(ePlayer).getCity(iData1);
		if (city != NULL) {
			city->SetOccupied(iData2 > 0);
		}
		break;
	case CUSTOM_OPERATION_CITY_SET_POPULATION:
		//ePlayer: Owner of the city
		//iData1: City ID. iData2: New value. iData3: bReassignPop.
		city = GET_PLAYER(ePlayer).getCity(iData1);
		if (city != NULL) {
			city->setPopulation(iData2, iData3);
		}
		break;
	case CUSTOM_OPERATION_CITY_SET_FOOD:
		//ePlayer: Owner of the city
		//iData1: City ID. iData2: New value.
		city = GET_PLAYER(ePlayer).getCity(iData1);
		if (city != NULL) {
			city->setFood(iData2);
		}
		break;
	case CUSTOM_OPERATION_CITY_CHANGE_RESIST:
		//ePlayer: Owner of the city
		//iData1: City ID. iData2: New value.
		city = GET_PLAYER(ePlayer).getCity(iData1);
		if (city != NULL) {
			city->ChangeResistanceTurns(iData2);
		}
		break;
	case CUSTOM_OPERATION_CITY_CHANGE_POPULATION:
		//ePlayer: Owner of the city
		//iData1: City ID. iData2: Change value. iData3: Reassign pop.
		city = GET_PLAYER(ePlayer).getCity(iData1);
		if (city != NULL) {
			city->changePopulation(iData2, iData3);
		}
		break;

	case CUSTOM_OPERATION_CITYBUILDING_SET_NUM_BUILDING:
		//ePlayer: Owner of the city
		//iData1: City ID. iData2: Building type. iData3: New value.
		city = GET_PLAYER(ePlayer).getCity(iData1);
		if (city != NULL) {
			city->GetCityBuildings()->SetNumRealBuilding((BuildingTypes)iData2, iData3);
		}
		break;

	case CUSTOM_OPERATION_PLOT_SET_IMPRVTYPE:
		//ePlayer: Player built the improvement
		//iData1: Plot X. iData2: Plot Y. iData1: Improvement ID. 
		plot = GC.getMap().plot(iData1, iData2);
		if (plot != NULL) {
			plot->setImprovementType((ImprovementTypes)iData3, ePlayer);
		}
		break;
	case CUSTOM_OPERATION_PLOT_SET_RESOURCE:
		//ePlayer: None
		//iData1: Resource ID. iData2: Num. iData3:For minor CIV iData4: Plot X. iData5: Plot Y.
		plot = GC.getMap().plot(iData4, iData5);
		if (plot != NULL) {
			plot->setResourceType(ResourceTypes(iData1), iData2, iData3);
		}
		break;
	case CUSTOM_OPERATION_PLOT_SET_ROUTE:
		//ePlayer: None
		//iData1: New value. iData2: X. iData3: Y.
		plot = GC.getMap().plot(iData2, iData3);
		if (plot != NULL) {
			plot->setRouteType((RouteTypes)iData1);
		}
		break;
	case CUSTOM_OPERATION_PLOT_SET_FEATURE:
		//ePlayer: None
		//iData1: Type. iData2: New value. iData3: X. iData4: Y.
		plot = GC.getMap().plot(iData3, iData4);
		if (plot != NULL) {
			plot->setFeatureType(FeatureTypes(iData1), iData2);
		}
		break;
	case CUSTOM_OPERATION_PLOT_CHANGE_BUILD_PROGRESS:
		//ePlayer: Done player
		//iData1: Type. iData2: New value. iData3: X. iData4: Y
		plot = GC.getMap().plot(iData3, iData4);
		if (plot != NULL) {
			plot->changeBuildProgress((BuildTypes)iData1, iData2, ePlayer);
		}
		break;
	case CUSTOM_OPERATION_PLOT_CHANGE_NUM_RESOURCE:
		//ePlayer: Done player
		//iData1: newValue. iData2: X. iData3: Y
		plot = GC.getMap().plot(iData2, iData3);
		if (plot != NULL) {
			plot->changeNumResource(iData1);
		}
		break;

	case CUSTOM_OPERATION_GAME_PLOT_EXTRA_YIELD:
		GC.getGame().setPlotExtraYield(iData1, iData2, YieldTypes(iData3), iData4);
		break;
	default:
		break;
	
	}
	
}

//------------------------------------------------------------------------------
// Use this method for customized operations.
void CvDllNetMessageHandler::ResponseFoundReligion(PlayerTypes ePlayer, ReligionTypes eReligion, const char* szCustomName, BeliefTypes eBelief1, BeliefTypes eBelief2, BeliefTypes eBelief3, BeliefTypes eBelief4, int iCityX, int iCityY)
{
	if ((UINT16(eReligion >> 16) > 0)) {
		TransmissCustomizedOperationFromResponseFoundReligion(ePlayer, eBelief1, eBelief2, eBelief3, eBelief4, iCityX, iCityY, eReligion, szCustomName);
		return;
	}

	CvGame& kGame(GC.getGame());
	CvGameReligions* pkGameReligions(kGame.GetGameReligions());

	CvCity* pkCity = GC.getMap().plot(iCityX, iCityY)->getPlotCity();
	if(pkCity && ePlayer != NO_PLAYER)
	{
		CvGameReligions::FOUNDING_RESULT eResult = pkGameReligions->CanFoundReligion(ePlayer, eReligion, szCustomName, eBelief1, eBelief2, eBelief3, eBelief4, pkCity);
		if(eResult == CvGameReligions::FOUNDING_OK)
			pkGameReligions->FoundReligion(ePlayer, eReligion, szCustomName, eBelief1, eBelief2, eBelief3, eBelief4, pkCity);
		else
		{
			CvGameReligions::NotifyPlayer(ePlayer, eResult);
			// We don't want them to lose the opportunity to found the religion, and the Great Prophet is already gone so just repost the notification
			// If someone beat them to the last religion, well... tough luck.
			CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
			if(kPlayer.isHuman() && eResult != CvGameReligions::FOUNDING_NO_RELIGIONS_AVAILABLE)
			{
				CvNotifications* pNotifications = kPlayer.GetNotifications();
				if(pNotifications)
				{
					CvString strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_FOUND_RELIGION");
					CvString strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_FOUND_RELIGION");
#if defined(MOD_API_EXTENSIONS)
					pNotifications->Add(NOTIFICATION_FOUND_RELIGION, strBuffer, strSummary, iCityX, iCityY, eReligion, pkCity->GetID());
#else
					pNotifications->Add(NOTIFICATION_FOUND_RELIGION, strBuffer, strSummary, iCityX, iCityY, -1, pkCity->GetID());
#endif
				}
				kPlayer.GetReligions()->SetFoundingReligion(true);
			}
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseEnhanceReligion(PlayerTypes ePlayer, ReligionTypes eReligion, const char* szCustomName, BeliefTypes eBelief1, BeliefTypes eBelief2, int iCityX, int iCityY)
{
	CvGame& kGame(GC.getGame());
	CvGameReligions* pkGameReligions(kGame.GetGameReligions());

	CvGameReligions::FOUNDING_RESULT eResult = pkGameReligions->CanEnhanceReligion(ePlayer, eReligion, eBelief1, eBelief2);
	if(eResult == CvGameReligions::FOUNDING_OK)
		pkGameReligions->EnhanceReligion(ePlayer, eReligion, eBelief1, eBelief2);
	else
	{
		CvGameReligions::NotifyPlayer(ePlayer, eResult);
		// We don't want them to lose the opportunity to enhance the religion, and the Great Prophet is already gone so just repost the notification
		CvCity* pkCity = GC.getMap().plot(iCityX, iCityY)->getPlotCity();
		CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
		if(kPlayer.isHuman() && eResult != CvGameReligions::FOUNDING_NO_RELIGIONS_AVAILABLE && pkCity)
		{
			CvNotifications* pNotifications = kPlayer.GetNotifications();
			if(pNotifications)
			{
				CvString strBuffer = GetLocalizedText("TXT_KEY_NOTIFICATION_ENHANCE_RELIGION");
				CvString strSummary = GetLocalizedText("TXT_KEY_NOTIFICATION_SUMMARY_ENHANCE_RELIGION");
#if defined(MOD_API_EXTENSIONS)
				pNotifications->Add(NOTIFICATION_ENHANCE_RELIGION, strBuffer, strSummary, iCityX, iCityY, eReligion, pkCity->GetID());
#else
				pNotifications->Add(NOTIFICATION_ENHANCE_RELIGION, strBuffer, strSummary, iCityX, iCityY, -1, pkCity->GetID());
#endif
			}
			kPlayer.GetReligions()->SetFoundingReligion(true);
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseMoveSpy(PlayerTypes ePlayer, int iSpyIndex, int iTargetPlayer, int iTargetCity, bool bAsDiplomat)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvPlayerEspionage* pPlayerEspionage = kPlayer.GetEspionage();

	if(pPlayerEspionage)
	{
		if(iTargetCity == -1)
		{
			pPlayerEspionage->ExtractSpyFromCity(iSpyIndex);
			GC.GetEngineUserInterface()->setDirty(EspionageScreen_DIRTY_BIT, true);
		}
		else
		{
			CvAssertMsg(iTargetPlayer != -1, "iTargetPlayer is -1");
			if(iTargetPlayer != -1)
			{
				PlayerTypes eTargetPlayer = (PlayerTypes)iTargetPlayer;
				CvCity* pCity = GET_PLAYER(eTargetPlayer).getCity(iTargetCity);
				CvAssertMsg(pCity, "pCity is null");
				if(pCity)
				{
					pPlayerEspionage->MoveSpyTo(pCity, iSpyIndex, bAsDiplomat);
					GC.GetEngineUserInterface()->setDirty(EspionageScreen_DIRTY_BIT, true);
				}
			}
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseStageCoup(PlayerTypes eSpyPlayer, int iSpyIndex)
{
	CvAssertMsg(eSpyPlayer != NO_PLAYER, "eSpyPlayer invalid");
	CvAssertMsg(iSpyIndex >= 0, "iSpyIndex invalid");

	CvPlayerAI& kPlayer = GET_PLAYER(eSpyPlayer);
	CvPlayerEspionage* pPlayerEspionage = kPlayer.GetEspionage();

	CvAssertMsg(pPlayerEspionage, "pPlayerEspionage is null");
	if(pPlayerEspionage)
	{
		bool bCoupSuccess = pPlayerEspionage->AttemptCoup(iSpyIndex);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseFaithPurchase(PlayerTypes ePlayer, FaithPurchaseTypes eFaithPurchaseType, int iFaithPurchaseIndex)
{
	CvAssertMsg(ePlayer != NO_PLAYER, "ePlayer invalid");
	CvAssertMsg(eFaithPurchaseType > -1, "Faith Purchase Type invalid");
	CvAssertMsg(iFaithPurchaseIndex > -1, "Faith Purchase Index invalid");

	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	kPlayer.SetFaithPurchaseType(eFaithPurchaseType);
	kPlayer.SetFaithPurchaseIndex(iFaithPurchaseIndex);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseLeagueVoteEnact(LeagueTypes eLeague, int iResolutionID, PlayerTypes eVoter, int iNumVotes, int iChoice)
{
	CvAssertMsg(eLeague != NO_LEAGUE, "eLeague invalid");
	CvAssertMsg(eVoter != NO_PLAYER, "eVoter invalid");

	CvLeague* pLeague = GC.getGame().GetGameLeagues()->GetLeague(eLeague);
	CvAssertMsg(pLeague->CanVote(eVoter), "eVoter not allowed to vote. Please send Anton your save file and version.");
	pLeague->DoVoteEnact(iResolutionID, eVoter, iNumVotes, iChoice);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseLeagueVoteRepeal(LeagueTypes eLeague, int iResolutionID, PlayerTypes eVoter, int iNumVotes, int iChoice)
{
	CvAssertMsg(eLeague != NO_LEAGUE, "eLeague invalid");
	CvAssertMsg(eVoter != NO_PLAYER, "eVoter invalid");

	CvLeague* pLeague = GC.getGame().GetGameLeagues()->GetLeague(eLeague);
	CvAssertMsg(pLeague->CanVote(eVoter), "eVoter not allowed to vote. Please send Anton your save file and version.");
	pLeague->DoVoteRepeal(iResolutionID, eVoter, iNumVotes, iChoice);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseLeagueVoteAbstain(LeagueTypes eLeague, PlayerTypes eVoter, int iNumVotes)
{
	CvAssertMsg(eLeague != NO_LEAGUE, "eLeague invalid");
	CvAssertMsg(eVoter != NO_PLAYER, "eVoter invalid");

	CvLeague* pLeague = GC.getGame().GetGameLeagues()->GetLeague(eLeague);
	CvAssertMsg(pLeague->CanVote(eVoter), "eVoter not allowed to vote. Please send Anton your save file and version.");
	pLeague->DoVoteAbstain(eVoter, iNumVotes);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseLeagueProposeEnact(LeagueTypes eLeague, ResolutionTypes eResolution, PlayerTypes eProposer, int iChoice)
{
	CvAssertMsg(eLeague != NO_LEAGUE, "eLeague invalid");
	CvAssertMsg(eResolution != NO_RESOLUTION, "eResolution invalid");
	CvAssertMsg(eProposer != NO_PLAYER, "eProposer invalid");

	CvLeague* pLeague = GC.getGame().GetGameLeagues()->GetLeague(eLeague);
	CvAssertMsg(pLeague->CanProposeEnact(eResolution, eProposer, iChoice), "eProposer not allowed to enact Resolution. Please send Anton your save file and version.");
	pLeague->DoProposeEnact(eResolution, eProposer, iChoice);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseLeagueProposeRepeal(LeagueTypes eLeague, int iResolutionID, PlayerTypes eProposer)
{
	CvAssertMsg(eLeague != NO_LEAGUE, "eLeague invalid");
	CvAssertMsg(eProposer != NO_PLAYER, "eProposer invalid");

	CvLeague* pLeague = GC.getGame().GetGameLeagues()->GetLeague(eLeague);
	CvAssertMsg(pLeague->CanProposeRepeal(iResolutionID, eProposer), "eProposer not allowed to repeal Resolution. Please send Anton your save file and version.");
	pLeague->DoProposeRepeal(iResolutionID, eProposer);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseLeagueEditName(LeagueTypes eLeague, PlayerTypes ePlayer, const char* szCustomName)
{
	CvAssertMsg(eLeague != NO_LEAGUE, "eLeague invalid");

	CvLeague* pLeague = GC.getGame().GetGameLeagues()->GetLeague(eLeague);
	pLeague->DoChangeCustomName(ePlayer, szCustomName);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseSetSwappableGreatWork(PlayerTypes ePlayer, int iWorkClass, int iWorkIndex)
{
	CvAssertMsg(ePlayer != NO_PLAYER, "ePlayer invalid");
	
	// is this player alive
	if (GET_PLAYER(ePlayer).isAlive())
	{
		// -1 indicates that they want to clear the slot
		if (iWorkIndex == -1)
		{
			if (iWorkClass == GC.getInfoTypeForString("GREAT_WORK_ARTIFACT"))
			{
				GET_PLAYER(ePlayer).GetCulture()->SetSwappableArtifactIndex(-1);
			}
			else if (iWorkClass == GC.getInfoTypeForString("GREAT_WORK_ART"))
			{
				GET_PLAYER(ePlayer).GetCulture()->SetSwappableArtIndex(-1);
			}
			else if (iWorkClass == GC.getInfoTypeForString("GREAT_WORK_LITERATURE"))
			{
				GET_PLAYER(ePlayer).GetCulture()->SetSwappableWritingIndex(-1);
			}			
			else if (iWorkClass == GC.getInfoTypeForString("GREAT_WORK_MUSIC"))
			{
				GET_PLAYER(ePlayer).GetCulture()->SetSwappableMusicIndex(-1);
			}
		}
		else
		{
			// does this player control this work
			if (GET_PLAYER(ePlayer).GetCulture()->ControlsGreatWork(iWorkIndex))
			{
				if (iWorkClass == GC.getInfoTypeForString("GREAT_WORK_ARTIFACT"))
				{
					GET_PLAYER(ePlayer).GetCulture()->SetSwappableArtifactIndex(iWorkIndex);
				}
				else if (iWorkClass == GC.getInfoTypeForString("GREAT_WORK_ART"))
				{
					GET_PLAYER(ePlayer).GetCulture()->SetSwappableArtIndex(iWorkIndex);
				}
				else if (iWorkClass == GC.getInfoTypeForString("GREAT_WORK_LITERATURE"))
				{
					GET_PLAYER(ePlayer).GetCulture()->SetSwappableWritingIndex(iWorkIndex);
				}			
				else if (iWorkClass == GC.getInfoTypeForString("GREAT_WORK_MUSIC"))
				{
					GET_PLAYER(ePlayer).GetCulture()->SetSwappableMusicIndex(iWorkIndex);
				}				
			}
		}
		GC.GetEngineUserInterface()->setDirty(GreatWorksScreen_DIRTY_BIT, true);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseSwapGreatWorks(PlayerTypes ePlayer1, int iWorkIndex1, PlayerTypes ePlayer2, int iWorkIndex2)
{
	GC.getGame().GetGameCulture()->SwapGreatWorks(ePlayer1, iWorkIndex1, ePlayer2, iWorkIndex2);
}

//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseMoveGreatWorks(PlayerTypes ePlayer, int iCity1, int iBuildingClass1, int iWorkIndex1, 
																																				 int iCity2, int iBuildingClass2, int iWorkIndex2)
{
	GC.getGame().GetGameCulture()->MoveGreatWorks(ePlayer, iCity1, iBuildingClass1, iWorkIndex1, iCity2, iBuildingClass2, iWorkIndex2);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseChangeIdeology(PlayerTypes ePlayer)
{
	CvAssertMsg(ePlayer != NO_PLAYER, "ePlayer invalid");

	// is this player alive
	CvPlayer &kPlayer = GET_PLAYER(ePlayer);
	if (kPlayer.isAlive())
	{
		PolicyBranchTypes ePreferredIdeology = kPlayer.GetCulture()->GetPublicOpinionPreferredIdeology();
		kPlayer.SetAnarchyNumTurns(GC.getSWITCH_POLICY_BRANCHES_ANARCHY_TURNS());
		kPlayer.GetPlayerPolicies()->DoSwitchIdeologies(ePreferredIdeology);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseGiftUnit(PlayerTypes ePlayer, PlayerTypes eMinor, int iUnitID)
{
	CvUnit* pkUnit = GET_PLAYER(ePlayer).getUnit(iUnitID);
	GET_PLAYER(eMinor).DoDistanceGift(ePlayer, pkUnit);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseLaunchSpaceship(PlayerTypes ePlayer, VictoryTypes eVictory)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvTeam& kTeam = GET_TEAM(kPlayer.getTeam());

	if(kTeam.canLaunch(eVictory))
	{
		kPlayer.launch(eVictory);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseLiberatePlayer(PlayerTypes ePlayer, PlayerTypes eLiberatedPlayer, int iCityID)
{
	GET_PLAYER(ePlayer).DoLiberatePlayer(eLiberatedPlayer, iCityID);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseMinorCivBullyGold(PlayerTypes ePlayer, PlayerTypes eMinor, int iGold)
{
	GET_PLAYER(eMinor).GetMinorCivAI()->DoMajorBullyGold(ePlayer, iGold);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseMinorCivBullyUnit(PlayerTypes ePlayer, PlayerTypes eMinor, UnitTypes eUnitType)
{
	GET_PLAYER(eMinor).GetMinorCivAI()->DoMajorBullyUnit(ePlayer, eUnitType);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseMinorCivGiftGold(PlayerTypes ePlayer, PlayerTypes eMinor, int iGold)
{
	// Enough Gold?
	if(GET_PLAYER(ePlayer).GetTreasury()->GetGold() >= iGold)
	{
		GET_PLAYER(eMinor).GetMinorCivAI()->DoGoldGiftFromMajor(ePlayer, iGold);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseMinorCivGiftTileImprovement(PlayerTypes eMajor, PlayerTypes eMinor, int iPlotX, int iPlotY)
{
	GET_PLAYER(eMinor).GetMinorCivAI()->DoTileImprovementGiftFromMajor(eMajor, iPlotX, iPlotY);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseMinorCivBuyout(PlayerTypes eMajor, PlayerTypes eMinor)
{
#if defined(MOD_GLOBAL_CS_MARRIAGE_KEEPS_RESOURCES)
	GET_PLAYER(eMinor).GetMinorCivAI()->DoBuyout(eMajor, MOD_GLOBAL_CS_MARRIAGE_KEEPS_RESOURCES);
#else
	GET_PLAYER(eMinor).GetMinorCivAI()->DoBuyout(eMajor);
#endif
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseMinorNoUnitSpawning(PlayerTypes ePlayer, PlayerTypes eMinor, bool bValue)
{
	GET_PLAYER(eMinor).GetMinorCivAI()->SetUnitSpawningDisabled(ePlayer, bValue);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponsePlayerDealFinalized(PlayerTypes eFromPlayer, PlayerTypes eToPlayer, PlayerTypes eActBy, bool bAccepted)
{
	CvGame& game = GC.getGame();
	PlayerTypes eActivePlayer = game.getActivePlayer();

	// is the deal valid?
#if defined(MOD_AI_MP_DIPLOMACY)
	if(!game.GetGameDeals()->FinalizeDeal(eFromPlayer, eToPlayer, bAccepted, true))
#else
	if(!game.GetGameDeals()->FinalizeDeal(eFromPlayer, eToPlayer, bAccepted))
#endif
	{
		Localization::String strMessage;
		Localization::String strSummary = Localization::Lookup("TXT_KEY_DEAL_EXPIRED");

		CvPlayerAI& kToPlayer = GET_PLAYER(eToPlayer);
		CvPlayerAI& kFromPlayer = GET_PLAYER(eFromPlayer);
		CvPlayerAI& kActivePlayer = GET_PLAYER(eActivePlayer);

		strMessage = Localization::Lookup("TXT_KEY_DEAL_EXPIRED_FROM_YOU");
		strMessage << kToPlayer.getNickName();
		kFromPlayer.GetNotifications()->Add(NOTIFICATION_PLAYER_DEAL_RESOLVED, strMessage.toUTF8(), strSummary.toUTF8(), eToPlayer, -1, -1);

		strMessage = Localization::Lookup("TXT_KEY_DEAL_EXPIRED_FROM_THEM");
		strMessage << kFromPlayer.getNickName();
		kToPlayer.GetNotifications()->Add(NOTIFICATION_PLAYER_DEAL_RESOLVED, strMessage.toUTF8(), strSummary.toUTF8(), eFromPlayer, -1, -1);
	}
	else
	{
		CvPlayerAI& kToPlayer = GET_PLAYER(eToPlayer);
		CvPlayerAI& kFromPlayer = GET_PLAYER(eFromPlayer);
		if(bAccepted)
		{
			Localization::String strSummary = Localization::Lookup("TXT_KEY_DEAL_ACCEPTED");
			Localization::String strMessage = Localization::Lookup("TXT_KEY_DEAL_ACCEPTED_BY_THEM");
			strMessage << kToPlayer.getNickName();
			kFromPlayer.GetNotifications()->Add(NOTIFICATION_PLAYER_DEAL_RESOLVED, strMessage.toUTF8(), strSummary.toUTF8(), eToPlayer, -1, -1);

			strSummary = Localization::Lookup("TXT_KEY_DEAL_ACCEPTED");
			strMessage = Localization::Lookup("TXT_KEY_DEAL_ACCEPTED_BY_YOU");
			strMessage << kFromPlayer.getNickName();
			kToPlayer.GetNotifications()->Add(NOTIFICATION_PLAYER_DEAL_RESOLVED, strMessage.toUTF8(), strSummary.toUTF8(), eFromPlayer, -1, -1);
		}
		else
		{
			if(eActBy == eFromPlayer)
			{
				Localization::String strSummary = Localization::Lookup("TXT_KEY_DEAL_WITHDRAWN");
				Localization::String strMessage = Localization::Lookup("TXT_KEY_DEAL_WITHDRAWN_BY_YOU");
				strMessage << kToPlayer.getNickName();
				kFromPlayer.GetNotifications()->Add(NOTIFICATION_PLAYER_DEAL_RESOLVED, strMessage.toUTF8(), strSummary.toUTF8(), eToPlayer, -1, -1);

				strSummary = Localization::Lookup("TXT_KEY_DEAL_WITHDRAWN");
				strMessage = Localization::Lookup("TXT_KEY_DEAL_WITHDRAWN_BY_THEM");
				strMessage << kFromPlayer.getNickName();
				kToPlayer.GetNotifications()->Add(NOTIFICATION_PLAYER_DEAL_RESOLVED, strMessage.toUTF8(), strSummary.toUTF8(), eFromPlayer, -1, -1);
			}
			else
			{
				Localization::String strSummary = Localization::Lookup("TXT_KEY_DEAL_REJECTED");
				Localization::String strMessage = Localization::Lookup("TXT_KEY_DEAL_REJECTED_BY_THEM");
				strMessage << kToPlayer.getNickName();
				kFromPlayer.GetNotifications()->Add(NOTIFICATION_PLAYER_DEAL_RESOLVED, strMessage.toUTF8(), strSummary.toUTF8(), eToPlayer, -1, -1);

				strSummary = Localization::Lookup("TXT_KEY_DEAL_REJECTED");
				strMessage = Localization::Lookup("TXT_KEY_DEAL_REJECTED_BY_YOU");
				strMessage << kFromPlayer.getNickName();
				kToPlayer.GetNotifications()->Add(NOTIFICATION_PLAYER_DEAL_RESOLVED, strMessage.toUTF8(), strSummary.toUTF8(), eFromPlayer, -1, -1);
			}
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponsePlayerOption(PlayerTypes ePlayer, PlayerOptionTypes eOption, bool bValue)
{
	GET_PLAYER(ePlayer).setOption(eOption, bValue);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponsePledgeMinorProtection(PlayerTypes ePlayer, PlayerTypes eMinor, bool bValue, bool bPledgeNowBroken)
{
	GET_PLAYER(eMinor).GetMinorCivAI()->DoChangeProtectionFromMajor(ePlayer, bValue, bPledgeNowBroken);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponsePushMission(PlayerTypes ePlayer, int iUnitID, MissionTypes eMission, int iData1, int iData2, int iFlags, bool bShift)
{
	CvUnit::dispatchingNetMessage(true);

	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvUnit* pkUnit = kPlayer.getUnit(iUnitID);

	if(pkUnit != NULL)
	{
		pkUnit->PushMission(eMission, iData1, iData2, iFlags, bShift, true);
	}

	CvUnit::dispatchingNetMessage(false);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseGreatPersonChoice(PlayerTypes ePlayer, UnitTypes eGreatPersonUnit)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pCity = kPlayer.GetGreatPersonSpawnCity(eGreatPersonUnit);
	if(pCity)
	{
#if defined(MOD_GLOBAL_TRULY_FREE_GP)
		pCity->GetCityCitizens()->DoSpawnGreatPerson(eGreatPersonUnit, true, false, MOD_GLOBAL_TRULY_FREE_GP);
#else
		pCity->GetCityCitizens()->DoSpawnGreatPerson(eGreatPersonUnit, true, false);
#endif
	}
	kPlayer.ChangeNumFreeGreatPeople(-1);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseMayaBonusChoice(PlayerTypes ePlayer, UnitTypes eGreatPersonUnit)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pCity = kPlayer.GetGreatPersonSpawnCity(eGreatPersonUnit);
	if(pCity)
	{
#if defined(MOD_GLOBAL_TRULY_FREE_GP)
		pCity->GetCityCitizens()->DoSpawnGreatPerson(eGreatPersonUnit, true, false, MOD_GLOBAL_TRULY_FREE_GP);
#else
		pCity->GetCityCitizens()->DoSpawnGreatPerson(eGreatPersonUnit, true, false);
#endif
	}
	kPlayer.ChangeNumMayaBoosts(-1);
	kPlayer.GetPlayerTraits()->SetUnitBaktun(eGreatPersonUnit);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseFaithGreatPersonChoice(PlayerTypes ePlayer, UnitTypes eGreatPersonUnit)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pCity = kPlayer.GetGreatPersonSpawnCity(eGreatPersonUnit);
	if(pCity)
	{
#if defined(MOD_GLOBAL_TRULY_FREE_GP)
		pCity->GetCityCitizens()->DoSpawnGreatPerson(eGreatPersonUnit, true, true, false);
#else
		pCity->GetCityCitizens()->DoSpawnGreatPerson(eGreatPersonUnit, true, true);
#endif
	}
	kPlayer.ChangeNumFaithGreatPeople(-1);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseGoodyChoice(PlayerTypes ePlayer, int iPlotX, int iPlotY, GoodyTypes eGoody, int iUnitID)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvPlot* pPlot = GC.getMap().plot(iPlotX, iPlotY);
	CvUnit* pUnit = kPlayer.getUnit(iUnitID);
	kPlayer.receiveGoody(pPlot, eGoody, pUnit);
}

//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseArchaeologyChoice(PlayerTypes ePlayer, ArchaeologyChoiceType eChoice)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	kPlayer.GetCulture()->DoArchaeologyChoice(eChoice);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseIdeologyChoice(PlayerTypes ePlayer, PolicyBranchTypes eChoice)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	kPlayer.GetPlayerPolicies()->SetPolicyBranchUnlocked(eChoice, true, false);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseRenameCity(PlayerTypes ePlayer, int iCityID, const char* szName)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvCity* pkCity = kPlayer.getCity(iCityID);
	NetworkMessageAdapter::StringShiftReverse(NetworkMessageAdapter::ReceiveBuffer, szName, iCityID);
	auto str = std::string(NetworkMessageAdapter::ReceiveBuffer, iCityID);
	LargeArgContainer cont;
	cont.ParseFromString(str);
	if(pkCity)
	{
		CvString strName = szName;
		pkCity->setName(strName);
	}
	cont.Clear();
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseRenameUnit(PlayerTypes ePlayer, int iUnitID, const char* szName)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvUnit* pkUnit = kPlayer.getUnit(iUnitID);
	if(pkUnit)
	{
		CvString strName = szName;
		pkUnit->setName(strName);
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseResearch(PlayerTypes ePlayer, TechTypes eTech, int iDiscover, bool bShift)
{
	ResponseResearch(ePlayer, eTech, iDiscover, NO_PLAYER, bShift);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseResearch(PlayerTypes ePlayer, TechTypes eTech, int iDiscover, PlayerTypes ePlayerToStealFrom, bool bShift)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvTeam& kTeam = GET_TEAM(kPlayer.getTeam());

	// Free tech
	if(iDiscover > 0)
	{
		// Make sure we can research this tech for free
		if(kPlayer.GetPlayerTechs()->CanResearchForFree(eTech))
		{
			kTeam.setHasTech(eTech, true, ePlayer, true, true);

			if(iDiscover > 1)
			{
				if(ePlayer == GC.getGame().getActivePlayer())
				{
					kPlayer.chooseTech(iDiscover - 1);
				}
			}
			kPlayer.SetNumFreeTechs(max(0, iDiscover - 1));
		}
	}
	// Stealing tech
	else if(ePlayerToStealFrom != NO_PLAYER)
	{
		// make sure we can still take a tech
		CvAssertMsg(kPlayer.GetEspionage()->m_aiNumTechsToStealList[ePlayerToStealFrom] > 0, "No techs to steal from player");
		CvAssertMsg(kPlayer.GetEspionage()->m_aaPlayerStealableTechList[ePlayerToStealFrom].size() > 0, "No techs to be stolen from this player");
		CvAssertMsg(kPlayer.GetPlayerTechs()->CanResearch(eTech), "Player can't research this technology");
		CvAssertMsg(GET_TEAM(GET_PLAYER(ePlayerToStealFrom).getTeam()).GetTeamTechs()->HasTech(eTech), "ePlayerToStealFrom does not have the requested tech");
		if (kPlayer.GetEspionage()->m_aiNumTechsToStealList[ePlayerToStealFrom] > 0)
		{
			kTeam.setHasTech(eTech, true, ePlayer, true, true);
			kPlayer.GetEspionage()->m_aiNumTechsToStealList[ePlayerToStealFrom]--;
		}
	}
	// Normal tech
	else
	{
		CvPlayerTechs* pPlayerTechs = kPlayer.GetPlayerTechs();
		CvTeamTechs* pTeamTechs = kTeam.GetTeamTechs();

		if(eTech == NO_TECH)
		{
			kPlayer.clearResearchQueue();
		}
		else if(pPlayerTechs->CanEverResearch(eTech))
		{
			if((pTeamTechs->HasTech(eTech) || pPlayerTechs->IsResearchingTech(eTech)) && !bShift)
			{
				kPlayer.clearResearchQueue();
			}

			kPlayer.pushResearch(eTech, !bShift);
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseReturnCivilian(PlayerTypes ePlayer, PlayerTypes eToPlayer, int iUnitID, bool bReturn)
{
	GET_PLAYER(ePlayer).DoCivilianReturnLogic(bReturn, eToPlayer, iUnitID);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseSellBuilding(PlayerTypes ePlayer, int iCityID, BuildingTypes eBuilding)
{
	CvCity* pCity = GET_PLAYER(ePlayer).getCity(iCityID);
	if(pCity)
	{
		pCity->GetCityBuildings()->DoSellBuilding(eBuilding);

#if defined(MOD_EVENTS_CITY)
		if (MOD_EVENTS_CITY) {
			GAMEEVENTINVOKE_HOOK(GAMEEVENT_CitySoldBuilding, ePlayer, iCityID, eBuilding);
		} else {
#endif
		ICvEngineScriptSystem1* pkScriptSystem = gDLL->GetScriptSystem();
		if (pkScriptSystem) 
		{
			CvLuaArgsHandle args;
			args->Push(ePlayer);
			args->Push(iCityID);
			args->Push(eBuilding);

			bool bResult;
			LuaSupport::CallHook(pkScriptSystem, "CitySoldBuilding", args.get(), bResult);
		}
#if defined(MOD_EVENTS_CITY)
		}
#endif
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseSetCityAIFocus(PlayerTypes ePlayer, int iCityID, CityAIFocusTypes eFocus)
{
	CvCity* pCity = GET_PLAYER(ePlayer).getCity(iCityID);
	if(pCity != NULL)
	{
		CvCityCitizens* pkCitizens = pCity->GetCityCitizens();
		if(pkCitizens != NULL)
		{
			pkCitizens->SetFocusType(eFocus);
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseSetCityAvoidGrowth(PlayerTypes ePlayer, int iCityID, bool bAvoidGrowth)
{
	CvCity* pCity = GET_PLAYER(ePlayer).getCity(iCityID);
	if(pCity != NULL)
	{
		CvCityCitizens* pkCitizens = pCity->GetCityCitizens();
		if(pkCitizens != NULL)
		{
			pkCitizens->SetForcedAvoidGrowth(bAvoidGrowth);
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseSwapUnits(PlayerTypes ePlayer, int iUnitID, MissionTypes eMission, int iData1, int iData2, int iFlags, bool bShift)
{
	CvUnit::dispatchingNetMessage(true);

	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);
	CvUnit* pkUnit = kPlayer.getUnit(iUnitID);

	if(pkUnit != NULL)
	{
		// Get target plot
		CvMap& kMap = GC.getMap();
		CvPlot* pkTargetPlot = kMap.plot(iData1, iData2);

		if(pkTargetPlot != NULL)
		{
			CvPlot* pkOriginationPlot = pkUnit->plot();

			// Find unit to move out
			for(int iI = 0; iI < pkTargetPlot->getNumUnits(); iI++)
			{
				CvUnit* pkUnit2 = pkTargetPlot->getUnitByIndex(iI);

				if(pkUnit2 && pkUnit2->AreUnitsOfSameType(*pkUnit))
				{
					// Start the swap
					pkUnit->PushMission(CvTypes::getMISSION_MOVE_TO(), iData1, iData2, MOVE_IGNORE_STACKING, bShift, true);

					// Move the other unit back out, again splitting if necessary
					pkUnit2->PushMission(CvTypes::getMISSION_MOVE_TO(), pkOriginationPlot->getX(), pkOriginationPlot->getY());
				}
			}
		}
	}
	CvUnit::dispatchingNetMessage(false);
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseUpdateCityCitizens(PlayerTypes ePlayer, int iCityID)
{
	CvCity* pCity = GET_PLAYER(ePlayer).getCity(iCityID);
	if(NULL != pCity && pCity->GetCityCitizens())
	{
		CvCityCitizens* pkCitizens = pCity->GetCityCitizens();
		if(pkCitizens != NULL)
		{
			pkCitizens->DoVerifyWorkingPlots();
			pkCitizens->DoReallocateCitizens();
		}
	}
}
//------------------------------------------------------------------------------
void CvDllNetMessageHandler::ResponseUpdatePolicies(PlayerTypes ePlayer, bool bNOTPolicyBranch, int iPolicyID, bool bValue)
{
	CvPlayerAI& kPlayer = GET_PLAYER(ePlayer);

	// Policy Update
	if(bNOTPolicyBranch)
	{
		const PolicyTypes ePolicy = static_cast<PolicyTypes>(iPolicyID);
		if(bValue)
		{
			kPlayer.doAdoptPolicy(ePolicy);
		}
		else
		{
			kPlayer.setHasPolicy(ePolicy, bValue);
			kPlayer.DoUpdateHappiness();
		}
	}
	// Policy Branch Update
	else
	{
		const PolicyBranchTypes eBranch = static_cast<PolicyBranchTypes>(iPolicyID);
		CvPlayerPolicies* pPlayerPolicies = kPlayer.GetPlayerPolicies();

		// If Branch was blocked by another branch, then unblock this one - this may be the only thing this NetMessage does
		if(pPlayerPolicies->IsPolicyBranchBlocked(eBranch))
		{
			// Can't switch to a Branch that's still locked. DoUnlockPolicyBranch below will handle this for us
			if(pPlayerPolicies->IsPolicyBranchUnlocked(eBranch))
			{
				//pPlayerPolicies->ChangePolicyBranchBlockedCount(eBranch, -1);
				pPlayerPolicies->DoSwitchToPolicyBranch(eBranch);
			}
		}

		// Unlock the branch if it hasn't been already
		if(!pPlayerPolicies->IsPolicyBranchUnlocked(eBranch))
		{
			pPlayerPolicies->DoUnlockPolicyBranch(eBranch);
		}
	}
}
//------------------------------------------------------------------------------
