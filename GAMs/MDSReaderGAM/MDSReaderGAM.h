#ifndef  GAMS_MDSREADERGAM_H_
#define  GAMS_MDSREADERGAM_H_


#define MDSREADERGAM_INT8 1
#define MDSREADERGAM_UINT8 2
#define MDSREADERGAM_INT16 3
#define MDSREADERGAM_UINT16 4
#define MDSREADERGAM_INT32 5
#define MDSREADERGAM_UINT32 6
#define MDSREADERGAM_INT64 7
#define MDSREADERGAM_UINT64 8
#define MDSREADERGAM_FLOAT32 9
#define MDSREADERGAM_FLOAT64 10



/*---------------------------------------------------------------------------*/
/*                        Standard header includes                           */
/*---------------------------------------------------------------------------*/

/*lint -u__cplusplus This is required as otherwise lint will get confused after including this header file.*/
#include "mdsobjects.h"
/*lint -D__cplusplus*/

/*---------------------------------------------------------------------------*/
/*                        Project header includes                            */
/*---------------------------------------------------------------------------*/
#include "GAM.h"
#include "MessageI.h"
#include "StructuredDataI.h"
#include "StreamString.h"

/*---------------------------------------------------------------------------*/
/*                           Class declaration                               */
/*---------------------------------------------------------------------------*/

namespace MARTe {

/**
 * @brief MDSReaderGAM is a GAM which allows to read data from segmented and non segmented nodes a MDSplus tree.
 * @details MDSReaderGAM is a GAM which takes data from MDSPlus nodes (as many as desired) and publishes it on a real time application.
 *
 * The MDSReaderGAM can either interpolate, decimate or take the raw data, as it is, from the tree depending on the parameter called "DataManagement" which is given
 * in the configuration file. This data source is intended mainly for the following two use cases:
 * <ul>
 * <li>Simulation, where the inputs are read from a pulse file.</li>
 * <li>Reference waveforms, normally defined in non segmented nodes as a signal defining few X and Y points and assuming interpolated values in between.</li>
 * </ul>
 * This GAM has a single input that is the current (relatime) time expressed in microseconds. it provides an arbitrary number of outputs whose value is the interpolated
 * value of the corresponding waveform. 
 * 
 * MDSReaderGAM can handle as many outputs as desired. Each node can have their on data type. If the current input time is less than the time associated with fthe
 first sample, or if it is greater than thre time associated with the last time, the output will be 0.
 * The supported types for input signal are
 * <ul>
 * <li>uint32</li>
 * <li>int32</li>
 * <li>uint64</li>
 * <li>int64</li>
* </ul>
 * The supported types for the outputs are:
 * <ul>
 * <li>uint8</li>
 * <li>int8</li>
 * <li>uint16</li>
 * <li>int16</li>
 * <li>uint32</li>
 * <li>int32</li>
 * <li>uint64</li>
 * <li>int64</li>
 * <li>float32</li>
 * <li>float64</li>
 * </ul>
 * 
 * Nodes are assumed to be signals, i.e. they bring also timebase information. The timebases can be different from nodes to nodes, except for the raw case (0) in DataManaegment
 * where they are assumed to represent a signal at exactly the same frequency of the actual runtime timebase. 
 * The supported type for the timebase are:
 * <ul>
 * <li>uint32</li>
 * <li>int32</li>
 * <li>uint64</li>
 * <li>int64</li>
 * </ul>
 *
 *The configuration syntax is (names and signal quantity are only given as an example):
 *<pre>
 * +MDSReaderGAM_0 = {
 *     Class = MDSReaderGAM
 *     TreeName = "test_tree" //Compulsory. Name of the MDSplus tree.
 *     ShotNumber = 1 //Compulsory. 0 --> last shot number (to use 0 shotid.sys must exist)
 *
 *     InputSignals = {
 *         Time = {
 * 	       Type = uint32   //can be uint64, int64, int32
 *         }
        OutputSignals = {
           Out1 = {
                DataExpr = "MDSplus expression"
                TimebaseExpr = "MDSplus expression"
                NumberOfDimensions = <can be either 0 or 1>
                NumberOfElements = <if NumberOfDimensions > 0>
                Type = <Any supported type>
           }     
        }
 *         ....
 *         ....
 *         ....
 *     }
 *     +Messages = { //Optional. If a message will be fired when all the declared signals have finished (i.e. current time bocomens greater than the last sample time)
 *         Class = ReferenceContainer
 *         +SignalsFinished = { //Optional, but if set, the name of the Object shall be TreeOpenedOK. If set a message containing a ConfigurationDatabase with param1=PULSE_NUMBER will be sent to the Destination, every time the Tree is successfully opened
 *             Class = Message
 *             Destination = SomeObject
 *             Function = SomeFunction
 *             Mode = ExpectsReply
 *         }
 *     }

 * }
 * </pre>
 */
class MDSReaderGAM: public GAM {
//TODO Add the macro DLL_API to the class declaration (i.e. class DLL_API MDSReaderNS)
public:
    CLASS_REGISTER_DECLARATION()

