#include "InBetweenNodeTopologyMessage.h"

uint8_t InBetweenNodeTopologyMessage::mNodesInPath;

InBetweenNodeTopologyMessage::InBetweenNodeTopologyMessage() {

}

InBetweenNodeTopologyMessage::InBetweenNodeTopologyMessage(
        const CycleType cycleType,
        const TrustLineBalance& maxFlow,
        const byte max_depth,
        vector<NodeUUID> &path) :
    mCycleType(cycleType),
    mMaxDepth(max_depth),
    mMaxFlow(maxFlow),
    mPath(path)
{
    mNodesInPath = (uint8_t) mPath.size();
}

InBetweenNodeTopologyMessage::InBetweenNodeTopologyMessage(
    BytesShared buffer) {

    deserializeFromBytes(buffer);
}

const Message::MessageType InBetweenNodeTopologyMessage::typeID() const {

    return Message::InBetweenNodeTopologyMessage;
}

pair<BytesShared, size_t> InBetweenNodeTopologyMessage::serializeToBytes() {

    vector<byte> MaxFlowBuffer = trustLineBalanceToBytes(mMaxFlow);

    size_t bytesCount = MaxFlowBuffer.size()
                        + sizeof(MessageType)
                        + sizeof(CycleType)
                        + sizeof(mMaxDepth)
                        + sizeof(mNodesInPath)
                        + mNodesInPath * NodeUUID::kBytesSize;
    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;

//    MessageType
    MessageType messageType = (MessageType) typeID();
    memcpy(
            dataBytesShared.get(),
            &messageType,
            sizeof(MessageType)
    );
    dataBytesOffset += sizeof(MessageType);
    // CycleType
    memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &mCycleType,
            sizeof(uint8_t)
    );
    dataBytesOffset += sizeof(uint8_t);
    // for max flow
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        MaxFlowBuffer.data(),
        MaxFlowBuffer.size()
    );
    dataBytesOffset += MaxFlowBuffer.size();
    //----------------------------------------------------
    // for max depth
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mMaxDepth,
        sizeof(mMaxDepth)
    );
    dataBytesOffset += sizeof(mMaxDepth);
    //----------------------------------------------------
    // For path
    // Write vector size first
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mNodesInPath,
        sizeof(mNodesInPath)
    );
    dataBytesOffset += sizeof(mNodesInPath);

    for(auto const& value: mPath) {
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &value,
            NodeUUID::kBytesSize
        );
        dataBytesOffset += NodeUUID::kBytesSize;
    }
    //----------------------------------------------------
    return make_pair(
            dataBytesShared,
            bytesCount
    );
}

void InBetweenNodeTopologyMessage::deserializeFromBytes(
        BytesShared buffer) {

// Message Type
    size_t bytesBufferOffset = 0;
    MessageType *messageType = new (buffer.get()) MessageType;
    bytesBufferOffset += sizeof(uint16_t);
//   Cycle Type
    memcpy(
     &mCycleType,
     buffer.get() + bytesBufferOffset,
     sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(uint8_t);
//   // Max flow
    vector<byte> amountBytes(
            buffer.get() + bytesBufferOffset,
            buffer.get() + bytesBufferOffset + kTrustLineBalanceSerializeBytesCount);


    mMaxFlow = bytesToTrustLineBalance(amountBytes);
    bytesBufferOffset += kTrustLineBalanceSerializeBytesCount;

    // for max depth
    memcpy(
            &mMaxDepth,
            buffer.get() + bytesBufferOffset,
            sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(uint8_t);

    // path
    memcpy(
            &mNodesInPath,
            buffer.get() + bytesBufferOffset,
            sizeof(uint8_t)
    );
    bytesBufferOffset += sizeof(uint8_t);

    for (uint8_t i = 1; i <= mNodesInPath; ++i) {
        NodeUUID stepNode;
        memcpy(
            stepNode.data,
            buffer.get() + bytesBufferOffset,
            NodeUUID::kBytesSize
        );
        bytesBufferOffset += NodeUUID::kBytesSize;
        mPath.push_back(stepNode);
    }
}

const size_t InBetweenNodeTopologyMessage::kOffsetToInheritedBytes() {
    static const size_t offset =
            + kTrustLineBalanceSerializeBytesCount
            + sizeof(mMaxDepth)
//            node in path
            + sizeof(uint8_t)
            + NodeUUID::kBytesSize * mNodesInPath
            + sizeof(MessageType)
            + sizeof(CycleType);
    return offset;
}

const TrustLineBalance& InBetweenNodeTopologyMessage::maxFlow() const {
    return mMaxFlow;
}

const uint8_t InBetweenNodeTopologyMessage::maxDepth() const {
    return mMaxDepth;
}

NodeUUID &InBetweenNodeTopologyMessage::getLastUUIDFromPath() {
    return mPath.back();
}

const InBetweenNodeTopologyMessage::CycleType InBetweenNodeTopologyMessage::cycleType() const {
    return mCycleType;
}

const vector<NodeUUID> InBetweenNodeTopologyMessage::Path() const {
    return mPath;
}