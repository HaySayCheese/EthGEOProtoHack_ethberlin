﻿#include "TransactionsManager.h"

/*!
 *
 * Throws RuntimeError in case if some internal components can't be initialised.
 */
TransactionsManager::TransactionsManager(
    NodeUUID &nodeUUID,
    string ethereumAddress,
    as::io_service &IOService,
    EquivalentsSubsystemsRouter *equivalentsSubsystemsRouter,
    ResourcesManager *resourcesManager,
    ResultsInterface *resultsInterface,
    StorageHandler *storageHandler,
    Keystore *keystore,
    Logger &logger,
    SubsystemsController *subsystemsController,
    TrustLinesInfluenceController *trustLinesInfluenceController) :

    mNodeUUID(nodeUUID),
    mEthereumAddress(ethereumAddress),
    mIOService(IOService),
    mEquivalentsSubsystemsRouter(equivalentsSubsystemsRouter),
    mResourcesManager(resourcesManager),
    mResultsInterface(resultsInterface),
    mStorageHandler(storageHandler),
    mKeysStore(keystore),
    mLog(logger),
    mSubsystemsController(subsystemsController),
    mTrustLinesInfluenceController(trustLinesInfluenceController),

    mScheduler(
        new TransactionsScheduler(
            mIOService,
            mTrustLinesInfluenceController,
            mLog)),

    mEquivalentsCyclesSubsystemsRouter(
        new EquivalentsCyclesSubsystemsRouter(
            mNodeUUID,
            mScheduler.get(),
            mSubsystemsController,
            mIOService,
            mEquivalentsSubsystemsRouter->equivalents(),
            mLog))
{
    subscribeForCommandResult(
        mScheduler->commandResultIsReadySignal);
    subscribeForSerializeTransaction(
        mScheduler->serializeTransactionSignal);
    subscribeForCloseCycleTransaction(
        mEquivalentsCyclesSubsystemsRouter->closeCycleSignal);
    subscribeForBuildCyclesFiveNodesTransaction(
        mEquivalentsCyclesSubsystemsRouter->buildFiveNodesCyclesSignal);
    subscribeForBuildCyclesSixNodesTransaction(
        mEquivalentsCyclesSubsystemsRouter->buildSixNodesCyclesSignal);
    subscribeForTryCloseNextCycleSignal(
        mScheduler->cycleCloserTransactionWasFinishedSignal);
    subscribeForGatewayNotificationSignal(
        mEquivalentsSubsystemsRouter->gatewayNotificationSignal);

    try {
        loadTransactionsFromStorage();

    } catch (exception &e) {
        error() << "loadTransactionsFromStorage. Details: " << e.what();
        throw RuntimeError(e.what());
    }

    mVisualInterface = nullptr;
}