    /**
     * @brief default constructor
     */
    MDSReaderGAM    ();

    /**
     * @brief default destructor.
     */
    virtual ~MDSReaderGAM();

    /**
     * @brief Copy data from the tree nodes to the dataSourceMemory
     * @details When a node does not have more data to retrieve the dataSourceMemory is filled with 0.
     * @return true if all nodes read return false.
     */
    virtual bool Execute();

    /**
     * @brief Reads, checks and initialises the DataSource parameters
     * @details Load from a configuration file the DataSource parameters.
     * If no errors occurs the following operations are performed:
     * <ul>
     * <li>Reads tree name </li>
     * <li>Reads the shot number </li>
     * <li>Opens the tree with the shot number </li>
     * <li>Reads the real-time thread Frequency parameter.</li>
     * </ul>
     * @param[in] data is the configuration file.
     * @return true if all parameters can be read and the values are valid
     */
    virtual bool Initialise(StructuredDataI & data);

    /**
     * @brief Read, checks and initialises the Signals
     * @details If no errors occurs the following operations are performed:
     * <ul>
     * <li>Reads nodes name (could be 1 or several nodes)</li>
     * <li>Opens nodes</li>
     * <li>Gets the node type</li>
     * <li>Verifies the node type</li>
     * <li>Gets number of elements per node (or signal).
     * <li>Gets the the size of the type in bytes</li>
     * <li>Allocates memory
     * <li>Read signals and their timebases in memory
     * </ul>
     * @param[in] data is the configuration file.
     * @return true if all parameters can be read and the values are valid
     */

    virtual bool Setup();

    
  private:
    /**
     * @brief Open MDS tree
     * @details Open the treeName and copy the pointer of the object to tree variables.
     */
    bool OpenTree();


    /**
     * @brief Gets the data node for a real time cycle.
     * @brief First determine the topology of the chunk of data to be read (i.e if there is enough data in the node, if the data has holes)
     * end then decides how to copy the data.
     * @param[in] nodeNumber node number to be copied to the dataSourceMemory.
     * @return true if node data can be copied. false if is the end of the node
     */
    bool GetDataNode(const uint32 nodeNumber);

    
    /**
     * @brief Calculates values, timebase, numSamples and the (average) period
     * @return true on succeed.
     */
    bool GetNodeDataAndSamplingTime(const uint32 idx, float64 * &data, uint32 &numElements, float64 * &timebase, uint32 &numSamples,
            float64 &tDiff) const;

    /**
     * @brief Copy the same value as many times as indicated.
     * @details this function decides the type of data to copy and then calls the MDSReaderNS::CopyTheSameValue()
     * @param[in] idxNumber is the node number from which the data must be copied.
     * @param[in] numberOfTimes how many samples must be copied.
     * @param[in] samplesOffset indicates how many samples has already copied.
     */
    void CopyValue(const uint32 idxNumber, uint32 element, float64 value);

    /**
     * @brief Template functions which actually performs the copy
     * @param[in] idxNumber is the node number from which the data must be copied.
     * @param[in] numberOfTimes how many samples must be copied.
     * @param[in] samplesOffset indicates how many samples has already copied.
     */
    template<typename T>
    void CopyValueTemplate(uint32 idxNumber, uint32 element, float64 value);

    /**
     * @brief First fills a hole and then copy data from the node
     * @param[in] nodeNumber node number from where copy data.
     * @param[in] minSegment indicates the segment from where start to be copied.
     * @param[in] numberOfDiscontinuities indicates the number of times that the algorithm must be applied
     */

