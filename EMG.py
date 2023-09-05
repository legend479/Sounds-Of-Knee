#python code to get data for EMG sensors from OM2M and plot it

import requests
import json
import matplotlib.pyplot as plt
import numpy as np


OM2M_URL = 'http://localhost:8080/~/in-cse/in-name/EMG_DATA/EMG_DATA_1/DATA'

#function to get data from OM2M (GET request)
def get_data():
    #get data from OM2M
    response = requests.get(OM2M_URL)
    #convert response to json format
    data = json.loads(response.text)
    #get data from json
    value = data['m2m:cin']['con']
    #print data
    print(value)
    #    