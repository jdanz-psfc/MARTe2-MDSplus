# --- MODULE CODE ---

import numpy

# variables that should remain visible in this module
global PygamRootInputDict
global PygamRootOutputDict
global PygamParameterDict
global data

data = {}

def dictionaryLayoutCheck(InfoDict):
    for key in InfoDict:
        if key not in ("types", "dimensions", "names", "name", "defaultValues"):
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
                

def initialize():
    
    global PygamRootInputDict
    global PygamRootOutputDict
    global PygamParameterDict
    
    global data
    
    RootInputDict['name']  = 'RootInputDict'
    RootOutputDict['name'] = 'RootOutputDict'
    ParameterDict['name']  = 'ParameterDict'
    
    dictTuple = (RootInputDict, RootOutputDict, ParameterDict)
    
    for argDict in dictTuple:
        dictionaryLayoutCheck(argDict)
        tupleCheck(argDict)
        numberFormatCheck(argDict)
        typeFormatCheck(argDict)
        dimensionFormatCheck(argDict)
        nameFormatCheck(argDict)
    
    # parameter default value checks
    defaulValueFormatCheck(ParameterDict)
    
    # data dictionary creation
    for paramIdx, paramName in enumerate(ParameterDict['names']):
        data[paramName] = numpy.array(ParameterDict['defaultValues'][paramIdx])
    
    # values from the custom Python code are imported here
    PygamRootInputDict  = RootInputDict
    PygamRootOutputDict = RootOutputDict
    PygamParameterDict  = ParameterDict
            
    return 1
    
# --- APIs ---

# -- number of arguments
def getNumberOfArguments(InfoDict):
    
    # just in case the inizialise function was somehow skipped
    numberFormatCheck(InfoDict)
    
    return len(InfoDict['types'])

def getNumberOfInputs():
    
    return getNumberOfArguments(PygamRootInputDict)

def getNumberOfOutputs():
    
    return getNumberOfArguments(PygamRootOutputDict)

def getNumberOfParameters():
    
    return getNumberOfArguments(PygamParameterDict)

# -- argument types
def getInputType(sigIdx):
    
    return PygamRootInputDict['types'][sigIdx]
    
def getOutputType(sigIdx):
    
    return PygamRootOutputDict['types'][sigIdx]

def getParameterType(param):
    
    if isinstance(param, int):
        paramIdx = param
    
    elif isinstance(param, str):
        try:
            paramIdx = PygamParameterDict['names'].index(param)
        except:
            raise Exception(param + ' not found.')
    
    return PygamParameterDict['types'][paramIdx]

# -- argument number of dimensions
def getInputNumberOfDimensions(sigIdx):
    
    return PygamRootInputDict['numberOfDimensions'][sigIdx]

def getOutputNumberOfDimensions(sigIdx):
    
    return PygamRootOutputDict['numberOfDimensions'][sigIdx]

def getParameterNumberOfDimensions(param):
    
    if isinstance(param, int):
        paramIdx = param
    
    elif isinstance(param, str):
        try:
            paramIdx = PygamParameterDict['names'].index(param)
        except:
            raise Exception(param + ' not found.')
    
    return PygamParameterDict['numberOfDimensions'][paramIdx]

# -- argument dimensions
def getInputDimensions(sigIdx):
    
    return PygamRootInputDict['dimensions'][sigIdx]

def getOutputDimensions(sigIdx):
    
    return PygamRootOutputDict['dimensions'][sigIdx]

def getParameterDimensions(param):
    
    if isinstance(param, int):
        paramIdx = param
    
    elif isinstance(param, str):
        try:
            paramIdx = PygamParameterDict['names'].index(param)
        except:
            raise Exception(param + ' not found.')
    
    return PygamParameterDict['dimensions'][paramIdx]

# -- argument names
def getInputName(sigIdx):
    
    return PygamRootInputDict['names'][sigIdx]

def getOutputName(sigIdx):
    
    return PygamRootOutputDict['names'][sigIdx]

def getParameterName(paramIdx):
    
    return PygamParameterDict['names'][paramIdx]

# -- parameter-specific functions
def getParameterDefaultValue(param):
    
    if isinstance(param, int):
        paramIdx = param
    
    elif isinstance(param, str):
        try:
            paramIdx = PygamParameterDict['names'].index(param)
        except:
            raise Exception(param + ' not found.')
    
    return PygamParameterDict['defaultValues'][paramIdx]

def setParameterValue(param, value):
    
    if isinstance(param, int):
        if param > len(data) - 1 or param < 0:
            raise Exception("Invalid index (either out of bounds or lesser than zero.)")
        else:
            paramName =  PygamParameterDict['names'][param]
            
    elif isinstance(param, str):
        paramName = param
    
    if paramName in data:
        data[paramName] = value
    else:
        raise Exception(paramName + " has not been declared as parameter.")
    
    return 1