#############################################################
#
# Copyright 2015 F4E | European Joint Undertaking for ITER 
#  and the Development of Fusion Energy ('Fusion for Energy')
#
# Licensed under the EUPL, Version 1.1 or - as soon they 
# will be approved by the European Commission - subsequent  
# versions of the EUPL (the "Licence"); 
# You may not use this work except in compliance with the 
# Licence. 
# You may obtain a copy of the Licence at: 
#  
# http://ec.europa.eu/idabc/eupl
#
# Unless required by applicable law or agreed to in 
# writing, software distributed under the Licence is 
# distributed on an "AS IS" basis, 
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
# express or implied. 
# See the Licence for the specific language governing 
# permissions and limitations under the Licence. 
#
# $Id: Makefile.inc 3 2012-01-15 16:26:07Z aneto $
#
#############################################################
OBJSX=PyGAM.x PyArgument.x

PACKAGE=Components/GAMs

ROOT_DIR=../../
MAKEDEFAULTDIR=$(MARTe2_DIR)/MakeDefaults
include $(MAKEDEFAULTDIR)/MakeStdLibDefs.$(TARGET)

INCLUDES += -I.
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L0Types
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L1Portability
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L2Objects
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L3Streams
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L4Messages
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L4Configuration
INCLUDES += -I$(MARTe2_DIR)/Source/Core/BareMetal/L5GAMs
INCLUDES += -I$(MARTe2_DIR)/Source/Core/Scheduler/L1Portability
INCLUDES += -I$(MARTe2_DIR)/Source/Core/Scheduler/L3Services
INCLUDES += -I$(MARTe2_DIR)/Source/Core/Scheduler/L4Messages

CPPFLAGS += -Wno-unused-function -Wno-error

##############################################################
####THE FOLLOWING DEFINITIONS MUST BE CHANGED IN ORDER TO ADAPT 
####TO THE INSTALLED PYTHON (>=3.7)
# Python header file folder
INCLUDES += -I/usr/include/python3.9/
INCLUDES += -I/usr/local/lib64/python3.9/site-packages/numpy/core/include
# Python interpreter folder
PYINTERPRETER = .

###############################################################


include Makefile-jcpp

# Linker flags requested by python3.7-config
LIBRARIES += -L$(PYINTERPRETER) -lpython3.9 -lutil -Xlinker -export-dynamic

all: $(OBJS) $(SUBPROJ) \
	$(BUILD_DIR)/PyGAM$(DLLEXT)
	    echo  $(OBJS)

include $(MAKEDEFAULTDIR)/MakeStdLibRules.$(TARGET)
#COMPILER=clang++
#CCOMPILER=clang
#LINKER=clang++

CPPFLAGS += -fpermissive -Wno-all -Wno-extra