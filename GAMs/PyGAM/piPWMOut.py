import numpy as np
import pygam
from copy import *

# --- CODE INFO ---

pygam.RootInputDict = {
        'types'      : ( np.float32, ),
        'dimensions' : ( (16, 1),  ),
        'names'      : ( 'Input',   )
        }

pygam.RootOutputDict = {
        'types'      : ( np.float32, ),
        'dimensions' : ( (16, 1),     ),
        'names'      : ( 'Output',        )
        }

pygam.ParameterDict = {
        'types'        : ( np.int32, np.int32 ), 
        'dimensions'   : ( (1,1),  (1,1)  ),
        'names'        : ( 'Frequency',  'ActChans',),
        'defaultValues': ( [1000],  [16],)
        }

# --- MAIN FUNCTION ---


def setup():
    freq = pygam.data['Frequency']    
    actChans = pygam.data['ActChans']   

def execute(x):
    print(str(x))
    return x,
