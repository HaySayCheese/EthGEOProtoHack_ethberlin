#ifndef GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H
#define GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H

#include "base/BasePaymentTransaction.h"
#include "base/PathStats.h"
#include "../../../../io/storage/StorageHandler.h"
#include <boost/functional/hash.hpp>

#include <unordered_map>
#include <unordered_set>

class CycleCloserInitiatorTransaction : public BasePaymentTransaction {

public:
    typedef shared_ptr<CycleCloserInitiatorTransaction> Shared;
    typedef shared_ptr<const CycleCloserInitiatorTransaction> ConstShared;

public:
    CycleCloserInitiatorTransaction(
        const NodeUUID &kCurrentNodeUUID,
        Path::ConstShared path,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        Logger *log)
        noexcept;

    CycleCloserInitiatorTransaction(
        BytesShared buffer,
        TrustLinesManager *trustLines,
        StorageHandler *storageHandler,
        Logger *log)
        throw (bad_alloc);

    TransactionResult::SharedConst run()
        throw (RuntimeError, bad_alloc);

    pair<BytesShared, size_t> serializeToBytes() const
        throw (bad_alloc);

protected:
    // TODO: move it into separate *.h file.
    typedef boost::uuids::uuid PathUUID;

protected:
    // Stages handlers
    // TODO: Add throws specififcations
    TransactionResult::SharedConst runAmountReservationStage ();
    TransactionResult::SharedConst runPreviousNeighborRequestProcessingStage();
    TransactionResult::SharedConst propagateVotesListAndWaitForConfigurationRequests ();
    TransactionResult::SharedConst runFinalParticipantsRequestsProcessingStage ();

protected:
    // Coordinator must return command result on transaction finishing.
    // Therefore this methods are overriden.
    TransactionResult::SharedConst approve();
    TransactionResult::SharedConst recover(
        const char *message = nullptr);
    TransactionResult::SharedConst reject(
        const char *message = nullptr);

protected:
    TransactionResult::SharedConst tryReserveNextIntermediateNodeAmount (
        PathStats *pathStats);

    TransactionResult::SharedConst askNeighborToReserveAmount(
        const NodeUUID &neighbor,
        PathStats *pathStats);

    TransactionResult::SharedConst processNeighborAmountReservationResponse();

    TransactionResult::SharedConst askNeighborToApproveFurtherNodeReservation(
        const NodeUUID &neighbor,
        PathStats *pathStats);

    TransactionResult::SharedConst processNeighborFurtherReservationResponse();

    TransactionResult::SharedConst askRemoteNodeToApproveReservation(
        PathStats *pathStats,
        const NodeUUID &remoteNode,
        const byte remoteNodePosition,
        const NodeUUID &nextNodeAfterRemote);

    TransactionResult::SharedConst processRemoteNodeResponse();

protected:
    const string logHeader() const;
    void deserializeFromBytes(
        BytesShared buffer);

protected:
    // Contains special stats data, such as current msx flow,
    // for path involved into the transaction.
    unique_ptr<PathStats> mPathStats;

    // Reservation stage contains it's own internal steps counter.
    byte mReservationsStage;

    // minimum value of Coordinator outgoing amount to first intremediate node
    // and incoming amount from last intermediate node
    TrustLineAmount mInitialTransactionAmount;

    // Contains nodes that has been requrested final paths configuration.
    // for more details, see TODO
    unordered_set<NodeUUID> mNodesRequestedFinalConfiguration;
};

#endif //GEO_NETWORK_CLIENT_CYCLECLOSERINITIATORTRANSACTION_H
