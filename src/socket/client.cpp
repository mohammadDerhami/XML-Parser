#include "client.h"

/**
 *
 * Implementation for "Client" class
 *
 */

/*Resets the client status for reuse by clearing results and data flags.*/
void Client::reset()
{
    std::lock_guard<std::mutex> lock(clientMtx);

    resultReady = false;
    result.clear();

    dataReady = false;
    inputData.clear();

}

