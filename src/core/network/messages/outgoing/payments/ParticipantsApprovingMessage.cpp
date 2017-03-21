﻿#include "ParticipantsApprovingMessage.h"


ParticipantsApprovingMessage::ParticipantsApprovingMessage(
    const NodeUUID& senderUUID,
    const TransactionUUID& transactionUUID) :

    TransactionMessage(
        senderUUID,
        transactionUUID)
{}

ParticipantsApprovingMessage::ParticipantsApprovingMessage(
    BytesShared buffer) :
    TransactionMessage(buffer)
{
    deserializeFromBytes(buffer);
}

/**
 * Inserts participant into participants list.
 * Initialises newly inserted participant with "false" vote.
 */
void ParticipantsApprovingMessage::addParticipant(
    const NodeUUID &participant)
{
    mVotes[participant] = false;
}

/**
 * Returns uuid of the node, that, by the protocol, must receive this message
 * in case if current node approves the operation.
 *
 * @throws NotFoundError in case if current node is last in votes list.
 */
const NodeUUID& ParticipantsApprovingMessage::nextParticipant(
    const NodeUUID& currentNodeUUID) const
{
    const auto kNextNodeUUID = mVotes.find(currenNodeUUID);
    if (kNextNodeUUID == mVotes.end())
        throw NotFoundError(
            "ParticipantsApprovingMessage::nextParticipant: "
            "there are no nodes left in votes list.");

    return kNextNodeUUID;
}

/**
 * Returns uuid of the node, that, by the protocol,
 * must receive this message right after the coordinator.
 *
 * @throws NotFoundError in case if no nodes are present yet.
 */
const NodeUUID&ParticipantsApprovingMessage::firstParticipant() const
{
    if (mVotes.empty())
        throw NotFoundError(
            "ParticipantsApprovingMessage::firstParticipant: "
            "there are no nodes in votes list yet");

    return *(mVotes.cbegin());
}

/**
 * Returns vote of the "participant", if it is present;
 *
 * @throws NotFoundError in case if no vote is present for "participant";
 */
ParticipantsApprovingMessage::Vote ParticipantsApprovingMessage::vote(
    const NodeUUID &participant) const
{
    if (mVotes.count(participant) == 0)
        throw NotFoundError("");

    return mVotes[participant];
}

const Message::MessageType ParticipantsApprovingMessage::typeID() const
{
    return Message::Payments_ParticipantsApprove;
}

/**
 * Message format:
 *  16B - Transaction uuid,
 *  4B  - Total participants count,
 *
 *  { Participant record
 *      16B - Participant 1 UUID,
 *      1B  - Participant 1 vote (true/false),
 *  }
 *
 *  ...
 *
 *  { Participant record
 *      16B - Participant N UUID
 *      1B  - Participant N vote (true/false)
 *  }
 */
pair<BytesShared, size_t> ParticipantsApprovingMessage::serializeToBytes()
{
    const auto kTotalParticipantsRecordsSegmentLength = 4;


    auto parentBytesAndCount = TransactionMessage::serializeToBytes();

    const auto kTotalParticipantsCount = mVotes.size();
    const auto kParticipantRecordSize =
        NodeUUID::kBytesSize
        + sizeof(byte);

    const auto kBufferSize =
        parentBytesAndCount.second
        + kTotalParticipantsRecordsSegmentLength
        + kTotalParticipantsCount * kParticipantRecordSize;

    BytesShared buffer = tryMalloc(kBufferSize);

    // Offsets
    const auto kParentMessageOffset = buffer.get();
    const auto kParticipantsRecordsCountOffset =
        kParentMessageOffset
        + parentBytesAndCount.second;

    const auto kFirstParticipantRecordOffset =
        kParticipantsRecordsCountOffset
        + kTotalParticipantsRecordsSegmentLength;

    // Parent message content
    memcpy(
        kParentMessageOffset,
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);

    // Records count
    memcpy(
        kParticipantsRecordsCountOffset,
        &kTotalParticipantsCount,
        kTotalParticipantsRecordsSegmentLength);

    // Nodes UUID and votes
    auto currentParticipantRecordOffset = kFirstParticipantRecordOffset;
    for (const auto kv : mVotes) {

        // TODO: possible redundant copy
        const auto kParticipantUUID = kv.first;
        memcpy(
            currentParticipantRecordOffset,
            &kParticipantUUID,
            NodeUUID::kBytesSize);

        const auto kVoteOffset = currentParticipantRecordOffset + NodeUUID::kBytesSize;
        const byte kVoteSerialized = kv.second;
        memcpy(
            kVoteOffset,
            &kVoteSerialized,
            sizeof(kVoteSerialized));

        currentParticipantRecordOffset++;
    }

    return make_pair(
        buffer,
        kBufferSize);
}

void ParticipantsApprovingMessage::deserializeFromBytes(
    BytesShared buffer)
{
    const auto kParticipantRecordSize =
        NodeUUID::kBytesSize
        + sizeof(byte);

    const auto kRecordsCountOffset =
        buffer.get()
        + TransactionMessage::kOffsetToInheritedBytes();

    const uint32_t kRecordsCount = *(kRecordsCountOffset);
    auto currentOffset =
        kRecordsCountOffset
        + 4;

    for (uint32_t i=0; i<kRecordsCount; ++i) {
        NodeUUID participantUUID;
        memcpy(
            participantUUID.data,
            currentOffset,
            NodeUUID::kBytesSize);

        const bool kVote =
            *(currentOffset + NodeUUID::kBytesSize);

        mVotes[participantUUID] = kVote;
        currentOffset += kParticipantRecordSize;
    }
}
