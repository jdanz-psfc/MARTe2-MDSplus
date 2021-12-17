
/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "CLASSMETHODREGISTER.h"
#include "UdpIn.h"
#include <mdsobjects.h>
#include <stdio.h>

//#define DEBUG 1


/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {
static const int32 FILE_FORMAT_BINARY = 1;
static const int32 FILE_FORMAT_CSV = 2;

UdpIn::UdpIn() :
        DataSourceI(),
        MessageI() {
	dataSourceMemory = NULL_PTR(char8 *);
	rawChans = NULL_PTR(int16 *);
	offsets = NULL_PTR(uint32 *);
	nOfChannels = NULL_PTR(uint32 *);
	sigTypes = NULL_PTR(TypeDescriptor *);
	nOfSignals = 0;
	totChannels = 0;
	port = 0;
	sock = 0;
	period = 0;
}

UdpIn::~UdpIn() {
//Free allocated buffers
    if (dataSourceMemory != NULL_PTR(char8 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(dataSourceMemory));
    }
    if (offsets != NULL_PTR(uint32 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(offsets));
    }    
    if (nOfChannels != NULL_PTR(uint32 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(nOfChannels));
    }
    if (rawChans != NULL_PTR(int16 *)) {
        GlobalObjectsDatabase::Instance()->GetStandardHeap()->Free(reinterpret_cast<void *&>(rawChans));
    }
    if(sigTypes != NULL_PTR(TypeDescriptor *))
      delete [] sigTypes;
}

bool UdpIn::AllocateMemory() {
    return true;
}

uint32 UdpIn::GetNumberOfMemoryBuffers() {
    return 1u;
}

bool UdpIn::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    bool ok = (dataSourceMemory != NULL_PTR(char8 *));
    if (ok) {
        /*lint -e{613} dataSourceMemory cannot be NULL here*/
        char8 *memPtr = &dataSourceMemory[offsets[signalIdx]];
        signalAddress = reinterpret_cast<void *&>(memPtr);
    }
    return ok;
}

const char8* UdpIn::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == InputSignals) {
            brokerName = "MemoryMapSynchronisedInputBroker";
    }
    return brokerName;
}

bool UdpIn::GetOutputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
    return false;
}

bool UdpIn::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8* const brokerClassName)
{
     return true;
}

bool UdpIn::GetInputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
  
    bool ok = true;

    ReferenceT<MemoryMapSynchronisedInputBroker> broker("MemoryMapSynchronisedInputBroker");
    ok = broker.IsValid();
    if (ok) {
        ok = broker->Init(InputSignals, *this, functionName, gamMemPtr);
    }

    if (ok) {
	ok = inputBrokers.Insert(broker);
    }
   return ok;
}

