#!/usr/bin/python
#import ctypes 
import ctypes
import os, sys

def  StringList2CPA(theList):
    theListBytes = []
    for i in range(len(theList)):
        theListBytes.append(bytes(theList[i], 'utf-8'))
    return theListBytes

if __name__ == "__main__":

    libvpop = ctypes.CDLL('/home/stratpilot/.vs/VPopLib/out/build/Linux-GCC-Debug/liblibvpop.so')


    weatherpath = '/home/stratpilot/projects/PY/weather.txt'
    inputpath = '/home/stratpilot/projects/PY/inputfilefromfeeding_study.txt'
    contampath = '/home/stratpilot/projects/PY/nutrientcontaminationfile.txt'
    resultpath = '/home/stratpilot/projects/PY/resultsfromfeedingstudy.txt'
    errorpath = '/home/stratpilot/projects/PY/errors.txt'
    infopath = '/home/stratpilot/projects/PY/info.txt'

    #Initialize Model
    if libvpop.InitializeModel() :
        a = libvpop.InitializeModel()
        print(a)
        print('Model Initialized')
    else :
        print('Error Initializing Model')



    #Clear Results and Weather lists
    if libvpop.ClearResultsBuffer() :
        print('Results Buffer Cleared')
    else :
        print('Error Clearing Results Buffer')


    if libvpop.ClearWeather() :
        print('Weather Cleared')
    else :
        print('Error Clearing Weather')

    #Load the Initial Conditions
    icf = open(inputpath)
    inputlist = icf.readlines()
    CPA = (ctypes.c_char_p * len(inputlist))()
    inputlist_bytes = StringList2CPA(inputlist)
    CPA[:] = inputlist_bytes
    if libvpop.SetICVariablesCPA(CPA, len(inputlist)) :
        print('Loaded IC Variables')
    else :
        print("Error Loading IC Variables")


    #Load Weather
    wf = open(weatherpath)
    weatherlines = wf.readlines()
    CPA = (ctypes.c_char_p * len(weatherlines))()
    weatherline_bytes = StringList2CPA(weatherlines) 
    CPA[:] = weatherline_bytes
    if libvpop.SetWeatherCPA(CPA, len(weatherlines)) :
       print('Loaded Weather')
    else :
        print("Error Loading Weather")

    #Load Contamination Table
    ct = open(contampath)
    contamlines = ct.readlines()
    CPA = (ctypes.c_char_p * len(contamlines))()
    contamlines_bytes = StringList2CPA(contamlines)
    CPA[:] = contamlines_bytes
    if libvpop.SetContaminationTableCPA(CPA, len(contamlines)) :
       print('Loaded Contamination Table')
    else :
        print("Error Loading Contamination Table")


    # Run Simulation multiple times
    Loops = 2
    for i in range(1,Loops) :
        k = i
        # Run Simulation
        if libvpop.RunSimulation() :
            a = 1
            print('Simulation Ran Successfully')
        else :
            a=2
            print('Error In Simulation Run')

        # Get Results
        theCount = ctypes.c_int(0)
        #p_Count.contents = theCount
        #p_Results = ctypes.POINTER(ctypes.c_char)()
        #p_Count = ctypes.POINTER(ctypes.c_int)()
        #libvpop.GetResultsCPA.argtypes = ctypes.P
        p_Results - (ctypes.c_char_p*4)(*map(ctypes.addressof, pResults))
        if libvpop.GetResultsCPA(ctypes.pointer(p_Results), ctypes.pointer(theCount)) :
            print(p_Results[1])
        # Store Results
        # Clear Results 