void TransactionsManager::loadTransactionsFromStorage()
{
    const auto ioTransaction = mStorageHandler->beginTransaction();
    const auto serializedTAs = ioTransaction->transactionHandler()->allTransactions();

    for(const auto &kTABuffer: serializedTAs) {
        auto *transactionType =
                new (kTABuffer.get()) BaseTransaction::SerializedTransactionType;
        auto transactionTypeId = *transactionType;
        auto *equivalent =
                new (kTABuffer.get()
                     + sizeof(BaseTransaction::SerializedTransactionType)) SerializedEquivalent;

        switch (transactionTypeId) {
            case BaseTransaction::CoordinatorPaymentTransaction: {
                try {
                    auto transaction = make_shared<CoordinatorPaymentTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mEquivalentsSubsystemsRouter->topologyCacheManager(*equivalent),
                        mEquivalentsSubsystemsRouter->maxFlowCacheManager(*equivalent),
                        mResourcesManager,
                        mEquivalentsSubsystemsRouter->pathsManager(*equivalent),
                        mKeysStore,
                        mLog,
                        mSubsystemsController);
                    subscribeForBuildCyclesThreeNodesTransaction(
                        transaction->mBuildCycleThreeNodesSignal);
                    subscribeForAuditSignal(
                        transaction->mTrustLineAuditSignal);
                    subscribeForPublicKeysSharingSignal(
                        transaction->mPublicKeysSharingSignal);
                    prepareAndSchedule(
                        transaction,
                        true,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized CoordinatorPaymentTransaction "
                            "with equivalent " << *equivalent  << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::IntermediateNodePaymentTransaction: {
                try {
                    auto transaction = make_shared<IntermediateNodePaymentTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mEquivalentsSubsystemsRouter->topologyCacheManager(*equivalent),
                        mEquivalentsSubsystemsRouter->maxFlowCacheManager(*equivalent),
                        mKeysStore,
                        mLog,
                        mSubsystemsController);
                    subscribeForBuildCyclesThreeNodesTransaction(
                        transaction->mBuildCycleThreeNodesSignal);
                    subscribeForBuildCyclesFourNodesTransaction(
                        transaction->mBuildCycleFourNodesSignal);
                    subscribeForAuditSignal(
                        transaction->mTrustLineAuditSignal);
                    subscribeForPublicKeysSharingSignal(
                        transaction->mPublicKeysSharingSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized IntermediateNodePaymentTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::ReceiverPaymentTransaction: {
                try {
                    auto transaction = make_shared<ReceiverPaymentTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mEquivalentsSubsystemsRouter->topologyCacheManager(*equivalent),
                        mEquivalentsSubsystemsRouter->maxFlowCacheManager(*equivalent),
                        mKeysStore,
                        mLog,
                        mSubsystemsController);
                    subscribeForBuildCyclesThreeNodesTransaction(
                        transaction->mBuildCycleThreeNodesSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized ReceiverPaymentTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::Payments_CycleCloserInitiatorTransaction: {
                try {
                    auto transaction = make_shared<CycleCloserInitiatorTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mEquivalentsCyclesSubsystemsRouter->cyclesManager(*equivalent),
                        mStorageHandler,
                        mEquivalentsSubsystemsRouter->topologyCacheManager(*equivalent),
                        mEquivalentsSubsystemsRouter->maxFlowCacheManager(*equivalent),
                        mKeysStore,
                        mLog,
                        mSubsystemsController);
                    subscribeForAuditSignal(
                        transaction->mTrustLineAuditSignal);
                    subscribeForPublicKeysSharingSignal(
                        transaction->mPublicKeysSharingSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized CycleCloserInitiatorTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::Payments_CycleCloserIntermediateNodeTransaction: {
                try {
                    auto transaction = make_shared<CycleCloserIntermediateNodeTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mEquivalentsCyclesSubsystemsRouter->cyclesManager(*equivalent),
                        mStorageHandler,
                        mEquivalentsSubsystemsRouter->topologyCacheManager(*equivalent),
                        mEquivalentsSubsystemsRouter->maxFlowCacheManager(*equivalent),
                        mKeysStore,
                        mLog,
                        mSubsystemsController);
                    subscribeForAuditSignal(
                        transaction->mTrustLineAuditSignal);
                    subscribeForPublicKeysSharingSignal(
                        transaction->mPublicKeysSharingSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized CycleCloserIntermediateNodeTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::OpenTrustLineTransaction: {
                // todo it is possible that no subsystems for current equivalent.
                // on this case subsystems for equivalent should be init
                try {
                    auto transaction = make_shared<OpenTrustLineTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        true,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized OpenTrustLineTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::AcceptTrustLineTransaction: {
                try {
                    auto transaction = make_shared<AcceptTrustLineTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized AcceptTrustLineTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::SetOutgoingTrustLineTransaction: {
                try {
                    auto transaction = make_shared<SetOutgoingTrustLineTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        true,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized SetOutgoingTrustLineTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::SetIncomingTrustLineTransaction: {
                try {
                    auto transaction = make_shared<SetIncomingTrustLineTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized SetIncomingTrustLineTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::CloseIncomingTrustLineTransactionType: {
                try {
                    auto transaction = make_shared<CloseIncomingTrustLineTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        true,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized CloseIncomingTrustLineTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::CloseOutgoingTrustLineTransactionType: {
                try {
                    auto transaction = make_shared<CloseOutgoingTrustLineTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized CloseOutgoingTrustLineTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::AuditSourceTransactionType: {
                try {
                    auto transaction = make_shared<AuditSourceTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        true,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized AuditSourceTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::AuditTargetTransactionType: {
                try {
                    auto transaction = make_shared<AuditTargetTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized AuditTargetTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::PublicKeysSharingSourceTransactionType: {
                try {
                    auto transaction = make_shared<PublicKeysSharingSourceTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        false,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized PublicKeysSharingSourceTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            case BaseTransaction::ConflictResolverInitiatorTransactionType: {
                try {
                    auto transaction = make_shared<ConflictResolverInitiatorTransaction>(
                        kTABuffer,
                        mNodeUUID,
                        mEquivalentsSubsystemsRouter->trustLinesManager(*equivalent),
                        mStorageHandler,
                        mKeysStore,
                        mTrustLinesInfluenceController,
                        mLog);
                    subscribeForProcessingConfirmationMessage(
                        transaction->processConfirmationMessageSignal);
                    prepareAndSchedule(
                        transaction,
                        false,
                        true,
                        true);
                } catch (NotFoundError &e) {
                    error() << "There are no subsystems for serialized ConflictResolverInitiatorTransaction "
                            "with equivalent " << *equivalent << " Details are: " << e.what();
                    continue;
                }
                break;
            }
            default: {
                throw RuntimeError(
                    "TrustLinesManager::loadTransactions. "
                        "Unexpected transaction type identifier " + to_string(transactionTypeId));
            }
        }

    }
}

void TransactionsManager::processCommand(
    BaseUserCommand::Shared command)
{
    // ToDo: sort calls in the call probability order.
    // For example, max flows calculations would be called much often, then credit usage transactions.
    // So, why we are checking trust lines commands first, and max flow is only in the middle of the check sequence?

    if (command->identifier() == SetOutgoingTrustLineCommand::identifier()) {
        launchSetOutgoingTrustLineTransaction(
            static_pointer_cast<SetOutgoingTrustLineCommand>(
                command));

    } else if (command->identifier() == CloseIncomingTrustLineCommand::identifier()) {
        launchCloseIncomingTrustLineTransaction(
            static_pointer_cast<CloseIncomingTrustLineCommand>(
                command));

    } else if (command->identifier() == EthereumAuditCommand::identifier()) {
        launchEthereumAuditTransaction(
            static_pointer_cast<EthereumAuditCommand>(
                 command));

    } else if (command->identifier() == EthereumCloseChannelCommand::identifier()) {
        launchEthereumCloseChannelTransaction(
            static_pointer_cast<EthereumCloseChannelCommand>(
                command));

    } else if (command->identifier() == CreditUsageCommand::identifier()) {
        launchCoordinatorPaymentTransaction(
            dynamic_pointer_cast<CreditUsageCommand>(
                command));

    } else if (command->identifier() == InitiateMaxFlowCalculationCommand::identifier()){
        launchInitiateMaxFlowCalculatingTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationCommand>(
                command));

    } else if (command->identifier() == InitiateMaxFlowCalculationFullyCommand::identifier()){
        launchMaxFlowCalculationFullyTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationFullyCommand>(
                command));

    } else if (command->identifier() == TotalBalancesCommand::identifier()){
        launchTotalBalancesTransaction(
            static_pointer_cast<TotalBalancesCommand>(
                command));

    } else if (command->identifier() == HistoryPaymentsCommand::identifier()){
        launchHistoryPaymentsTransaction(
            static_pointer_cast<HistoryPaymentsCommand>(
                command));

    } else if (command->identifier() == HistoryAdditionalPaymentsCommand::identifier()){
        launchAdditionalHistoryPaymentsTransaction(
            static_pointer_cast<HistoryAdditionalPaymentsCommand>(
                command));

    } else if (command->identifier() == HistoryTrustLinesCommand::identifier()){
        launchHistoryTrustLinesTransaction(
            static_pointer_cast<HistoryTrustLinesCommand>(
                command));

    } else if (command->identifier() == HistoryWithContractorCommand::identifier()){
        launchHistoryWithContractorTransaction(
            static_pointer_cast<HistoryWithContractorCommand>(
                command));

    } else if (command->identifier() == GetFirstLevelContractorsCommand::identifier()){
        launchGetFirstLevelContractorsTransaction(
            static_pointer_cast<GetFirstLevelContractorsCommand>(
                command));

    } else if (command->identifier() == GetTrustLinesCommand::identifier()){
        launchGetTrustLinesTransaction(
            static_pointer_cast<GetTrustLinesCommand>(
                command));

    } else if (command->identifier() == GetTrustLineCommand::identifier()){
        launchGetTrustLineTransaction(
            static_pointer_cast<GetTrustLineCommand>(
                command));

    } else if (command->identifier() == EquivalentListCommand::identifier()){
        launchGetEquivalentListTransaction(
            static_pointer_cast<EquivalentListCommand>(
                command));

    // BlackList Commands
    } else if (command->identifier() == AddNodeToBlackListCommand::identifier()){

        launchAddNodeToBlackListTransaction(
            static_pointer_cast<AddNodeToBlackListCommand>(
                command));

    } else if (command->identifier() == CheckIfNodeInBlackListCommand::identifier()){
        launchCheckIfNodeInBlackListTransaction(
            static_pointer_cast<CheckIfNodeInBlackListCommand>(
                command));

    } else if (command->identifier() == RemoveNodeFromBlackListCommand::identifier()){
        launchRemoveNodeFromBlackListTransaction(
            static_pointer_cast<RemoveNodeFromBlackListCommand>(
                command));

    } else if (command->identifier() == GetBlackListCommand::identifier()){
        launchGetBlackListTransaction(
            static_pointer_cast<GetBlackListCommand>(
                command));

    } else if (command->identifier() == PaymentTransactionByCommandUUIDCommand::identifier()){
        launchPaymentTransactionByCommandUUIDTransaction(
            static_pointer_cast<PaymentTransactionByCommandUUIDCommand>(
                command));

    } else {
        throw ValueError(
            "TransactionsManager::processCommand: "
                "Unexpected command identifier " + command->identifier());
    }
}

void TransactionsManager::processMessage(
    Message::Shared message)
{
    // ToDo: sort calls in the call probability order.
    // For example, max flows calculations would be called much often, then credit usage transactions.

    /*
     * Max flow
     */
    if (message->typeID() == Message::MessageType::MaxFlow_InitiateCalculation) {
        launchReceiveMaxFlowCalculationOnTargetTransaction(
            static_pointer_cast<InitiateMaxFlowCalculationMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_ResultMaxFlowCalculation) {
        try {
            mScheduler->tryAttachMessageToCollectTopologyTransaction(message);
        } catch (NotFoundError &) {
            launchReceiveResultMaxFlowCalculationTransaction(
                static_pointer_cast<ResultMaxFlowCalculationMessage>(message));
        }

    } else if (message->typeID() == Message::MessageType::MaxFlow_ResultMaxFlowCalculationFromGateway) {
        try {
            mScheduler->tryAttachMessageToCollectTopologyTransaction(message);
        } catch (NotFoundError &) {
            launchReceiveResultMaxFlowCalculationTransactionFromGateway(
                static_pointer_cast<ResultMaxFlowCalculationGatewayMessage>(message));
        }

    } else if (message->typeID() == Message::MessageType::MaxFlow_CalculationSourceFirstLevel) {
        launchMaxFlowCalculationSourceFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceFstLevelMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_CalculationTargetFirstLevel) {
        launchMaxFlowCalculationTargetFstLevelTransaction(
            static_pointer_cast<MaxFlowCalculationTargetFstLevelMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_CalculationSourceSecondLevel) {
        launchMaxFlowCalculationSourceSndLevelTransaction(
            static_pointer_cast<MaxFlowCalculationSourceSndLevelMessage>(message));

    } else if (message->typeID() == Message::MessageType::MaxFlow_CalculationTargetSecondLevel) {
        launchMaxFlowCalculationTargetSndLevelTransaction(
            static_pointer_cast<MaxFlowCalculationTargetSndLevelMessage>(message));

    /*
     * Payments
     */
    } else if (message->typeID() == Message::Payments_ReceiverInitPaymentRequest) {
        launchReceiverPaymentTransaction(
            static_pointer_cast<ReceiverInitPaymentRequestMessage>(message));

    } else if (message->typeID() == Message::Payments_IntermediateNodeReservationRequest) {
        // It is possible, that transaction was already initialised
        // by the ReceiverInitPaymentRequest.
        // In this case - message must be simply attached to it,
        // no new transaction must be launched.
        try {
            mScheduler->tryAttachMessageToTransaction(message);

        } catch (NotFoundError &) {
            launchIntermediateNodePaymentTransaction(
                    static_pointer_cast<IntermediateNodeReservationRequestMessage>(message));
        }

    } else if (message->typeID() == Message::Payments_IntermediateNodeCycleReservationRequest) {
        // It is possible, that transaction was already initialised
        // by the ReceiverInitPaymentRequest.
        // In this case - message must be simply attached to it,
        // no new transaction must be launched.
        try {
            mScheduler->tryAttachMessageToTransaction(message);

        } catch (NotFoundError &) {
            launchCycleCloserIntermediateNodeTransaction(
                static_pointer_cast<IntermediateNodeCycleReservationRequestMessage>(message));
        }
    } else if(message->typeID() == Message::MessageType::Payments_VotesStatusRequest) {
        launchVotesResponsePaymentsTransaction(
            static_pointer_cast<VotesStatusRequestMessage>(message));

    } else if (message->typeID() == Message::Payments_EthereumReceipt) {
        launchEthereumPaymentReceiptProcessingTransaction(
            static_pointer_cast<EthereumOutgoingReceiptMessage>(message));

    /*
     * Cycles
     */
    } else if (message->typeID() == Message::MessageType::Cycles_FiveNodesBoundary
               or message->typeID() == Message::MessageType::Cycles_SixNodesBoundary) {
        mScheduler->tryAttachMessageToCyclesFiveAndSixNodes(message);

    } else if (message->typeID() == Message::MessageType::Cycles_SixNodesMiddleware) {
        launchSixNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesSixNodesInBetweenMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_FiveNodesMiddleware) {
        launchFiveNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesFiveNodesInBetweenMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_ThreeNodesBalancesRequest){
        launchThreeNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesThreeNodesBalancesRequestMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_FourNodesNegativeBalanceRequest){
        launchFourNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesFourNodesNegativeBalanceRequestMessage>(message));

    } else if (message->typeID() == Message::MessageType::Cycles_FourNodesPositiveBalanceRequest){
        launchFourNodesCyclesResponseTransaction(
            static_pointer_cast<CyclesFourNodesPositiveBalanceRequestMessage>(message));

    /*
     * Trust lines
     */
    } else if (message->typeID() == Message::TrustLines_SetIncoming) {
        launchSetIncomingTrustLineTransaction(
            static_pointer_cast<SetIncomingTrustLineMessage>(message));

    } else if (message->typeID() == Message::TrustLines_SetIncomingInitial) {
        launchAcceptTrustLineTransaction(
            static_pointer_cast<SetIncomingTrustLineInitialMessage>(message));

    } else if (message->typeID() == Message::TrustLines_CloseOutgoing) {
        launchCloseOutgoingTrustLineTransaction(
            static_pointer_cast<CloseOutgoingTrustLineMessage>(message));

    } else if (message->typeID() == Message::TrustLines_Confirmation) {
        try {
            mScheduler->tryAttachMessageToTransaction(message);
        } catch (NotFoundError &) {
            if (!findSerializedTransactionAndLaunchIt(message)) {
                warning() << "Can't attach TrustLinesConfirmation message to transaction ";
            }
        }

    } else if (message->typeID() == Message::TrustLines_PublicKey) {
        try {
            mScheduler->tryAttachMessageToTransaction(message);

        } catch (NotFoundError &) {
            if (!findSerializedTransactionAndLaunchIt(message)) {
                launchPublicKeysSharingTargetTransaction(
                    static_pointer_cast<PublicKeyMessage>(message));
            }
        }

    } else if (message->typeID() == Message::TrustLines_HashConfirmation) {
        try {
            mScheduler->tryAttachMessageToTransaction(message);
        } catch (NotFoundError &) {
            if (!findSerializedTransactionAndLaunchIt(message)) {
                warning() << "Can't attach TrustLines_HashConfirmation message to transaction ";
            }
        }

    } else if (message->typeID() == Message::TrustLines_Audit) {
        try {
            mScheduler->tryAttachMessageToTransaction(message);
        } catch (NotFoundError &) {
            if (!findSerializedTransactionAndLaunchIt(message)) {
                launchAuditTargetTransaction(
                    static_pointer_cast<AuditMessage>(message));
            }
        }

    } else if (message->typeID() == Message::TrustLines_AuditConfirmation) {
        try {
            mScheduler->tryAttachMessageToTransaction(message);
        } catch (NotFoundError &) {
            if (!findSerializedTransactionAndLaunchIt(message)) {
                warning() << "Can't attach TrustLines_AuditConfirmation message to transaction ";
            }
        }

    } else if (message->typeID() == Message::TrustLines_ConflictResolver) {
        launchConflictResolveContractorTransaction(
            static_pointer_cast<ConflictResolverMessage>(message));

    } else if (message->typeID() == Message::TrustLines_ConflictResolverConfirmation) {
        try {
            mScheduler->tryAttachMessageToTransaction(message);
        } catch (NotFoundError &) {
            if (!findSerializedTransactionAndLaunchIt(message)) {
                warning() << "Can't attach TrustLines_ConflictResolverConfirmation message to transaction ";
            }
        }

    } else if (message->typeID() == Message::TrustLines_EthereumAudit) {
        launchEthereumAuditTargetTransaction(
            static_pointer_cast<EthereumAuditMessage>(message));

    /*
     * Gateway notification & RoutingTable
     */
    } else if (message->typeID() == Message::GatewayNotification) {
        launchGatewayNotificationReceiverTransaction(
            static_pointer_cast<GatewayNotificationMessage>(message));

    } else if (message->typeID() == Message::RoutingTableResponse) {
        try {
            mScheduler->tryAttachMessageToRoutingTableTransaction(message);
        } catch (NotFoundError &e) {
            launchRoutingTableUpdatingTransaction(
                static_pointer_cast<RoutingTableResponseMessage>(message));
        }

    /*
     * Attaching to existing transactions
     */
    } else {
        mScheduler->tryAttachMessageToTransaction(message);
    }
}

void TransactionsManager::launchSetOutgoingTrustLineTransaction(
    SetOutgoingTrustLineCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        info() << "launchSetOutgoingTrustLineTransaction: init new equivalent "
               << command->equivalent();
        mEquivalentsSubsystemsRouter->initNewEquivalent(command->equivalent());
        mEquivalentsCyclesSubsystemsRouter->initNewEquivalent(command->equivalent());
        auto transaction = make_shared<OpenTrustLineTransaction>(
            mNodeUUID,
            mEthereumAddress,
            command,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mStorageHandler,
            mSubsystemsController,
            mKeysStore,
            mEquivalentsSubsystemsRouter->iAmGateway(command->equivalent()),
            mTrustLinesInfluenceController,
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            true,
            true,
            true);
        return;
    }

    try {
        auto trustLinesManager = mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
        if (trustLinesManager->trustLineIsPresent(command->contractorUUID())) {
            if (trustLinesManager->outgoingTrustAmount(command->contractorUUID()) != TrustLine::kZeroAmount()) {
                info() << "TrustLine alredy opened";
                return;
            }
            auto transaction = make_shared<SetOutgoingTrustLineTransaction>(
                mNodeUUID,
                mEthereumAddress,
                command,
                trustLinesManager,
                mStorageHandler,
                mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
                mSubsystemsController,
                mKeysStore,
                mVisualInterface.get(),
                mTrustLinesInfluenceController,
                mLog);
            subscribeForProcessingConfirmationMessage(
                transaction->processConfirmationMessageSignal);
            prepareAndSchedule(
                transaction,
                true,
                true,
                true);
        } else {
            auto transaction = make_shared<OpenTrustLineTransaction>(
                mNodeUUID,
                mEthereumAddress,
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mStorageHandler,
                mSubsystemsController,
                mKeysStore,
                mEquivalentsSubsystemsRouter->iAmGateway(command->equivalent()),
                mTrustLinesInfluenceController,
                mLog);
            subscribeForProcessingConfirmationMessage(
                transaction->processConfirmationMessageSignal);
            prepareAndSchedule(
                transaction,
                true,
                true,
                true);
        }
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for SetOutgoingTrustLineTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchCloseIncomingTrustLineTransaction(
    CloseIncomingTrustLineCommand::Shared command)
{
    try {
        auto transaction = make_shared<CloseIncomingTrustLineTransaction>(
            mNodeUUID,
            command,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyTrustLineManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
            mSubsystemsController,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            true,
            true,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CloseIncomingTrustLineTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchEthereumAuditTransaction(
    EthereumAuditCommand::Shared command)
{
    try {
        auto transaction = make_shared<EthereumAuditTransaction>(
            mNodeUUID,
            mEthereumAddress,
            command,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            true,
            true,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchEthereumAuditTransaction "
                   "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchEthereumCloseChannelTransaction(
    EthereumCloseChannelCommand::Shared command)
{
    try {
        auto transaction = make_shared<EthereumCloseChannelTransaction>(
            mNodeUUID,
            mEthereumAddress,
            command,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            true,
            true,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchEthereumCloseChannelTransaction "
                   "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchSetIncomingTrustLineTransaction(
    SetIncomingTrustLineMessage::Shared message)
{
    try {
        auto transaction = make_shared<SetIncomingTrustLineTransaction>(
            mNodeUUID,
            mEthereumAddress,
            message,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyTrustLineManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mSubsystemsController,
            mKeysStore,
            mVisualInterface.get(),
            mTrustLinesInfluenceController,
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for SetIncomingTrustLineTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchAcceptTrustLineTransaction(
    SetIncomingTrustLineInitialMessage::Shared message)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent());
    } catch (NotFoundError &e) {
        info() << "launchAcceptTrustLineTransaction: init new equivalent "
               << message->equivalent();
        mEquivalentsSubsystemsRouter->initNewEquivalent(message->equivalent());
        mEquivalentsCyclesSubsystemsRouter->initNewEquivalent(message->equivalent());
    }

    try {
        auto transaction = make_shared<AcceptTrustLineTransaction>(
            mNodeUUID,
            mEthereumAddress,
            message,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mSubsystemsController,
            mKeysStore,
            mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent()),
            mTrustLinesInfluenceController,
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for AcceptTrustLineTransaction "
                   "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchCloseOutgoingTrustLineTransaction(
    CloseOutgoingTrustLineMessage::Shared message)
{
    try {
        auto transaction = make_shared<CloseOutgoingTrustLineTransaction>(
            mNodeUUID,
            message,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CloseOutgoingTrustLineTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchPublicKeysSharingTargetTransaction(
    PublicKeyMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<PublicKeysSharingTargetTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mStorageHandler,
                mKeysStore,
                mTrustLinesInfluenceController,
                mLog),
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for PublicKeysSharingTargetTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchAuditTargetTransaction(
    AuditMessage::Shared message)
{
    try {
        auto transaction = make_shared<AuditTargetTransaction>(
            mNodeUUID,
            message,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for AuditTargetTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchConflictResolveContractorTransaction(
    ConflictResolverMessage::Shared message)
{
    try {
        auto transaction = make_shared<ConflictResolverContractorTransaction>(
            mNodeUUID,
            message,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);
        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);
        prepareAndSchedule(
            transaction,
            false,
            true,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ConflictResolverContractorTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchEthereumAuditTargetTransaction(
    EthereumAuditMessage::Shared message)
{
    try {
        auto transaction = make_shared<EthereumAuditTargetTransaction>(
            mNodeUUID,
            mEthereumAddress,
            message,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mLog);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchEthereumAuditTargetTransaction "
                   "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchInitiateMaxFlowCalculatingTransaction(
    InitiateMaxFlowCalculationCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<InitiateMaxFlowCalculationTransaction>(
                mNodeUUID,
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->iAmGateway(command->equivalent()),
                mLog),
            true,
            true,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for InitiateMaxFlowCalculationTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationFullyTransaction(
    InitiateMaxFlowCalculationFullyCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationFullyTransaction>(
                mNodeUUID,
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
                mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
                mLog),
            true,
            true,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationFullyTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveMaxFlowCalculationOnTargetTransaction(
    InitiateMaxFlowCalculationMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<ReceiveMaxFlowCalculationOnTargetTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ReceiveMaxFlowCalculationOnTargetTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveResultMaxFlowCalculationTransaction(
    ResultMaxFlowCalculationMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<ReceiveResultMaxFlowCalculationTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ReceiveResultMaxFlowCalculationTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchReceiveResultMaxFlowCalculationTransactionFromGateway(
    ResultMaxFlowCalculationGatewayMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<ReceiveResultMaxFlowCalculationTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ReceiveResultMaxFlowCalculationTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationSourceFstLevelTransaction(
    MaxFlowCalculationSourceFstLevelMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationSourceFstLevelTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog,
                mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent())),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationSourceFstLevelTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationTargetFstLevelTransaction(
    MaxFlowCalculationTargetFstLevelMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationTargetFstLevelTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog,
                mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent())),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationTargetFstLevelTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationSourceSndLevelTransaction(
    MaxFlowCalculationSourceSndLevelMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationSourceSndLevelTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
                mLog,
                mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent())),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationSourceSndLevelTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchMaxFlowCalculationTargetSndLevelTransaction(
    MaxFlowCalculationTargetSndLevelMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<MaxFlowCalculationTargetSndLevelTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
                mLog,
                mEquivalentsSubsystemsRouter->iAmGateway(message->equivalent())),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for MaxFlowCalculationTargetSndLevelTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchCoordinatorPaymentTransaction(
    CreditUsageCommand::Shared command)
{
    try {
        auto transaction = make_shared<CoordinatorPaymentTransaction>(
            mNodeUUID,
            command,
            mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(command->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(command->equivalent()),
            mResourcesManager,
            mEquivalentsSubsystemsRouter->pathsManager(command->equivalent()),
            mKeysStore,
            mLog,
            mSubsystemsController,
            mVisualInterface.get());
        subscribeForBuildCyclesThreeNodesTransaction(
            transaction->mBuildCycleThreeNodesSignal);
        subscribeForAuditSignal(
            transaction->mTrustLineAuditSignal);
        subscribeForPublicKeysSharingSignal(
            transaction->mPublicKeysSharingSignal);
        subscribeForSendEthereumReceiptsSignal(
            transaction->mEthereumOutgoingReceiptSignal);
        prepareAndSchedule(transaction, true, false, true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CoordinatorPaymentTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchReceiverPaymentTransaction(
    ReceiverInitPaymentRequestMessage::Shared message)
{
    try {
        auto transaction = make_shared<ReceiverPaymentTransaction>(
            mNodeUUID,
            message,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mKeysStore,
            mLog,
            mSubsystemsController,
            mVisualInterface.get());
        subscribeForBuildCyclesThreeNodesTransaction(
            transaction->mBuildCycleThreeNodesSignal);
        prepareAndSchedule(transaction, false, false, true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for ReceiverPaymentTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                message,
                mLog),
            false,
            false,
            true);
    }
}

void TransactionsManager::launchIntermediateNodePaymentTransaction(
    IntermediateNodeReservationRequestMessage::Shared message)
{
    try {
        auto transaction = make_shared<IntermediateNodePaymentTransaction>(
            mNodeUUID,
            message,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForBuildCyclesThreeNodesTransaction(
            transaction->mBuildCycleThreeNodesSignal);
        subscribeForBuildCyclesFourNodesTransaction(
            transaction->mBuildCycleFourNodesSignal);
        subscribeForAuditSignal(
            transaction->mTrustLineAuditSignal);
        subscribeForPublicKeysSharingSignal(
            transaction->mPublicKeysSharingSignal);
        subscribeForSendEthereumReceiptsSignal(
            transaction->mEthereumOutgoingReceiptSignal);
        prepareAndSchedule(transaction, false, false, true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for IntermediateNodePaymentTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                message,
                mLog),
            false,
            false,
            true);
    }
}

void TransactionsManager::launchVotesResponsePaymentsTransaction(
    VotesStatusRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<VotesStatusResponsePaymentTransaction>(
                mNodeUUID,
                message,
                mStorageHandler,
                mScheduler->isTransactionInProcess(
                    message->transactionUUID()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchEthereumPaymentReceiptProcessingTransaction(
    EthereumOutgoingReceiptMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<EthereumPaymentReceiptProcessingTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mEquivalentsSubsystemsRouter->ethereumSiganturesManager(message->equivalent()),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchEthereumSendPaymentReceiptsTransaction(
    map<NodeUUID, TrustLineAmount> outgoingReceipts,
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<EthereumSendPaymentReceiptsTransaction>(
                mNodeUUID,
                equivalent,
                outgoingReceipts,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::onCloseCycleTransaction(
    const SerializedEquivalent equivalent,
    Path::ConstShared cycle)
{
    try {
        auto transaction = make_shared<CycleCloserInitiatorTransaction>(
            mNodeUUID,
            cycle,
            equivalent,
            mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
            mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(equivalent),
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForAuditSignal(
            transaction->mTrustLineAuditSignal);
        subscribeForPublicKeysSharingSignal(
            transaction->mPublicKeysSharingSignal);
        prepareAndSchedule(
            transaction,
            true,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CycleCloserInitiatorTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::launchCycleCloserIntermediateNodeTransaction(
    IntermediateNodeCycleReservationRequestMessage::Shared message)
{
    try{
        auto transaction = make_shared<CycleCloserIntermediateNodeTransaction>(
            mNodeUUID,
            message,
            mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
            mEquivalentsCyclesSubsystemsRouter->cyclesManager(message->equivalent()),
            mStorageHandler,
            mEquivalentsSubsystemsRouter->topologyCacheManager(message->equivalent()),
            mEquivalentsSubsystemsRouter->maxFlowCacheManager(message->equivalent()),
            mKeysStore,
            mLog,
            mSubsystemsController);
        subscribeForAuditSignal(
            transaction->mTrustLineAuditSignal);
        subscribeForPublicKeysSharingSignal(
            transaction->mPublicKeysSharingSignal);
        prepareAndSchedule(
            transaction,
            false,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CycleCloserIntermediateNodeTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                message,
                mLog),
            false,
            false,
            true);
    }
}

void TransactionsManager::launchThreeNodesCyclesInitTransaction(
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesThreeNodesInitTransaction>(
                mNodeUUID,
                contractorUUID,
                equivalent,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->routingTableManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesThreeNodesInitTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::launchThreeNodesCyclesResponseTransaction(
    CyclesThreeNodesBalancesRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesThreeNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesThreeNodesReceiverTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchSixNodesCyclesInitTransaction(
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesSixNodesInitTransaction>(
                mNodeUUID,
                equivalent,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesSixNodesInitTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::launchSixNodesCyclesResponseTransaction(
    CyclesSixNodesInBetweenMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesSixNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesSixNodesReceiverTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchFiveNodesCyclesInitTransaction(
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFiveNodesInitTransaction>(
                mNodeUUID,
                equivalent,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFiveNodesInitTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::launchFiveNodesCyclesResponseTransaction(
    CyclesFiveNodesInBetweenMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFiveNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFiveNodesReceiverTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchFourNodesCyclesInitTransaction(
    const NodeUUID &creditorUUID,
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFourNodesInitTransaction>(
                mNodeUUID,
                creditorUUID,
                equivalent,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->routingTableManager(equivalent),
                mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFourNodesInitTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::launchFourNodesCyclesResponseTransaction(
    CyclesFourNodesNegativeBalanceRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFourNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFourNodesReceiverTransaction "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

void TransactionsManager::launchFourNodesCyclesResponseTransaction(
    CyclesFourNodesPositiveBalanceRequestMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<CyclesFourNodesReceiverTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter->trustLinesManager(message->equivalent()),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for CyclesFourNodesReceiverTransaction (positive) "
                "with equivalent " << message->equivalent() << " Details are: " << e.what();
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchTotalBalancesTransaction(
    TotalBalancesCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<TotalBalancesTransaction>(
                mNodeUUID,
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            true);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for TotalBalancesTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchHistoryPaymentsTransaction(
    HistoryPaymentsCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchHistoryPaymentsTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
        return;
    }
    try {
        prepareAndSchedule(
            make_shared<HistoryPaymentsTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchAdditionalHistoryPaymentsTransaction(
    HistoryAdditionalPaymentsCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchAdditionalHistoryPaymentsTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
        return;
    }
    try {
        prepareAndSchedule(
            make_shared<HistoryAdditionalPaymentsTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}


/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchHistoryTrustLinesTransaction(
    HistoryTrustLinesCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchHistoryTrustLinesTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
        return;
    }
    try {
        prepareAndSchedule(
            make_shared<HistoryTrustLinesTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

/*!
 *
 * Throws MemoryError.
 */
void TransactionsManager::launchHistoryWithContractorTransaction(
    HistoryWithContractorCommand::Shared command)
{
    try {
        mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for launchHistoryWithContractorTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
        return;
    }
    try {
        prepareAndSchedule(
            make_shared<HistoryWithContractorTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchGetFirstLevelContractorsTransaction(
    GetFirstLevelContractorsCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetFirstLevelContractorsTransaction>(
                mNodeUUID,
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for GetFirstLevelContractorsTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}


void TransactionsManager::launchGetTrustLinesTransaction(
    GetTrustLinesCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetFirstLevelContractorsBalancesTransaction>(
                mNodeUUID,
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for GetFirstLevelContractorsBalancesTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }

}

void TransactionsManager::launchGetTrustLineTransaction(
    GetTrustLineCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetFirstLevelContractorBalanceTransaction>(
                mNodeUUID,
                command,
                mEquivalentsSubsystemsRouter->trustLinesManager(command->equivalent()),
                mLog),
            true,
            false,
            false);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for GetFirstLevelContractorBalanceTransaction "
                "with equivalent " << command->equivalent() << " Details are: " << e.what();
        prepareAndSchedule(
            make_shared<NoEquivalentTransaction>(
                mNodeUUID,
                command,
                mLog),
            false,
            false,
            false);
    }
}

void TransactionsManager::launchGetEquivalentListTransaction(
    EquivalentListCommand::Shared command)
{
    prepareAndSchedule(
        make_shared<GetEquivalentListTransaction>(
            mNodeUUID,
            command,
            mEquivalentsSubsystemsRouter,
            mLog),
        true,
        false,
        false);
}

void TransactionsManager::launchAddNodeToBlackListTransaction(
    AddNodeToBlackListCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<AddNodeToBlackListTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mEquivalentsSubsystemsRouter,
                mSubsystemsController,
                mTrustLinesInfluenceController,
                mKeysStore,
                mLog),
            true,
            true,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchCheckIfNodeInBlackListTransaction(
    CheckIfNodeInBlackListCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<CheckIfNodeInBlackListTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchRemoveNodeFromBlackListTransaction(
    RemoveNodeFromBlackListCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<RemoveNodeFromBlackListTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchGetBlackListTransaction(
    GetBlackListCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<GetBlackListTransaction>(
                mNodeUUID,
                command,
                mStorageHandler,
                mLog),
            true,
            false,
            false);
    } catch (ConflictError &e) {
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchPaymentTransactionByCommandUUIDTransaction(
    PaymentTransactionByCommandUUIDCommand::Shared command)
{
    try {
        prepareAndSchedule(
            make_shared<PaymentTransactionByCommandUUIDTransaction>(
                mNodeUUID,
                command,
                mScheduler->paymentTransactionByCommandUUID(
                    command->paymentTransactionCommandUUID()),
                mLog),
            false,
            false,
            false);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchGatewayNotificationSenderTransaction()
{
    try {
        prepareAndSchedule(
            make_shared<GatewayNotificationSenderTransaction>(
                mNodeUUID,
                mEquivalentsSubsystemsRouter,
                mEquivalentsCyclesSubsystemsRouter.get(),
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchGatewayNotificationReceiverTransaction(
    GatewayNotificationMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<GatewayNotificationReceiverTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter,
                mStorageHandler,
                mLog),
            false,
            false,
            true);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchRoutingTableUpdatingTransaction(
    RoutingTableResponseMessage::Shared message)
{
    try {
        prepareAndSchedule(
            make_shared<RoutingTableUpdatingTransaction>(
                mNodeUUID,
                message,
                mEquivalentsSubsystemsRouter,
                mEquivalentsCyclesSubsystemsRouter.get(),
                mLog),
            false,
            false,
            false);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    }
}

void TransactionsManager::launchFindPathByMaxFlowTransaction(
    const TransactionUUID &requestedTransactionUUID,
    const NodeUUID &destinationNodeUUID,
    const SerializedEquivalent equivalent)
{
    try {
        prepareAndSchedule(
            make_shared<FindPathByMaxFlowTransaction>(
                mNodeUUID,
                destinationNodeUUID,
                requestedTransactionUUID,
                equivalent,
                mEquivalentsSubsystemsRouter->pathsManager(equivalent),
                mResourcesManager,
                mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
                mEquivalentsSubsystemsRouter->topologyTrustLineManager(equivalent),
                mEquivalentsSubsystemsRouter->topologyCacheManager(equivalent),
                mEquivalentsSubsystemsRouter->maxFlowCacheManager(equivalent),
                mLog),
            true,
            true,
            false);
    } catch (ConflictError &e){
        throw ConflictError(e.message());
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for FindPathByMaxFlowTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::attachResourceToTransaction(
    BaseResource::Shared resource)
{
    mScheduler->tryAttachResourceToTransaction(
        resource);
}

void TransactionsManager::subscribeForSubsidiaryTransactions(
    BaseTransaction::LaunchSubsidiaryTransactionSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onSubsidiaryTransactionReady,
            this,
            _1));
}

void TransactionsManager::subscribeForOutgoingMessages(
    BaseTransaction::SendMessageSignal &signal)
{
    // ToDo: connect signals of transaction and core directly (signal -> signal)
    // Boost allows this type of connectivity.
    signal.connect(
        boost::bind(
            &TransactionsManager::onTransactionOutgoingMessageReady,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForOutgoingMessagesWithCaching(
    BaseTransaction::SendMessageWithCachingSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onTransactionOutgoingMessageWithCachingReady,
            this,
            _1,
            _2,
            _3));
}

void TransactionsManager::subscribeForSerializeTransaction(
    TransactionsScheduler::SerializeTransactionSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onSerializeTransaction,
            this,
            _1));
}

void TransactionsManager::subscribeForCommandResult(
    TransactionsScheduler::CommandResultSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onCommandResultReady,
            this,
            _1));
}

void TransactionsManager::subscribeForBuildCyclesThreeNodesTransaction(
    BasePaymentTransaction::BuildCycleThreeNodesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuildCycleThreeNodesTransaction,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForBuildCyclesFourNodesTransaction(
    BasePaymentTransaction::BuildCycleFourNodesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuildCycleFourNodesTransaction,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForBuildCyclesFiveNodesTransaction(
    EquivalentsCyclesSubsystemsRouter::BuildFiveNodesCyclesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuildCycleFiveNodesTransaction,
            this,
            _1));
}

void TransactionsManager::subscribeForBuildCyclesSixNodesTransaction(
    EquivalentsCyclesSubsystemsRouter::BuildSixNodesCyclesSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onBuildCycleSixNodesTransaction,
            this,
            _1));
}

void TransactionsManager::subscribeForCloseCycleTransaction(
    EquivalentsCyclesSubsystemsRouter::CloseCycleSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onCloseCycleTransaction,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForTryCloseNextCycleSignal(
    TransactionsScheduler::CycleCloserTransactionWasFinishedSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onTryCloseNextCycleSlot,
            this,
            _1));
}

void TransactionsManager::subscribeForProcessingConfirmationMessage(
    BaseTransaction::ProcessConfirmationMessageSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onProcessConfirmationMessageSlot,
            this,
            _1));
}

void TransactionsManager::subscribeForGatewayNotificationSignal(
    EquivalentsSubsystemsRouter::GatewayNotificationSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onGatewayNotificationSlot,
            this));
}

void TransactionsManager::subscribeForAuditSignal(
    BasePaymentTransaction::TrustLineAuditSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onAuditOnTrustLineSlot,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForPublicKeysSharingSignal(
    BasePaymentTransaction::PublicKeysSharingSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::onPublicKeySharingSlot,
            this,
            _1,
            _2));
}

void TransactionsManager::subscribeForSendEthereumReceiptsSignal(
    BasePaymentTransaction::EthereumOutgoingReceiptsSignal &signal)
{
    signal.connect(
        boost::bind(
            &TransactionsManager::launchEthereumSendPaymentReceiptsTransaction,
            this,
            _1,
            _2));
}

void TransactionsManager::onTransactionOutgoingMessageReady(
    Message::Shared message,
    const NodeUUID &contractorUUID)
{
    transactionOutgoingMessageReadySignal(
        message,
        contractorUUID);
}

void TransactionsManager::onTransactionOutgoingMessageWithCachingReady(
    TransactionMessage::Shared message,
    const NodeUUID &contractorUUID,
    Message::MessageType incomingMessageTypeFilter)
{
    transactionOutgoingMessageWithCachingReadySignal(
        message,
        contractorUUID,
        incomingMessageTypeFilter);
}

/*!
 * Writes received result to the outgoing results fifo.
 *
 * Throws RuntimeError - in case if result can't be processed.
 */
void TransactionsManager::onCommandResultReady(
    CommandResult::SharedConst result)
{
    try {
        auto message = result->serialize();

        info() << "Result for command " + result->identifier();

        if (result->identifier() == HistoryPaymentsCommand::identifier() or
                result->identifier() == HistoryTrustLinesCommand::identifier() or
                result->identifier() == HistoryWithContractorCommand::identifier() or
                result->identifier() == HistoryAdditionalPaymentsCommand::identifier() or
                result->identifier() == GetTrustLinesCommand::identifier()) {
            auto shortMessage = result->serializeShort();
            info() << "CommandResultReady: " << shortMessage;
        } else {
            info() << "CommandResultReady: " << message;
        }

        mResultsInterface->writeResult(
            message.c_str(),
            message.size());

    } catch (...) {
        throw RuntimeError(
            "TransactionsManager::onCommandResultReady: "
                "Error occurred when command result has accepted");
    }
}

void TransactionsManager::onSubsidiaryTransactionReady(
    BaseTransaction::Shared transaction)
{
    subscribeForSubsidiaryTransactions(
        transaction->runSubsidiaryTransactionSignal);

    subscribeForOutgoingMessages(
        transaction->outgoingMessageIsReadySignal);

    subscribeForProcessingConfirmationMessage(
        transaction->processConfirmationMessageSignal);

    mScheduler->postponeTransaction(
        transaction,
        50);
}

void TransactionsManager::onBuildCycleThreeNodesTransaction(
    set<NodeUUID> &contractorsUUID,
    const SerializedEquivalent equivalent)
{
    for (const auto &contractorUUID : contractorsUUID) {
        launchThreeNodesCyclesInitTransaction(
            contractorUUID,
            equivalent);
    }
}

void TransactionsManager::onBuildCycleFourNodesTransaction(
    set<NodeUUID> &creditors,
    const SerializedEquivalent equivalent)
{
    for (const auto &kCreditor : creditors) {
        launchFourNodesCyclesInitTransaction(
            kCreditor,
            equivalent);
    }
}

void TransactionsManager::onBuildCycleFiveNodesTransaction(
    const SerializedEquivalent equivalent)
{
    launchFiveNodesCyclesInitTransaction(
        equivalent);
}

void TransactionsManager::onBuildCycleSixNodesTransaction(
    const SerializedEquivalent equivalent)
{
    launchSixNodesCyclesInitTransaction(
        equivalent);
}

void TransactionsManager::onTryCloseNextCycleSlot(
    const SerializedEquivalent equivalent)
{
    try {
        mEquivalentsCyclesSubsystemsRouter->cyclesManager(equivalent)->closeOneCycle(true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for Closing next cycle "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::onProcessConfirmationMessageSlot(
    ConfirmationMessage::Shared confirmationMessage)
{
    info() << "onProcessConfirmationMessageSlot from " << confirmationMessage->senderUUID
           << " on equivalent " << confirmationMessage->equivalent();
    ProcessConfirmationMessageSignal(
        confirmationMessage);
}

void TransactionsManager::onGatewayNotificationSlot()
{
    launchGatewayNotificationSenderTransaction();
}

void TransactionsManager::onAuditOnTrustLineSlot(
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent)
{
    try {
        auto transaction = make_shared<AuditSourceTransaction>(
            mNodeUUID,
            contractorUUID,
            equivalent,
            mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
            mStorageHandler,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);

        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);

        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);

        subscribeForSubsidiaryTransactions(
            transaction->runSubsidiaryTransactionSignal);

        // wait for finish of contractor payment TA
        mScheduler->postponeTransaction(
            transaction,
            500);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for AuditSourceTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

void TransactionsManager::onPublicKeySharingSlot(
    const NodeUUID &contractorUUID,
    const SerializedEquivalent equivalent)
{
    try {
        auto transaction = make_shared<PublicKeysSharingSourceTransaction>(
            mNodeUUID,
            contractorUUID,
            equivalent,
            mEquivalentsSubsystemsRouter->trustLinesManager(equivalent),
            mStorageHandler,
            mKeysStore,
            mTrustLinesInfluenceController,
            mLog);

        subscribeForProcessingConfirmationMessage(
            transaction->processConfirmationMessageSignal);

        prepareAndSchedule(
            transaction,
            true,
            false,
            true);
    } catch (NotFoundError &e) {
        error() << "There are no subsystems for PublicKeysSharingSourceTransaction "
                "with equivalent " << equivalent << " Details are: " << e.what();
    }
}

/**
 *
 * @throws bad_alloc;
 */
void TransactionsManager::prepareAndSchedule(
    BaseTransaction::Shared transaction,
    bool regenerateUUID,
    bool subsidiaryTransactionSubscribe,
    bool outgoingMessagesSubscribe)
{
    if (outgoingMessagesSubscribe) {
        subscribeForOutgoingMessages(
            transaction->outgoingMessageIsReadySignal);
        subscribeForOutgoingMessagesWithCaching(
            transaction->sendMessageWithCachingSignal);
    }
    if (subsidiaryTransactionSubscribe) {
        subscribeForSubsidiaryTransactions(
            transaction->runSubsidiaryTransactionSignal);
    }

    while (true) {
        try {

            mScheduler->scheduleTransaction(
                transaction);
            return;

        } catch(bad_alloc &) {
            throw bad_alloc();

        } catch(ConflictError &e) {
            if (regenerateUUID) {
                warning() << "prepareAndSchedule:" << "TransactionUUID: " << transaction->currentTransactionUUID()
                          << " New TransactionType:" << transaction->transactionType()
                          << " Recreate.";
                transaction->recreateTransactionUUID();
            } else {
                warning() << "prepareAndSchedule:" << "TransactionUUID: " << transaction->currentTransactionUUID()
                          << " New TransactionType:" << transaction->transactionType()
                          << " Exit.";
                return;
            }
        }
    }
}

// some transactions can be stored in database when didn't receive response message on time.
// because request message stored on communicator messages queue response should be delivered later.
// on this case transaction should be deserialized and run.
// if there is no transaction with requested transaction uuid, method returns false.
bool TransactionsManager::findSerializedTransactionAndLaunchIt(
    Message::Shared message)
{
    if (!message->isTransactionMessage()) {
        throw ValueError(
            "TransactionsScheduler::findSerializedTransactionAndLaunchIt: "
                "message received is not TransactionMessage " + to_string(message->typeID()));
    }

    auto transactionMessage = static_pointer_cast<TransactionMessage>(message);
    info() << "findSerializedTransactionAndLaunchIt " << transactionMessage->transactionUUID()
           << " eq " << transactionMessage->equivalent();
    auto ioTransaction = mStorageHandler->beginTransaction();
    BytesShared serializedTransaction;
    try {
        serializedTransaction = ioTransaction->transactionHandler()->getTransaction(
            transactionMessage->transactionUUID());
    } catch (NotFoundError& ) {
        return false;
    }

    auto *transactionType =
        new (serializedTransaction.get()) BaseTransaction::SerializedTransactionType;
    auto transactionTypeId = *transactionType;
    // todo maybe compare message->equivalent and transaction->equivalent
    // todo discuss what reaction should be on NotFoundError, now it is return false
    switch (transactionTypeId) {
        case BaseTransaction::OpenTrustLineTransaction: {
            // todo it is possible that no subsystems for current equivalent.
            // on this case subsystems for equivalent should be init
            try {
                auto transaction = make_shared<OpenTrustLineTransaction>(
                    serializedTransaction,
                    mNodeUUID,
                    mEquivalentsSubsystemsRouter->trustLinesManager(
                        transactionMessage->equivalent()),
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog);
                transaction->pushContext(message);
                subscribeForProcessingConfirmationMessage(
                    transaction->processConfirmationMessageSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    true,
                    true);
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized OpenTrustLineTransaction "
                        "with equivalent " << transactionMessage->equivalent() << " Details are: " << e.what();
                return false;
            }
            break;
        }
        case BaseTransaction::AcceptTrustLineTransaction: {
            try {
                auto transaction = make_shared<AcceptTrustLineTransaction>(
                    serializedTransaction,
                    mNodeUUID,
                    mEquivalentsSubsystemsRouter->trustLinesManager(
                        transactionMessage->equivalent()),
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog);
                transaction->pushContext(message);
                subscribeForProcessingConfirmationMessage(
                    transaction->processConfirmationMessageSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    false,
                    true);
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized AcceptTrustLineTransaction "
                        "with equivalent " << transactionMessage->equivalent() << " Details are: " << e.what();
                return false;
            }
            break;
        }
        case BaseTransaction::SetOutgoingTrustLineTransaction: {
            try {
                auto transaction = make_shared<SetOutgoingTrustLineTransaction>(
                    serializedTransaction,
                    mNodeUUID,
                    mEquivalentsSubsystemsRouter->trustLinesManager(
                        transactionMessage->equivalent()),
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog);
                transaction->pushContext(message);
                subscribeForProcessingConfirmationMessage(
                    transaction->processConfirmationMessageSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    true,
                    true);
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized SetOutgoingTrustLineTransaction "
                        "with equivalent " << transactionMessage->equivalent() << " Details are: " << e.what();
                return false;
            }
            break;
        }
        case BaseTransaction::SetIncomingTrustLineTransaction: {
            try {
                auto transaction = make_shared<SetIncomingTrustLineTransaction>(
                    serializedTransaction,
                    mNodeUUID,
                    mEquivalentsSubsystemsRouter->trustLinesManager(
                        transactionMessage->equivalent()),
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog);
                transaction->pushContext(message);
                subscribeForProcessingConfirmationMessage(
                    transaction->processConfirmationMessageSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    false,
                    true);
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized SetIncomingTrustLineTransaction "
                        "with equivalent " << transactionMessage->equivalent() << " Details are: " << e.what();
                return false;
            }
            break;
        }
        case BaseTransaction::CloseIncomingTrustLineTransactionType: {
            try {
                auto transaction = make_shared<CloseIncomingTrustLineTransaction>(
                    serializedTransaction,
                    mNodeUUID,
                    mEquivalentsSubsystemsRouter->trustLinesManager(
                        transactionMessage->equivalent()),
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog);
                transaction->pushContext(message);
                subscribeForProcessingConfirmationMessage(
                    transaction->processConfirmationMessageSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    true,
                    true);
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized CloseIncomingTrustLineTransaction "
                        "with equivalent " << transactionMessage->equivalent() << " Details are: " << e.what();
                return false;
            }
            break;
        }
        case BaseTransaction::CloseOutgoingTrustLineTransactionType: {
            try {
                auto transaction = make_shared<CloseOutgoingTrustLineTransaction>(
                    serializedTransaction,
                    mNodeUUID,
                    mEquivalentsSubsystemsRouter->trustLinesManager(
                        transactionMessage->equivalent()),
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog);
                transaction->pushContext(message);
                subscribeForProcessingConfirmationMessage(
                    transaction->processConfirmationMessageSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    false,
                    true);
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized CloseOutgoingTrustLineTransaction "
                        "with equivalent " << transactionMessage->equivalent() << " Details are: " << e.what();
                return false;
            }
            break;
        }
        case BaseTransaction::AuditSourceTransactionType: {
            try {
                auto transaction = make_shared<AuditSourceTransaction>(
                    serializedTransaction,
                    mNodeUUID,
                    mEquivalentsSubsystemsRouter->trustLinesManager(
                        transactionMessage->equivalent()),
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog);
                transaction->pushContext(message);
                subscribeForProcessingConfirmationMessage(
                    transaction->processConfirmationMessageSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    true,
                    true);
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized AuditSourceTransaction "
                        "with equivalent " << transactionMessage->equivalent() << " Details are: " << e.what();
                return false;
            }
            break;
        }
        case BaseTransaction::AuditTargetTransactionType: {
            try {
                auto transaction = make_shared<AuditTargetTransaction>(
                    serializedTransaction,
                    mNodeUUID,
                    mEquivalentsSubsystemsRouter->trustLinesManager(
                        transactionMessage->equivalent()),
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog);
                transaction->pushContext(message);
                subscribeForProcessingConfirmationMessage(
                    transaction->processConfirmationMessageSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    false,
                    true);
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized AuditTargetTransaction "
                        "with equivalent " << transactionMessage->equivalent() << " Details are: " << e.what();
                return false;
            }
            break;
        }
        case BaseTransaction::PublicKeysSharingSourceTransactionType: {
            try {
                auto transaction = make_shared<PublicKeysSharingSourceTransaction>(
                    serializedTransaction,
                    mNodeUUID,
                    mEquivalentsSubsystemsRouter->trustLinesManager(
                        transactionMessage->equivalent()),
                    mStorageHandler,
                    mKeysStore,
                    mTrustLinesInfluenceController,
                    mLog);
                transaction->pushContext(message);
                subscribeForProcessingConfirmationMessage(
                    transaction->processConfirmationMessageSignal);
                prepareAndSchedule(
                    transaction,
                    false,
                    false,
                    true);
            } catch (NotFoundError &e) {
                error() << "There are no subsystems for serialized PublicKeysSharingSourceTransaction "
                        "with equivalent " << transactionMessage->equivalent() << " Details are: " << e.what();
                return false;
            }
            break;
        }
        default: {
            error() << "findSerializedTransactionAndLaunchIt. Unexpected transaction type identifier "
                    << transactionTypeId;
            return false;
        }
    }
    return true;
}

void TransactionsManager::onSerializeTransaction(
    BaseTransaction::Shared transaction)
{
    const auto kTransactionTypeId = transaction->transactionType();
    const auto ioTransaction = mStorageHandler->beginTransaction();
    switch (kTransactionTypeId) {
        case BaseTransaction::TransactionType::CoordinatorPaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<CoordinatorPaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::TransactionType::IntermediateNodePaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<IntermediateNodePaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::TransactionType::ReceiverPaymentTransaction: {
            const auto kChildTransaction = static_pointer_cast<ReceiverPaymentTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::TransactionType::Payments_CycleCloserInitiatorTransaction: {
            const auto kChildTransaction = static_pointer_cast<CycleCloserInitiatorTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        case BaseTransaction::TransactionType::Payments_CycleCloserIntermediateNodeTransaction: {
            const auto kChildTransaction = static_pointer_cast<CycleCloserIntermediateNodeTransaction>(transaction);
            const auto transactionBytesAndCount = kChildTransaction->serializeToBytes();
            ioTransaction->transactionHandler()->saveRecord(
                kChildTransaction->currentTransactionUUID(),
                transactionBytesAndCount.first,
                transactionBytesAndCount.second);
            break;
        }
        default: {
            throw RuntimeError(
                "TrustLinesManager::onSerializeTransaction. "
                    "Unexpected transaction type identifier " + to_string(kTransactionTypeId));
        }
    }
}

string TransactionsManager::logHeader()
    noexcept
{
    return "[TransactionsManager]";
}

LoggerStream TransactionsManager::error() const
    noexcept
{
    return mLog.error(logHeader());
}

LoggerStream TransactionsManager::warning() const
    noexcept
{
    return mLog.warning(logHeader());
}

LoggerStream TransactionsManager::info() const
    noexcept
{
    return mLog.info(logHeader());
}

void TransactionsManager::activateVisualInterface()
{
    if (mVisualInterface == nullptr) {
        mVisualInterface = make_unique<VisualInterface>(mLog);
        info() << "Visual interface activated";
    } else {
        warning() << "Visual interface already activated";
    }
}

void TransactionsManager::deactivateVisualInterface()
{
    if (mVisualInterface != nullptr) {
        mVisualInterface = nullptr;
        info() << "Visual interface deactivated";
    } else {
        warning() << "Visual interface already deactivated";
    }
}