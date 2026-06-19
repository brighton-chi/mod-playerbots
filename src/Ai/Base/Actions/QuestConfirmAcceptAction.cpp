#include "QuestConfirmAcceptAction.h"

#include "QuestPackets.h"
#include "WorldPacket.h"

bool QuestConfirmAcceptAction::Execute(Event event)
{
    WorldPacket packet(event.getPacket());
    uint32 questId;
    packet >> questId;

    WorldPackets::Quest::QuestConfirmAcceptClient confirmPacket{WorldPacket(CMSG_QUEST_CONFIRM_ACCEPT)};
    confirmPacket.QuestId = questId;
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest || !bot->CanAddQuest(quest, true))
    {
        return false;
    }
    std::ostringstream out;
    out << "Quest: " << chat->FormatQuest(quest) << " confirm accept";
    botAI->TellMaster(out);
    bot->GetSession()->HandleQuestConfirmAccept(confirmPacket);
    return true;
}
