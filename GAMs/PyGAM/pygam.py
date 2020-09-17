# --- MODULE CODE ---

import numpy

# variables that should remain visible in this module
global PygamRootInputDict
global PygamRootOutputDict
global PygamParameterDict
global data

data = {}
RootInputDict = {}
RootOutputDict = {}
ParameterDict = {}

def dictionaryLayoutCheck(InfoDict):
    for key in InfoDict:
        if key not in ("types", "numberOfDimensions", "dimensions", "names", "name", "defaultValues"):
            raise Exception("\'" + key + "\' key in " + InfoDict['name'] + " is not allowed.")
        if key == "defaultValues" and InfoDict['name'] != 'ParameterDict':
            raise Exception("\'defaultValues\' key is allowed only in parameterDict, and not in " + InfoDict['name'] + ".")
    return 1

def tupleCheck(InfoDict):
    for key in InfoDict:
        if not key == "name":
            if not isinstance (InfoDict[key], tuple):
                raise TypeError("\'" + key + "\' key in " + InfoDict['name'] + " must be a tuple (a tuple with just one element must end with a comma).")
    return 1

# checks that all keys in the dictionary have the same length
def numberFormatCheck(InfoDict):
    lengths = []
    # a list is created with length of all keys (except 'name' key wich hold the name of the dictionary)
    for key in InfoDict:
        if not key == "name":
            lengths.append(len(InfoDict[key]))
    # a set contains only unique values
    uinqueLengths = set(lengths)
    # if they are all equal, then set is made of just one element
    if not len(uinqueLengths) == 1:
        raise Exception(InfoDict['name'] + " has keys with different number of elements. All keys must contain the same number of elements.")
    return 1

def typeFormatCheck(InfoDict):
    for i in range(len(InfoDict['types'])):
        if (type(InfoDict['types'][i]) != type):
            raise TypeError(InfoDict['name'] + "'types' key contains a non-type value.")
    return 1

# besides checking the format of 'dimensions' key, also creates a new
# 'numberOfDimensions' key with syntetic information on data dimensions
def dimensionFormatCheck(InfoDict):
    numberOfDimensions = list()
    for i, dims in enumerate(InfoDict['dimensions']):
        if not isinstance (dims, tuple):
            raise TypeError(InfoDict['name'] + "['dimensions'][" + str(i) + "] must be a tuple.")
        if len(dims) != 2:
            raise Exception(InfoDict['name'] + "['dimensions'][" + str(i) + "] must be a 1x2 tuple.")
        for j in InfoDict['dimensions'][i]:
            if not isinstance (j, int):
                raise Exception("Error in " + InfoDict['name'] + "['dimensions'][" + str(i) + "]: entries must be integers")
        if dims[0] == 1 and dims[1] == 1:     # scalar
            numberOfDimensions.append(int(0))
        elif (dims[0] > 1 and dims[1] == 1) or (dims[0] == 1 and dims[1] > 1):   # vector
            numberOfDimensions.append(int(1))
        elif dims[0] > 1 and dims[1] > 1:     # matrix
            numberOfDimensions.append(int(2))
        else:
            raise Exception("Error in " + InfoDict['name'] + "['dimensions'][" + str(i) + "]: entries must be equal or greater than 1.")
    InfoDict['numberOfDimensions'] = tuple(numberOfDimensions)

def nameFormatCheck(InfoDict):
    for i, name in enumerate(InfoDict['names']):
        if not isinstance(name, str):
            raise Exception("Error in " + InfoDict['name'] + "['names'][" + str(i) + "]: entries must be strings")

def defaulValueFormatCheck(InfoDict):
    for idx, value in enumerate(InfoDict['defaultValues']):
        if not isinstance(value, list):
            raise Exception("Error in " + InfoDict['name'] + "['defaulValues'][" + str(idx) + "]: entries must be lists.")
        if InfoDict['numberOfDimensions'][idx] == 0:   # scalar
            if not isinstance(value[0], (int, float)):
                raise Exception("Error in " + InfoDict['name'] + "['defaulValues'][" + str(idx) + "]: list entries must be either integer of float.")
        elif InfoDict['numberOfDimensions'][idx] == 1: # vector
            for entry in value:
                if not isinstance(entry, (int, float)):
                    raise Exception("Error in " + InfoDict['name'] + "['defaulValues'][" + str(idx) + "]: list entries must be either integer of float.")
        else:                                          # matrix
            for row in value:
                for entry in row:
                    if not isinstance(entry, (int, float)):
                        raise Exception("Error in " + InfoDict['name'] + "['defaulValues'][" + str(idx) + "]: list entries must be either integer of float.")
                

def initialize(moduleName):
    
    global PygamRootInputDict
    global PygamRootOutputDict
    global PygamParameterDict


