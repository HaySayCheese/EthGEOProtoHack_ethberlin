#include "SetIncomingTrustLineMessage.h"


SetIncomingTrustLineMessage::SetIncomingTrustLineMessage(
    const SerializedEquivalent equivalent,
    const NodeUUID &sender,
    const TransactionUUID &transactionUUID,
    const NodeUUID &destination,
    const TrustLineAmount &amount,
    const string ethereumAddress)
    noexcept :

    DestinationMessage(
        equivalent,
        sender,
        transactionUUID,
        destination),
    mAmount(amount),
    mEthereumAddress(ethereumAddress)
{}

SetIncomingTrustLineMessage::SetIncomingTrustLineMessage(
    BytesShared buffer)
    noexcept :
    DestinationMessage(buffer)
{
    // todo: use desrializer

    size_t bytesBufferOffset = DestinationMessage::kOffsetToInheritedBytes();
    //----------------------------------------------------
    vector<byte> amountBytes(
        buffer.get() + bytesBufferOffset,
        buffer.get() + bytesBufferOffset + kTrustLineAmountBytesCount);
    mAmount = bytesToTrustLineAmount(amountBytes);
    bytesBufferOffset += kTrustLineAmountBytesCount;

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
    } else {
        mEthereumAddress = "";
    }
}


const Message::MessageType SetIncomingTrustLineMessage::typeID() const
    noexcept
{
    return Message::TrustLines_SetIncoming;
}

const TrustLineAmount &SetIncomingTrustLineMessage::amount() const
    noexcept
{
    return mAmount;
}

const string SetIncomingTrustLineMessage::ethereumAddress() const
{
    return mEthereumAddress;
}

pair<BytesShared, size_t> SetIncomingTrustLineMessage::serializeToBytes() const
{
    // todo: use serializer

    auto parentBytesAndCount = DestinationMessage::serializeToBytes();

    size_t bytesCount = parentBytesAndCount.second
                        + kTrustLineAmountBytesCount
                        + sizeof(byte)
                        + kEthereumAddressHexSize;

    BytesShared dataBytesShared = tryCalloc(bytesCount);
    size_t dataBytesOffset = 0;
    //----------------------------------------------------
    memcpy(
        dataBytesShared.get(),
        parentBytesAndCount.first.get(),
        parentBytesAndCount.second);
    dataBytesOffset += parentBytesAndCount.second;
    //----------------------------------------------------
    vector<byte> buffer = trustLineAmountToBytes(mAmount);
    memcpy(
        dataBytesShared.get() + dataBytesOffset,
        buffer.data(),
        kTrustLineAmountBytesCount);
    dataBytesOffset += kTrustLineAmountBytesCount;
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
    } else {
        bool ethereumAddressPresence = false;
        memcpy(
            dataBytesShared.get() + dataBytesOffset,
            &ethereumAddressPresence,
            sizeof(byte));
    }
    //----------------------------
    return make_pair(
        dataBytesShared,
        bytesCount);
}

const bool SetIncomingTrustLineMessage::isAddToConfirmationRequiredMessagesHandler() const
{
    return true;
}

const bool SetIncomingTrustLineMessage::isCheckCachedResponse() const
{
    return true;
}

const size_t SetIncomingTrustLineMessage::kOffsetToInheritedBytes() const
    noexcept
{
    static const auto kOffset =
            DestinationMessage::kOffsetToInheritedBytes()
            + kTrustLineAmountBytesCount
            + sizeof(byte)
            + kEthereumAddressHexSize;

    return kOffset;
}