bool UdpIn::Synchronise() {
  
#ifdef DEBUG
    if((counter % 10000) == 0) printf("Synchronise...%d \n", port);
#endif
    bool ok = true;

    int32 nBytes;
    socklen_t addrLen;
 /*   nBytes = recvfrom(sock, rawChans, (nOfSignals - 1)*sizeof(int16), 0, (struct sockaddr *)&senderAddr, &addrLen);
    if(nBytes < 0)
    {
    	REPORT_ERROR(ErrorManagement::FatalError,"Error receiving usp datagram");
        return 0;
    } */

    do  {
    	nBytes = recvfrom(sock, rawChans, (nOfSignals - 1)*sizeof(int16), MSG_DONTWAIT, (struct sockaddr *)&senderAddr, &addrLen);
    }
    while(nBytes<= 0);
 /*   if(nBytes < 0)
    {
    	REPORT_ERROR(ErrorManagement::FatalError,"Error receiving usp datagram");
        return 0;
    }*/
#ifdef DEBUG
    if((counter % 10000) == 0) printf("Received %d bytes: %d\n", nBytes, rawChans[0]);
#endif
    if(nBytes < totChannels * sizeof(int16))
    {
    	REPORT_ERROR(ErrorManagement::FatalError,"Wrong datagram dimension expected");
        return 0;
    }
    *reinterpret_cast<uint32 *>(dataSourceMemory) = (uint32)(counter * period * 1E6);
    uint32 chanIdx = 0;
    for (uint32 n = 1u; (n < nOfSignals) && (ok); n++) {
        TypeDescriptor type = sigTypes[n];
	for(uint32 currChan = 0; currChan < nOfChannels[n]; currChan++)
	{
	    if(type == Float32Bit)
	    {
		*reinterpret_cast<float32 *>(&dataSourceMemory[offsets[n] + currChan * sizeof(float32)]) = rawChans[chanIdx++];
    	    }
    	    if(type == Float64Bit)
    	    {
	    	*reinterpret_cast<float64 *>(&dataSourceMemory[offsets[n]] + currChan * sizeof(float64)) = rawChans[chanIdx++];
    	    }
    	    else if (type == SignedInteger16Bit)
    	    {
	    	*reinterpret_cast<int16 *>(&dataSourceMemory[offsets[n]] + currChan * sizeof(int16)) = rawChans[chanIdx++];
    	    }
    	    else if (type == SignedInteger32Bit)
    	    {
	        *reinterpret_cast<int32 *>(&dataSourceMemory[offsets[n]] + currChan * sizeof(int32)) = rawChans[chanIdx++];
    	    }
    	    else if (type == UnsignedInteger16Bit)
    	    {
	        *reinterpret_cast<uint16 *>(&dataSourceMemory[offsets[n]] + currChan * sizeof(uint16)) = rawChans[chanIdx++];
    	    }
    	    else if (type == UnsignedInteger32Bit)
    	    {
	        *reinterpret_cast<uint32 *>(&dataSourceMemory[offsets[n]] + currChan * sizeof(uint32)) = rawChans[chanIdx++];
    	    }
	}
   }
   counter++;
    
  return ok;
}
 
 
 
/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool UdpIn::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) {
    return true;
}

bool UdpIn::Initialise(StructuredDataI& data) {
    bool ok = DataSourceI::Initialise(data);
     if(ok) {
        ok = data.Read("Period", period);
       if (!ok) {
            REPORT_ERROR(ErrorManagement::Information, "Period shall be specified");
        }
    }
     if(ok) {
        ok = data.Read("Port", port);
       if (!ok) {
            REPORT_ERROR(ErrorManagement::Information, "Port shall be specified ");
        }
    }

    if(ok)
    {
	ok = data.MoveRelative("Signals");
    	if(!ok) {
		REPORT_ERROR(ErrorManagement::ParametersError,"Signals node Missing.");
	}
    }
    if(ok)
    {
    	nOfSignals = data.GetNumberOfChildren();
    	ok = nOfSignals >= 2;
	if(!ok) {
		REPORT_ERROR(ErrorManagement::ParametersError,"At least two signals shall be defined.");
	}
    }
    data.MoveToAncestor(1u);

   return ok;
}


