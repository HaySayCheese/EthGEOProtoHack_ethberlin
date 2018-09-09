#include "TrustLineConfirmationMessage.h"

TrustLineConfirmationMessage::TrustLineConfirmationMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &senderUUID,
    const TransactionUUID &transactionUUID,
    bool isContractorGateway,
    const OperationState state,
    const string &ethereumAddress,
    const string &ethereumChannelId) :

    ConfirmationMessage(
        equivalent,
        senderUUID,
        transactionUUID,
        state),
    mIsContractorGateway(isContractorGateway),
    mEthereumAddress(ethereumAddress),
    mEthereumChannelId(ethereumChannelId)
{}

TrustLineConfirmationMessage::TrustLineConfirmationMessage(
    BytesShared buffer):

    ConfirmationMessage(buffer)
{
    size_t bytesBufferOffset = ConfirmationMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    memcpy(
        &mIsContractorGateway,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
    bytesBufferOffset += sizeof(byte);

    bool ethereumAddressPresence = false;
    memcpy(
        &ethereumAddressPresence,
        buffer.get() + bytesBufferOffset,
        sizeof(byte));
    bytesBufferOffset += sizeof(byte);

    if (ethereumAddressPresence) {
        string ethereumAddressBuffer(
            (char*)(buffer.get() + bytesBufferOffset),
            kEthereumAddressHexSize);
        mEthereumAddress = ethereumAddressBuffer;
        bytesBufferOffset += kEthereumAddressHexSize;

        string ethereumChannelIdBuffer(
            (char*)(buffer.get() + bytesBufferOffset),
            kEthereumChannelIdHexSize);
        mEthereumChannelId = ethereumChannelIdBuffer;
    } else {
        mEthereumAddress = "";
        mEthereumChannelId = "";
    }
}

const Message::MessageType TrustLineConfirmationMessage::typeID() const
{
    return Message::TrustLines_Confirmation;
}

const bool TrustLineConfirmationMessage::isContractorGateway() const
{
    return mIsContractorGateway;
}

const string& TrustLineConfirmationMessage::ethereumAddress() const
{
    return mEthereumAddress;
}

const string& TrustLineConfirmationMessage::ethereumChannelId() const
{
    return mEthereumChannelId;
}

pair<BytesShared, size_t> TrustLineConfirmationMessage::serializeToBytes() const
    throw (bad_alloc)
{
    auto parentBytesAndCount = ConfirmationMessage::serializeToBytes();

    size_t bytesCount =
            parentBytesAndCount.second
            + sizeof(byte)
            + sizeof(byte)
            + kEthereumAddressHexSize
            + kEthereumChannelIdHexSize;

    BytesShared dataBytesShared = tryMalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        &mIsContractorGateway,
        sizeof(byte));
    dataBytesOffset += sizeof(byte);
    //----------------------------
    if (!mEthereumAddress.empty()) {
        bool ethereumAddressPresence = true;
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &ethereumAddressPresence,
            sizeof(byte));
        dataBytesOffset += sizeof(byte);
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mEthereumAddress.c_str(),
            kEthereumAddressHexSize);
        dataBytesOffset += kEthereumAddressHexSize;
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            mEthereumChannelId.c_str(),
            kEthereumChannelIdHexSize);
    } else {
        bool ethereumAddressPresence = false;
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &ethereumAddressPresence,
            sizeof(byte));
    }
    //----------------------------------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}