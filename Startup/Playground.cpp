/**
 * @file Playground.cpp
 * @brief Source file for class Playground
 * @date Nov 8, 2016 TODO Verify the value and format of the date
 * @author aneto TODO Verify the name and format of the author
 *
 * @copyright Copyright 2015 F4E | European Joint Undertaking for ITER and
 * the Development of Fusion Energy ('Fusion for Energy').
 * Licensed under the EUPL, Version 1.1 or - as soon they will be approved
 * by the European Commission - subsequent versions of the EUPL (the "Licence")
 * You may not use this work except in compliance with the Licence.
 * You may obtain a copy of the Licence at: http://ec.europa.eu/idabc/eupl
 *
 * @warning Unless required by applicable law or agreed to in writing, 
 * software distributed under the Licence is distributed on an "AS IS"
 * basis, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the Licence permissions and limitations under the Licence.

 * @details This source file contains the definition of all the methods for
 * the class Playground (public, protected, and private). Be aware that some 
 * methods, such as those inline could be defined on the header file, instead.
 */

#define DLL_API

/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "ClassRegistryDatabase.h"
#include "ClassRegistryItem.h"
#include "ConfigurationDatabase.h"
#include "File.h"
#include "GlobalObjectsDatabase.h"
#include "Object.h"
#include "ObjectRegistryDatabase.h"
#include "ProcessorType.h"
#include "RealTimeApplication.h"
#include "Reference.h"
#include "ReferenceT.h"
#include "StreamString.h"
#include "StandardParser.h"

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

void PlaygroundErrorProcessFunction(const MARTe::ErrorManagement::ErrorInformation &errorInfo, const char * const errorDescription) {
    MARTe::StreamString errorCodeStr;
    MARTe::ErrorManagement::ErrorCodeToStream(errorInfo.header.errorType, errorCodeStr);
    printf("[%s - %s:%d]: %s\n", errorCodeStr.Buffer(), errorInfo.fileName, errorInfo.header.lineNumber, errorDescription);
}

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
int main(int argc, char **argv) {
    using namespace MARTe;
    ProcessorType::SetDefaultCPUs(0x1);
    SetErrorProcessFunction(&PlaygroundErrorProcessFunction);
    if (argc != 5) {
        printf("Arguments are -f FILENAME -s FIRST_STATE | -m MSG_DESTINATION:MSG_FUNCTION\n");
        return -1;
    }
    StreamString argv1 = argv[1];
    StreamString argv3 = argv[3];
    StreamString filename;
    StreamString firstState;
    StreamString messageArgs;

    if (argv1 == "-f") {
        filename = argv[2];
    }
    else if (argv3 == "-f") {
        filename = argv[4];
    }
    else {
        printf("Arguments are -f FILENAME -s FIRST_STATE | -m MSG_DESTINATION:MSG_FUNCTION\n");
        return -1;
    }

    if (argv1 == "-s") {
        firstState = argv[2];
    }
    else if (argv3 == "-s") {
        firstState = argv[4];
    }
    else if (argv1 == "-m") {
        messageArgs = argv[2];
    }
    else if (argv3 == "-m") {
        messageArgs = argv[4];
    }
    else {
        printf("Arguments are -f FILENAME -s FIRST_STATE | -m MSG_DESTINATION:MSG_FUNCTION\n");
        return -1;
    }

    BasicFile f;
    bool ok = f.Open(filename.Buffer(), BasicFile::ACCESS_MODE_R);
    if (ok) {
        f.Seek(0);
    }
    else {
        printf("Failed to open file %s\n", argv[1]);
    }
    ConfigurationDatabase cdb;
    StreamString err;
    if (ok) {
        StandardParser parser(f, cdb, &err);
        ok = parser.Parse();
    }
    if (!ok) {
        printf("Failed to parse %s\n", err.Buffer());
    }

    ObjectRegistryDatabase *objDb = NULL;
    if (ok) {
        objDb = ObjectRegistryDatabase::Instance();
        objDb->Initialise(cdb);
    }
    if (!ok) {
        printf("Failed to load godb\n");
    }

    if (ok) {
        uint32 nOfObjs = objDb->Size();
        uint32 n;
        bool found = false;
        for (n = 0u; (n < nOfObjs) && (!found); n++) {
            ReferenceT < RealTimeApplication > rtApp = objDb->Get(n);
            found = rtApp.IsValid();
            if (found) {
                ok = rtApp->ConfigureApplication();
                if (!ok) {
                    printf("Failed to load Configure RealTimeApplication\n");
                    return -1;
                }
                if (firstState.Size() > 0) {
                    if (ok) {
                        ok = rtApp->PrepareNextState(firstState.Buffer());
                    }
                    if (ok) {
                        rtApp->StartNextStateExecution();
                    }
                }
                else {
                    ReferenceT<Message> message(new Message());
                    ConfigurationDatabase msgConfig;
                    StreamString destination;
                    StreamString function;
                    char8 term;
                    messageArgs.Seek(0LLU);
                    ok = messageArgs.GetToken(destination, ":", term);
                    if (ok) {
                        ok = messageArgs.GetToken(function, ":", term);
                    }
                    if (!ok) {
                        printf("Message format is MSG_DESTINATION:MSG_FUNCTION\n");
                    }
                    msgConfig.Write("Destination", destination.Buffer());
                    msgConfig.Write("Function", function.Buffer());
                    if (ok) {
                        ok = message->Initialise(msgConfig);
                    }
                    if (ok) {
                        MessageI::SendMessage(message);
                    }
                }
            }
        }
    }
    f.Close();
    if (ok) {
        while (1) {
            Sleep::Sec(1.0);
        }
    }

    return 0;
}

