if [ -z ${MARTe2_DIR+x} ]; then echo "Please set the MARTe2_DIR environment variable"; exit; fi
if [ -z ${MARTe2_Components_DIR+x} ]; then echo "Please set the MARTe2_Components_DIR environment variable"; exit; fi

if [ -z ${MARTE_DIR+x} ]; then export MARTE_DIR=.; fi;


LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/.
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/GAMs/FunctionGeneratorGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/GAMs/SimulinkInterfaceGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/GAMs/TestGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/GAMs/Raptor/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/GAMs/equinoxGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/GAMs/FFTGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/GAMs/MathExpressionGAM
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/GAMs/SpiderCalGAM
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/GAMs/PyGAM
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/opt/anaconda3/lib/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/DataSources/mcc118
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/DataSources/StreamOut
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/DataSources/StreamIn
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/DataSources/ConsoleOut
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/DataSources/MDSReaderNS
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTE_DIR/../Build/armv8-linux/Components/Interfaces/MDSEventManager
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_DIR/Build/armv8-linux/Core/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/EpicsDataSource/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/LinuxTimer/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/LoggerDataSource/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/NI6259/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/NI6368/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/SDN/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/FileDataSource/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/RealTimeThreadSynchronisation/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/RealTimeThreadAsyncBridge/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/UDP/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/GAMs/IOGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/GAMs/SSMGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/GAMs/WaveformGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/GAMs/BaseLib2GAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/GAMs/ConversionGAM/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/Interfaces/BaseLib2Wrapper/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/Interfaces/EPICSInterface/
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$MARTe2_Components_DIR/Build/armv8-linux/Components/DataSources/MDSWriter


#Disable CPU speed changing
#cpupower frequency-set --governor performance
#service cpuspeed stop

echo $LD_LIBRARY_PATH
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH
#cgdb --args ../Build/linux/Startup/Playground.ex $1 $2 $3 $4
#strace -o/tmp/strace.err ../Build/linux/Startup/Playground.ex $1 $2  $3 $4

$MARTE_DIR/../Build/armv8-linux/Startup/Playground.ex $1 $2 $3 $4 
#valgrind $MARTE_DIR/../Build/armv8-linux/Startup/Playground.ex $1 $2 $3 $4 
