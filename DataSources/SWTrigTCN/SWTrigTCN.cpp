
/*---------------------------------------------------------------------------*/
/*                         Standard header includes                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                         Project header includes                           */
/*---------------------------------------------------------------------------*/
#include "AdvancedErrorManagement.h"
#include "CLASSMETHODREGISTER.h"
#include "SWTrigTCN.h"
#include <mdsobjects.h>
#include <stdio.h>
#include <tcn.h>

/*---------------------------------------------------------------------------*/
/*                           Static definitions                              */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*                           Method definitions                              */
/*---------------------------------------------------------------------------*/
namespace MARTe {

SWTrigTCN::SWTrigTCN():  DataSourceI()
{
    absStartTime = relStartTime = 0;
    for(uint32 trigIdx = 0; trigIdx < MAX_TRIGGERS; trigIdx++)
    {
	trigIdxs[trigIdx] = -1;
    }
    absStartTimeNode = NULL;
    relStartTimeNode = NULL;
 }

SWTrigTCN::~SWTrigTCN() 
{
    if(absStartTimeNode) delete absStartTimeNode;
    if(relStartTimeNode) delete relStartTimeNode;
}
 
bool SWTrigTCN::AllocateMemory() {
      return true;
}

uint32 SWTrigTCN::GetNumberOfMemoryBuffers() {
    return 1u;
}

bool SWTrigTCN::GetSignalMemoryBuffer(const uint32 signalIdx, const uint32 bufferIdx, void*& signalAddress) {
    if(signalIdx != 0)
    {
        REPORT_ERROR(ErrorManagement::FatalError, "More than one signal defined in SWTRigTCN");
        return false;
    }
    signalAddress = reinterpret_cast<void *>(&inputBits);
    return true;
}

const char8* SWTrigTCN::GetBrokerName(StructuredDataI& data, const SignalDirection direction) {
    const char8* brokerName = "";
    if (direction == OutputSignals) {
            brokerName = "MemoryMapSynchronisedOutputBroker";
    }
    return brokerName;
}

bool SWTrigTCN::GetInputBrokers(ReferenceContainer& inputBrokers, const char8* const functionName, void* const gamMemPtr) {
    return false;
}

bool SWTrigTCN::IsSupportedBroker(const SignalDirection direction, const uint32 functionIdx, const uint32 functionSignalIdx, const char8* const brokerClassName)
{
     return true;
}

bool SWTrigTCN::GetOutputBrokers(ReferenceContainer& outputBrokers, const char8* const functionName, void* const gamMemPtr) {
  
    bool ok = true;

    ReferenceT<MemoryMapSynchronisedOutputBroker> broker("MemoryMapSynchronisedOutputBroker");
    ok = broker.IsValid();
    if (ok) {
        ok = broker->Init(OutputSignals, *this, functionName, gamMemPtr);
    }

    if (ok) {
	ok = outputBrokers.Insert(broker);
    }
   return ok;
}


bool SWTrigTCN::Synchronise() {
    ErrorManagement::ErrorType err;
    for (size_t i = 0; i < MAX_TRIGGERS; i++)
    {
	if(trigIdxs[i] >= 0)
	{
	    if(	  (trigEdges[i] == ANY_EDGE && (prevInputBits & (1 << trigIdxs[i])) != (inputBits & (1 << trigIdxs[i])))
		||(trigEdges[i] == RISING_EDGE && (prevInputBits & (1 << trigIdxs[i])) < (inputBits & (1 << trigIdxs[i])))	//0 < 1
		||(trigEdges[i] == FALLING_EDGE && (prevInputBits & (1 << trigIdxs[i])) > (inputBits & (1 << trigIdxs[i]))))
	    {
		hpn_timestamp_t currTime;
		if(tcn_get_raw_time(&currTime) != TCN_SUCCESS)
		{		
       		    REPORT_ERROR(ErrorManagement::FatalError, "Cannot get TCN time");
        	    return false;
		}
		char buf[ISO_8601_LEN];
		printf("Current Time:\n%s\n", tcn_strftime(currTime, buf, ISO_8601_LEN));

		if(absStartTime == 0)
		{
		    
		    try {
		    	absStartTime = absStartTimeNode->getData()->getLong();
				printf("LETTO absStartTime");
		    	relStartTime = relStartTimeNode->getData()->getDouble();
		    }catch(MDSplus::MdsException &exc)
		    {
      		    	REPORT_ERROR(ErrorManagement::FatalError, "Cannot get Start time %s, %s, %s", exc.what(), absStartTimeNode->getPath(), relStartTimeNode->getPath());
        	    	return false;
		    }
		}
		double currEventTime = (currTime - absStartTime)/1E9 + relStartTime;
printf("TEMPI: %lu %lu %f %f\n", (unsigned long)currTime, (unsigned long)absStartTime, relStartTime, currEventTime);

		MDSplus::Data *timeD = new MDSplus::Float64(currEventTime);
		MDSplus::Event::setEvent(trigEventNames[i].Buffer(), timeD);
		MDSplus::deleteData(timeD);
	    }
	}
	prevInputBits = inputBits;
    }
    return true;
}

 
/*lint -e{715}  [MISRA C++ Rule 0-1-11], [MISRA C++ Rule 0-1-12]. Justification: NOOP at StateChange, independently of the function parameters.*/
bool SWTrigTCN::PrepareNextState(const char8* const currentStateName, const char8* const nextStateName) 
{
    bool ok = true;
    return ok;
}

bool SWTrigTCN::Initialise(StructuredDataI& data) {
    REPORT_ERROR(ErrorManagement::Debug, "Initialise");
    bool ok = DataSourceI::Initialise(data);
    if (ok) {
	StreamString treeName;
        ok = data.Read("Tree", treeName);
	if(!ok)
	{
	    REPORT_ERROR(ErrorManagement::ParametersError, "Tree name shall be defined");
	    return false;
	}
	int shot;
	ok = data.Read("Shot", shot);
	if(!ok)
	{
	    REPORT_ERROR(ErrorManagement::ParametersError, "Shot  shall be defined");
	    return false;
	}
	StreamString relPath, absPath;
	ok = data.Read("RelTimePath", relPath);
	if(!ok)
	{
	    REPORT_ERROR(ErrorManagement::ParametersError, "RelTimePath shall be defined");
	    return false;
	}
	ok = data.Read("AbsTimePath", absPath);
	if(!ok)
	{
	    REPORT_ERROR(ErrorManagement::ParametersError, "RelTimePath shall be defined");
	    return false;
	}
	try {
	    MDSplus::Tree *t = new MDSplus::Tree(treeName.Buffer(), shot);
	    relStartTimeNode = t->getNode(relPath.Buffer());
	    absStartTimeNode = t->getNode(absPath.Buffer());
	}catch(MDSplus::MdsException &exc)
	{
	    REPORT_ERROR(ErrorManagement::ParametersError, "Cannot get Relatinve or Absolute node references");
	    return false;
	}
	char nameBuf[64];
	for(int i = 0; i < MAX_TRIGGERS; i++)
	{
	    sprintf(nameBuf, "TrigIdx%d", i+1);
	    ok = data.Read(nameBuf, trigIdxs[i]);
printf("TRIGIDX[%d]=%d\n", i, trigIdxs[i]);
	    if(!ok) trigIdxs[i] = -1;
	    if(trigIdxs[i] >= 0)
	    {
		sprintf(nameBuf, "EvName%d", i+1);
		ok = data.Read(nameBuf, trigEventNames[i]);
		if(!ok)
		{
	    	    REPORT_ERROR(ErrorManagement::ParametersError, "EvName%d shall be specified", i+1);
	      	    return false;
		}
		sprintf(nameBuf, "TrigEdge%d", i+1);
		StreamString trigEdgeName;
		ok = data.Read(nameBuf, trigEdgeName);
		if(!ok)
		    trigEdges[i] = RISING_EDGE;
		else
		{
		    if(trigEdgeName == "RISING")
			trigEdges[i] = RISING_EDGE;
		    else if(trigEdgeName == "FALLING")
			trigEdges[i] = FALLING_EDGE;
		    else if (trigEdgeName == "ANY")
			trigEdges[i] = ANY_EDGE;
		    else
		    {
	    	    	REPORT_ERROR(ErrorManagement::ParametersError, "TrigEdge%d shall be either RISING, FALLING or ANY", i+1);
	      	    	return false;
		    }
		}
	    }
	}
	ok = true;
    }
    return ok;
}

		

bool SWTrigTCN::SetConfiguredDatabase(StructuredDataI& data) 
{
    REPORT_ERROR(ErrorManagement::Debug, "SetConfiguredDatabase");
    bool ok = DataSourceI::SetConfiguredDatabase(data);
    //Check signal properties and compute memory
    uint32 nOfSignals = 0u;  
    if (ok) { // Check that only one GAM is Connected to the MDSReaderNS
        uint32 auxNumberOfFunctions = GetNumberOfFunctions();
        ok = (auxNumberOfFunctions == 1u);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Exactly one Function allowed to interact with this SWTrigTCN DataSource. Number of Functions = %u", auxNumberOfFunctions);
        }
    }
    if (ok) { //read number of nodes per function numberOfNodeNames
        //0u (second argument) because previously it is checked
        ok = GetFunctionNumberOfSignals(OutputSignals, 0u, nOfSignals);        //0u (second argument) because previously it is checked
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "GetFunctionNumberOfSignals() returned false");
        }
	if (ok) {
            ok = (nOfSignals == 1u);
            if (!ok) {
                REPORT_ERROR(ErrorManagement::ParametersError, "The number of signals shall be exactly one");
            }
	}
    }

    if(ok)
    {
	uint32 nSamples;
    	ok = GetFunctionSignalSamples(OutputSignals, 0u, 0, nSamples);
        if (ok) {
             ok = (nSamples == 1u);
        }
        if (!ok) {
             REPORT_ERROR(ErrorManagement::ParametersError, "The number of samples for output signals shall be exactly 1");
 	}
    }
    TypeDescriptor type;
    if (ok) { //read the type specified in the configuration file 
        type = GetSignalType(0);
        ok = !(type == InvalidType);
        if (!ok) {
            REPORT_ERROR(ErrorManagement::ParametersError, "Invalid type for Counter");
        }
        ok = (type == UnsignedInteger32Bit || type == SignedInteger32Bit);
	if(!ok)
	{
            REPORT_ERROR(ErrorManagement::ParametersError, "Counter type shall be either UnsignedInteger32Bit or SignedInteger32Bit");
	}
    }

    if(ok)
    {
    	int retval = tcn_register_device("/etc/opt/codac-6.1/tcn/nisync-6683h.xml");
    	if (retval != TCN_SUCCESS) 
    	{
       	    REPORT_ERROR(ErrorManagement::FatalError, "Cannot register TCN device");
       	    ok = false;
	}
    }
    if(ok)
    {
	int retval = tcn_init();
	if (retval != TCN_SUCCESS) 
   	{
       	    REPORT_ERROR(ErrorManagement::FatalError, "Cannot initialize TCN ");
       	    ok = false;
	}
    }
    prevInputBits = 0;
    return ok;
}




uint32 SWTrigTCN::GetNumberOfBuffers() const {
    return 1;
}


CLASS_REGISTER(SWTrigTCN, "1.0")
}

