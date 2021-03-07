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
        p_Results = ctypes.POINTER(ctypes.c_char_p)()
        if libvpop.GetResultsCPA(ctypes.byref(p_Results),ctypes.byref(theCount)) :
            # Store Reaults
            max = int(theCount.value)
            print('Number lines of results' ,max)
            outfile = open(resultpath, "w")
            for j in range(0, max -1): 
                outfile.write(p_Results[j].decode("utf-8"))
            outfile.close()
            print('Wrote Results to file')
            libvpop.ClearResultsBuffer()
    
    # Get Info and Errors
    p_Errors = ctypes.POINTER(ctypes.c_char_p)()
    NumErrors = ctypes.c_int(0)
    if libvpop.GetErrorListCPA(ctypes.byref(p_Errors), ctypes.byref(NumErrors)) :
        # Get Errors
        max = int(NumErrors.value)
        outfile = open(errorpath, "w")
        for j in range(0,max-1) :
            outfile.write(p_Errors[j].decode("utf-8"))
        outfile.close()
        print('Wrote Error List to file')
        libvpop.ClearErrorList()
    
    p_Info = ctypes.POINTER(ctypes.c_char_p)()
    NumInfo = ctypes.c_int(0)
    if libvpop.GetInfoListCPA(ctypes.byref(p_Info), ctypes.byref(NumInfo)) :
        # Get Info
        max = int(NumInfo.value)
        print('Number of info lines.' ,max)
        outfile = open(infopath, "w")
        for j in range(0,max-1) :
            outfile.write(p_Info[j].decode("utf-8"))
        outfile.close()
        print('Wrote Info List to file')
        libvpop.ClearInfoList()   
                

       