bool UdpIn::SetConfiguredDatabase(StructuredDataI& data) {
	
    uint32 totalSignalMemory = 0;
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    if (ok) { // Check that only one GAM is Connected to the MDSReaderNS
        uint32 auxNumberOfFunctions = GetNumberOfFunctions();
        ok = (auxNumberOfFunctions == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Exactly one Function allowed to interact with this UdpIn DataSource. Number of Functions = %u",
                         auxNumberOfFunctions);
        }
    }
    if (ok) { //read number of nodes per function numberOfNodeNames
        //0u (second argument) because previously it is checked
        ok = GetFunctionNumberOfSignals(InputSignals, 0u, nOfSignals);        //0u (second argument) because previously it is checked
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "GetFunctionNumberOfSignals() returned false");
        }
    }


    if(ok) //Check Time signal
    {
    	StreamString timeSignalName;
    	ok = GetSignalName(0, timeSignalName);
	if(!ok || timeSignalName != "Time")
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "The first signal shall be Time");
	    ok = false;
        }
	if(ok)
	{
	    uint32 nEls;
	    GetSignalNumberOfElements(0, nEls);
	    if(nEls != 1)
	    {
           	REPORT_ERROR(ErrorManagement::ParametersError, "The first signal shall have one element");
	    	ok = false;
 	    }
	    TypeDescriptor firstType = GetSignalType(0);
	    if ((firstType != UnsignedInteger32Bit) && (firstType != SignedInteger32Bit))
 	    {
           	REPORT_ERROR(ErrorManagement::ParametersError, "Time signal shall be either int32 of uint32");
	    	ok = false;
 	    }
	}
    } 

    if(ok)
    {
   	offsets  = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));
    	nOfChannels  = reinterpret_cast<uint32 *>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(nOfSignals * sizeof(int32)));
    	sigTypes = new TypeDescriptor[nOfSignals];
        //lint -e{613} Possible use of null pointer. type previously allocated (see previous line).
        for (uint32 i = 0u; (i < nOfSignals) && ok; i++) {
            sigTypes[i] = GetSignalType(i);
	}
    	memset(offsets, 0, nOfSignals * sizeof(int32));
    	memset(nOfChannels, 0, nOfSignals * sizeof(int32));
    	totChannels = 0;
    	uint32 nBytes;
	for (uint32 n = 0u; (n < nOfSignals) && ok; n++) {
   
	    offsets[n] = totalSignalMemory;
	    if(ok)
	    {
		GetSignalNumberOfElements(n, nOfChannels[n]);
		if(n > 0) //Time is not considered
		    totChannels += nOfChannels[n];
		if(!ok)  {
            	    REPORT_ERROR(ErrorManagement::ParametersError,
                         "Error getting number of elements for signal %d", n);
		}
	    }
	    if(ok)
	    {
		ok = GetSignalByteSize(n, nBytes);
	    }
	    if(ok)
	    {
           	sigTypes[n] = GetSignalType(n);
	      	bool cond1 = (sigTypes[n] == UnsignedInteger64Bit);
	      	bool cond2 = (sigTypes[n] == UnsignedInteger32Bit);
	      	bool cond3 = (sigTypes[n] == UnsignedInteger16Bit);
	      	bool cond4 = (sigTypes[n] == SignedInteger32Bit);
	      	bool cond5 = (sigTypes[n] == SignedInteger64Bit);
	      	bool cond6 = (sigTypes[n] == SignedInteger16Bit);
	      	bool cond7 = (sigTypes[n] == Float32Bit);
	      	bool cond8 = (sigTypes[n] == Float64Bit);
	     	ok = cond1 || cond2 || cond3 || cond4 || cond5 || cond6 || cond7 || cond8;
	      	if (!ok) {
                	REPORT_ERROR(ErrorManagement::ParametersError, "Unsupported type. Possible time types are: uint64, int64, uin32 or int32\n");
		}
	    }
	    totalSignalMemory += nBytes;
	}
    } 
    if(ok)
    {
      	dataSourceMemory = reinterpret_cast<char8*>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(totalSignalMemory));
      	rawChans = reinterpret_cast<int16*>(GlobalObjectsDatabase::Instance()->GetStandardHeap()->Malloc(totChannels * sizeof(int16)));
    }
    if (ok) {
        for (uint32 n = 0u; (n < nOfSignals) && ok; n++) {
            uint32 nSamples;
            ok = GetFunctionSignalSamples(InputSignals, 0u, n, nSamples);
            if (ok) {
                ok = (nSamples == 1u);
            }
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples shall be exactly 1");
            }
        }
    }
    counter = 0;
    if(ok)
    {
   	if((sock=socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  	{
     	    REPORT_ERROR(ErrorManagement::ParametersError, "Cannot open UDP Socket");
   	    ok = false;
  	}
        int disable = 1;
	if(setsockopt(sock, SOL_SOCKET, SO_NO_CHECK, 
(void *)&disable, sizeof(disable)) < 0)
  	{
     	    REPORT_ERROR(ErrorManagement::ParametersError, "Cannot open UDP Socket XXX");
   	    ok = false;
  	}
   }
    if(ok)
    {
 	memset(&addr, 0, sizeof(addr));
  	addr.sin_family = AF_INET;
  	addr.sin_port = htons(port);
  	addr.sin_addr.s_addr = htonl(INADDR_ANY);
  	if(bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  	{
      	    perror("ERROR BINDING SOCKET\n");
      	    ok = false;
  	}
    } 
    return ok;   
}


uint32 UdpIn::GetNumberOfBuffers() const {
    return 1;
}

CLASS_REGISTER(UdpIn, "1.0")
}

