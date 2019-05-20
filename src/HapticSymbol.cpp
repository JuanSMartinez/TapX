#include "../inc/HapticSymbol.hpp"

namespace TapX
{
    //Constructor
    HapticSymbol::HapticSymbol(std::string symbolLabel)
    : id(symbolLabel)
    {
        matrixRowIndex = 0;
        matrixRows = 0;
    }

    //Destructor
    HapticSymbol::~HapticSymbol()
    {
        free(dataMatrix);
    }

    //Get the current index row of the matrix being consumed
    size_t HapticSymbol::getMatrixRowIndex() const
    {
        return matrixRowIndex;
    }

    //Determine if the matrix has been consumed
    bool HapticSymbol::matrixConsumed() const
    {
        return matrixRowIndex == matrixRows;
    }

    //Increase the matrix row index to consume a new row
    bool HapticSymbol::increaseIndex()
    {
        if(matrixRowIndex < matrixRows)
        {
            matrixRowIndex++;
            return true;
        }
        else
        {
            return false;
        }
    }

    //Get value of the matrix at a certain position
    float HapticSymbol::getValueAt(int i, int j)
    {
        return dataMatrix[i*24 + j];
    }

    //Initialize matrix data from a data path that contains the csv file for this symbol
    void HapticSymbol::initializeData(std::string dataPath)
    {
        std::string path = dataPath + id + ".csv";

        std::vector<std::string> vector;
        std::string line;
        std::ifstream file;
        file.open(path);
        while(file.good())
        {
            getline(file,line);
            vector.push_back(line);
        }
        file.close();

        //Create the matrix
        matrixRows = vector.size();
        dataMatrix = (float*)calloc(matrixRows*24, sizeof(float));
        
        for(int i = 0; i < matrixRows; i++)
        {
            std::string row = vector[i];
            size_t pos = row.find(",");
            std::string value;
            int j = 0;
            while(pos != std::string::npos){
                value = row.substr(0,pos);
                row.erase(0, pos + 1);
                dataMatrix[i*24 + j] = atof(value.c_str());
                j++;
                pos = row.find(",");
            }
            dataMatrix[i*24 + j] = atof(row.c_str());
        }
    }
    
    //Initialize matrix data as a period of silence of a certain amount of ms and a sample frequency
    void HapticSymbol::initializeData(int durationMs, int fs)
    {
        matrixRows = fs/(durationMs*1000);
        for(int i = 0; i < matrixRows; i++)
        {
            for(int j = 0; j < 24; j++)
                dataMatrix[i*24 + j] = 0.0;
        }
    }

    //Reset the index to zero
    void HapticSymbol::resetIndex() 
    {
        matrixRowIndex = 0;
    }
}