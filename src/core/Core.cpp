#include "Core.h"

Core::Core() {
    zeroPointers();
}

Core::~Core() {
    cleanupMemory();
}

int Core::run() {
    auto initCode = initCoreComponents();
    if (initCode != 0)
        return initCode;

    try {
        mCommunicator->beginAcceptMessages();
        mCommandsInterface->beginAcceptCommands();

        mIOService.run();
        return 0;

    } catch (const Exception &e) {
        mLog.logException("core", e);
        return -1;
    }
}

int Core::initCoreComponents() {
    int initCode;

    initCode = initSettings();
    if (initCode != 0)
        return initCode;

    initCode = initCommandsInterface();
    if (initCode != 0)
        return initCode;

    json conf;
    try {
        // Optimised conf read.
        // (several params at once)
        // For the details - see settings realisation.
        conf = mSettings->loadParsedJSON();

    } catch (Exception &e) {
        mLog.logException("settings", e);
        return -1;

    } catch (std::exception &e) {
        mLog.logException("settings", e);
        return -1;
    }

    initCode = initCommunicator(conf);
    if (initCode != 0)
        return initCode;
}

int Core::initSettings() {
    try {
        mSettings = new Settings();

    } catch (const Exception &e) {
        mLog.logException("settings", e);
        return -1;

    } catch (const std::exception &e) {
        mLog.logException("settings", e);
        return -1;
    }

    return 0;
}

int Core::initCommandsInterface() {
    try {
        mCommandsInterface = new CommandsInterface(mIOService);

    } catch (const Exception &e) {
        mLog.logException("commands interface", e);
        return -1;

    } catch (const std::exception &e) {
        mLog.logException("commands interface", e);
        return -1;
    }

    return 0;
}

int Core::initCommunicator(const json &conf) {
    try {
        mCommunicator = new Communicator(
            mIOService, mSettings->interface(&conf), mSettings->port(&conf));

    } catch (const Exception &e) {
        mLog.logException("communicator", e);
        return -1;

    } catch (const std::exception &e) {
        mLog.logException("communicator", e);
        return -1;
    }

    return 0;
}

void Core::cleanupMemory() {
    if (mSettings != nullptr) {
        delete mSettings;
    }

    if (mCommunicator != nullptr) {
        delete mCommunicator;
    }

    if (mCommandsInterface != nullptr) {
        delete mCommandsInterface;
    }
}

void Core::zeroPointers() {
    mSettings = nullptr;
    mCommunicator = nullptr;
    mCommandsInterface = nullptr;
}
