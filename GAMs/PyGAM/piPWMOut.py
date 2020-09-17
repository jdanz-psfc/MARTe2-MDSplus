import numpy as np
import pygam
from copy import *

# --- CODE INFO ---

pygam.RootInputDict['piPWMOut']= {
        'types'      : ( np.float32, ),
        'dimensions' : ( (1, 16),  ),
        'names'      : ( 'Input',   )
        }

pygam.RootOutputDict['piPWMOut'] = {
        'types'      : ( np.float32, ),
        'dimensions' : ( (1, 16),     ),
        'names'      : ( 'Output',        )
        }

pygam.ParameterDict['piPWMOut'] = {
        'types'        : ( np.int32, np.int32 ), 
        'dimensions'   : ( (1,1),  (1,1)  ),
        'names'        : ( 'Frequency',  'ActChans',),
        'defaultValues': ( [1000],  [16],)
        }

# --- MAIN FUNCTION ---


import board
import busio
import adafruit_pca9685
def setup():
    global freq, actChans, hat
    i2c = busio.I2C(board.SCL, board.SDA)
    hat = adafruit_pca9685.PCA9685(i2c)
    freq = pygam.data['Frequency']    
    actChans = pygam.data['ActChans']   
    hat.frequency = freq

def execute(x):
    print('in execute')
    print(actChans)
    print(x)
    for chan in range(int(actChans)):
        value = min(1., max(x[chan], 0.)) * 65535
        print('value is %f'%value)
        value = int(value)
        print('setting PWM channel %d to %d' % (chan, value))
        hat.channels[chan].duty_cycle=value
    print(x)
    print(x.__class__)
    return x,
