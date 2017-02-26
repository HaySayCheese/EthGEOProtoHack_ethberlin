//
// Created by mc on 17.02.17.
//

#include "MaxFlowCalculationSourceFstLevelOutMessage.h"

MaxFlowCalculationSourceFstLevelOutMessage::MaxFlowCalculationSourceFstLevelOutMessage(
    const NodeUUID& senderUUID,
    const NodeUUID& targetUUID) :

    MaxFlowCalculationMessage(senderUUID, targetUUID) {};

const Message::MessageType MaxFlowCalculationSourceFstLevelOutMessage::typeID() const {

    return Message::MessageTypeID::MaxFlowCalculationSourceFstLevelOutMessageType;
}