#    PygamRootInputDict = {}
#    PygamRootOutputDict = {}
#    PygamParameterDict = {}
    
    global data
    data = {}

    RootInputDict[moduleName]['name']  = 'RootInputDict'
    RootOutputDict[moduleName]['name'] = 'RootOutputDict'
    ParameterDict[moduleName]['name']  = 'ParameterDict'
    
    dictTuple = (RootInputDict, RootOutputDict, ParameterDict)
    
    for argDict in dictTuple:
        dictionaryLayoutCheck(argDict[moduleName])
        tupleCheck(argDict[moduleName])
        numberFormatCheck(argDict[moduleName])
        typeFormatCheck(argDict[moduleName])
        dimensionFormatCheck(argDict[moduleName])
        nameFormatCheck(argDict[moduleName])
    
    # parameter default value checks
    defaulValueFormatCheck(ParameterDict[moduleName])
    
    # data dictionary creation
    for paramIdx, paramName in enumerate(ParameterDict[moduleName]['names']):
        data[paramName] = numpy.array(ParameterDict[moduleName]['defaultValues'][paramIdx])
    
    # values from the custom Python code are imported here
    try:
        PygamRootInputDict[moduleName]  = RootInputDict[moduleName]
        PygamRootOutputDict[moduleName] = RootOutputDict[moduleName]
        PygamParameterDict[moduleName]  = ParameterDict[moduleName]
    except:
        PygamRootInputDict = {moduleName :  RootInputDict[moduleName]}
        PygamRootOutputDict = {moduleName :  RootOutputDict[moduleName]}
        PygamParameterDict = {moduleName :  ParameterDict[moduleName]}
    return 1
    
# --- APIs ---

# -- number of arguments
def getNumberOfArguments(InfoDict):
    
    # just in case the inizialise function was somehow skipped
    numberFormatCheck(InfoDict)
    
    return len(InfoDict['types'])

def getNumberOfInputs(moduleName):
    
    return getNumberOfArguments(PygamRootInputDict[moduleName])

def getNumberOfOutputs(moduleName):
    
    return getNumberOfArguments(PygamRootOutputDict[moduleName])

def getNumberOfParameters(moduleName):
    
    return getNumberOfArguments(PygamParameterDict[moduleName])

# -- argument types
def getInputType(moduleName, sigIdx):
    
    return PygamRootInputDict[moduleName]['types'][sigIdx]
    
def getOutputType(moduleName, sigIdx):
    
    return PygamRootOutputDict[moduleName]['types'][sigIdx]

def getParameterType(moduleName, param):
    
    if isinstance(param, int):
        paramIdx = param
    
    elif isinstance(param, str):
        try:
            paramIdx = PygamParameterDict[moduleName]['names'].index(param)
        except:
            raise Exception(param + ' not found.')
    
    return PygamParameterDict[moduleName]['types'][paramIdx]

# -- argument number of dimensions
def getInputNumberOfDimensions(moduleName, sigIdx):
    
    return PygamRootInputDict[moduleName]['numberOfDimensions'][sigIdx]

def getOutputNumberOfDimensions(moduleName, sigIdx):
    
    return PygamRootOutputDict[moduleName]['numberOfDimensions'][sigIdx]

def getParameterNumberOfDimensions(moduleName,param):
    
    if isinstance(param, int):
        paramIdx = param
    
    elif isinstance(param, str):
        try:
            paramIdx = PygamParameterDict[moduleName]['names'].index(param)
        except:
            raise Exception(param + ' not found.')
    
    return PygamParameterDict[moduleName]['numberOfDimensions'][paramIdx]

# -- argument dimensions
def getInputDimensions(moduleName,sigIdx):
    
    return PygamRootInputDict[moduleName]['dimensions'][sigIdx]

def getOutputDimensions(moduleName,sigIdx):
    
    return PygamRootOutputDict[moduleName]['dimensions'][sigIdx]

def getParameterDimensions(moduleName,param):
    
    if isinstance(param, int):
        paramIdx = param
    
    elif isinstance(param, str):
        try:
            paramIdx = PygamParameterDict[moduleName]['names'].index(param)
        except:
            raise Exception(param + ' not found.')
    
    return PygamParameterDict[moduleName]['dimensions'][paramIdx]

# -- argument names
def getInputName(moduleName,sigIdx):
    
    return PygamRootInputDict[moduleName]['names'][sigIdx]

def getOutputName(moduleName, sigIdx):
    
    return PygamRootOutputDict[moduleName]['names'][sigIdx]

def getParameterName(moduleName, paramIdx):
    
    return PygamParameterDict[moduleName]['names'][paramIdx]

# -- parameter-specific functions
def getParameterDefaultValue(moduleName, param):
    
    if isinstance(param, int):
        paramIdx = param
    
    elif isinstance(param, str):
        try:
            paramIdx = PygamParameterDict[moduleName]['names'].index(param)
        except:
            raise Exception(param + ' not found.')
    
    return PygamParameterDict[moduleName]['defaultValues'][paramIdx]

def setParameterValue(moduleName, param, value):
    
    if isinstance(param, int):
        if param > len(data) - 1 or param < 0:
            raise Exception("Invalid index (either out of bounds or lesser than zero.)")
        else:
            paramName =  PygamParameterDict[moduleName]['names'][param]
            
    elif isinstance(param, str):
        paramName = param
    
    if paramName in data:
        data[paramName] = value
    else:
        raise Exception(paramName + " has not been declared as parameter.")
    
    return 1