    /**
     * @brief Convert the the MDSplus type into MARTe type.
     */
    TypeDescriptor ConvertMDStypeToMARTeType(StreamString mdsType) const;

    bool AllNodesEnd() const;

    
    void notifySignalsEnded();
    /**
     * The name of the MDSplus tree to be opened.
     */
    StreamString treeName;

    /**
     * The MDSplus tree to be opened.
     */
    MDSplus::Tree *tree;

    /**
     * The data expressions to be managed.
     */
    StreamString *dataExpr;


    /**
     * The timebase expressions to be managed.
     */
    StreamString *timebaseExpr;


    /**
     * The number of nodes to be managed.
     */
    uint32 numberOfNodeNames;


    /**
     * In SetConfiguredDatabase() the information is modified. I.e the node name is not copied because is unknown parameter for MARTe.
     */
    ConfigurationDatabase originalSignalInformation;

    /**
     * Indicates the pulse number to open. If it is not specified -1 by default.
     */
    int32 shotNumber;

        /**
     * Input (time) type
    */
    TypeDescriptor inputType;
    
    /**
     * Number of elements that should be read each MARTe cycle. It is read from the configuration file
     */
    uint32 *numberOfElements;

    /**
     * Output signals memory.
     */
    void **outSignalsMemory;

    void *inSignalMemory;
     /**
     * Number of Elements for each signal.
     */
    uint32 *nElements;

     /**
     * Time in seconds. It indicates the time of each sample. At the beginning of each cycle currentTime = time.
     */
    float64 currentTime;

     /**
     * Total number of cycles.
     */
    uint64 numCycles;

    /**
    * Turnaround for MARTe2 inefficiency in Type constants management
    **/
    uint32 inType;
    uint32 *outTypes; 

    
   

    /**
     * Management of the data. indicates what to do with the data.
     * 1 --> linear interpolation
     * 2 --> hold last value.
     *
     */

    uint8 *dataManagement;

    /**
     * Management of the  hole (when there is no data stored).
     * 0 --> add 0
     * 1 --> hold last value
     */
     float64 *samplingTime;
 
    bool *endNode;
    float64 *nodeSamplingTime;
///GABRIELE    
    int timebaseMode; //Not used now
    float64 **signalData;
    float64 **signalTimebase;
    uint32 *numSignalSamples;
    /**
     * hold the last signalSample considered in output generation where the t was found. It is used for optimization since the time could not go back.
     */
    uint32 *lastSignalSample;
    /**
     * The messages to send when signals have terminated .
     */
    ReferenceT<Message> *signalsEndedMsg;
    uint32 nOfMessages;
    bool signalsEndedNotified;
    bool *useColumnOrder;


/*---------------------------------------------------------------------------*/
/*                        Inline method definitions                          */
/*---------------------------------------------------------------------------*/

        //std::cout << "COPY Offset: " <<  offsets[idxNumber] << std::endl; 
        uint32 convertType(TypeDescriptor type)
        {
                if (type == UnsignedInteger8Bit)
                        return MDSREADERGAM_UINT8;
                if (type == SignedInteger8Bit)
                        return MDSREADERGAM_INT8;
                if (type == UnsignedInteger16Bit)
                        return MDSREADERGAM_UINT16;
                if (type == SignedInteger16Bit)
                        return MDSREADERGAM_INT16;
                if (type == UnsignedInteger32Bit)
                        return MDSREADERGAM_UINT32;
                if (type == SignedInteger32Bit)
                        return MDSREADERGAM_INT32;
                if (type == UnsignedInteger64Bit)
                        return MDSREADERGAM_UINT64;
                if (type == SignedInteger64Bit)
                        return MDSREADERGAM_INT64;
                if (type == Float32Bit)
                        return MDSREADERGAM_FLOAT32;
                if (type == Float64Bit)
                        return MDSREADERGAM_FLOAT64;
                return 0;

        }
 };
template<typename T>
void  MDSReaderGAM::CopyValueTemplate(uint32 idxNumber, uint32 element, float64 value) {
        T *ptr = reinterpret_cast<T *>(outSignalsMemory[idxNumber]);
        ptr[element] = value;
}

}


#endif /* GAMS_MDSREADERGAM_H_ */
