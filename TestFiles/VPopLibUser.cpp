// VPopLibUser.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <chrono>
#include <string>
#include "VPopLib.h"

using namespace std;

// This function is specifically tailored for the .wth format files with NO HEADER
void LoadWeatherFileWTH(string FileName, vector<string>& WeatherList)
{
    ifstream weatherfile (FileName);
    string line;
    if (weatherfile.is_open())
    {
        WeatherList.clear();
        while (getline(weatherfile, line))
        {
            WeatherList.push_back(line);
        }
    }
    else
    {
        cout << "could not open weather file" << endl;
    }



    weatherfile.close();
}

bool LoadICList(vector<string>& ICList, string InputFileName)
{
    bool success = false;
    if (InputFileName.size() == 0) // Test only
    {
        ICList.clear();
        ICList.push_back("icworkerbrood=5000");
        ICList.push_back("icworkeradults =5000");
        ICList.push_back("icdronelarvae = 200");
        ICList.push_back("icqueenstrength=   5");
        ICList.push_back("trees=500");
        ICList.push_back("icforagerlifespan=8");
        ICList.push_back("foragermaxprop = 0.8");
        success = true;
    }
    else // Load the ICList from the file
    {
        ifstream inputfile (InputFileName);
        string line;
        if (inputfile.is_open())
        {
            ICList.clear();
            while (getline(inputfile, line))
            {
                ICList.push_back(line);
            }
            inputfile.close();
            success = true;
        }
    }
    return success;
}

bool LoadCTList(vector<string> &theCTList, string ContaminationTablePath)
{
    ifstream contfile(ContaminationTablePath);
    string line;
    bool retval = false;
    if (contfile.is_open())
    {
        theCTList.clear();
        while (getline(contfile, line))
        {
            theCTList.push_back(line);
        }
        contfile.close();
        retval = true;
    }
    return retval;

}

void OutputStringList(vector<string> List, string Filename, string title = "")
{
    if (List.size() == 0) cout << "Empty Output String List" << title << endl;
    else
    {
        ofstream resultfile(Filename);
        if (resultfile)
        {
            for (size_t i = 0; i < List.size(); i++)
            {
                resultfile << List[i] << endl;
            }
            resultfile.close();
        }
    }
}




int main(int argc, char** argv)
{
    string InputFilePath = "C:/Users/strat/Desktop/verificationtests/Parameter Files/InputFileFromFeeding_Study.txt";
    string WeatherFilePath = "C:/Users/strat/Desktop/verificationtests/Parameter Files/15055_grid_35.875_lat.txt";
    string ResultFilePath = "C:/Users/strat/Desktop/verificationtests/VPopLibVerification/ResultsfromFeedingStudyLib.txt";
    string ErrorFilePath = "C:/Users/strat/Desktop/verificationtests/VPopLibVerification/Errors.txt";
    string MessageFilePath = "C:/Users/strat/Desktop/verificationtests/VPopLibVerification/Messages.txt";
    string ContaminationTablePath = "C:/Users/strat/Desktop/verificationtests/Parameter Files/NutrientContaminationFile.txt";
    bool VPLIB_Return = true;
    bool ReleaseVersion = true;

    // Command line parameters are InputFilePath WeatherFilePath ResultFilePath ErrorFilePath MessageFilePath and ContaminationTablePath.  If any are present, assume all are
    if (argc > 1)  // ignore the program name
    {
        InputFilePath = argv[1];
        WeatherFilePath = argv[2];
        ResultFilePath = argv[3];
        ErrorFilePath = argv[4];
        MessageFilePath = argv[5];
        ContaminationTablePath = argv[6];
    }
    VPLIB_Return = InitializeModel();
    if (!VPLIB_Return) cout << "InitializeModel" << endl;
    VPLIB_Return = ClearResultsBuffer();
    if (!VPLIB_Return) cout << "ClearResultsBuffer failed" << endl;

    vector<string> theList;
    LoadICList(theList, InputFilePath);  //Local function to load the name/value pairs for the initial conditions from a file
    VPLIB_Return = SetICVariablesV(theList);
    if (!VPLIB_Return) cout << "SetICVariables failed" << endl;

    VPLIB_Return = ClearWeather();
    if (!VPLIB_Return) cout << "ClearWeather failed" << endl;

    vector<string> theWeatherList;
    LoadWeatherFileWTH(WeatherFilePath, theWeatherList); // Local function to load weather list from a filel;
    VPLIB_Return = SetWeatherV(theWeatherList);
    if (!VPLIB_Return) cout << "SetWeather failed" << endl;

    vector<string> theCTList;
    LoadCTList(theCTList, ContaminationTablePath);
    VPLIB_Return = SetContaminationTable(theCTList);
    if (!VPLIB_Return) cout << "SetContaminationTable failed" << endl;

    auto start = chrono::system_clock::now();
    vector<string> Result;
    int i;
    int maxruns = 10;
    for (i = 0; i < maxruns; i++)  // Run multiple times
    {
        VPLIB_Return = RunSimulation();
        if (!VPLIB_Return) cout << "RunSimulation failed" << endl;

        // Character Pointer Array version of GetResults.
        char** resultsCPA;
        int resultSize;
        VPLIB_Return = GetResultsCPA(&resultsCPA, &resultSize);
        if (!VPLIB_Return)
        {
            cout << "GetResults CPA failed" << endl;
        }
        else
        {
            // convert the character pointer array into string list in order to save
            for (int i = 0; i < resultSize; i++)
            {
                char* CPstring = resultsCPA[i];
                std::string line(CPstring);
                Result.push_back(line);
                delete CPstring;  // Free up the memory allocated by GetResultsCPA
            }
            delete resultsCPA;
        }

        // String vector version of GetResults
        //VPLIB_Return = GetResults(Result);
        //if (!VPLIB_Return) cout << "GetResults failed" << endl;

        if (Result.size() > 0) OutputStringList(Result, ResultFilePath);  // Local function to list results on console
        VPLIB_Return = ClearResultsBuffer();
        if (!VPLIB_Return) cout << "ClearResultsBuffer failed" << endl;

    }
    auto end = chrono::system_clock::now();
    cout << "Exiting Loop" << endl;
    chrono::duration<double> diff = end - start;
    cout << "Time to run " << i << " simulations = " << diff.count() << " Seconds" << endl;


    vector<string> infolist;
    VPLIB_Return = GetInfoList(infolist);
    if (!VPLIB_Return) cout << "GetInfoList failed" << endl;
    OutputStringList(infolist, MessageFilePath, "Information"); //Local function to list any passed information messages to the console

    char** infoCPA;
    int infoSize;
    VPLIB_Return = GetInfoListCPA(&infoCPA, &infoSize);

    vector<string> errlist;
    VPLIB_Return = GetErrorList(errlist);
    if (!VPLIB_Return) cout << "GetErrorList failed" << endl;
    OutputStringList(errlist, ErrorFilePath, "Errors"); // Local function to list the errors to the console

    if (Result.size() > 0) OutputStringList(Result, "Result.txt", "Results");  // Local function to list results on console


}

