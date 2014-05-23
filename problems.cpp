#include "problems.h"
#include <string>
#include <fstream>
#include <iostream>
#include <math.h>

using namespace std;

HopfieldNetwork problems::createRookProblem(const unsigned int boardSize)
{
    const unsigned int neuronCount=boardSize*boardSize;
    const vector<bool> neuronValues=vector<bool>(neuronCount, false); // for clarity, could just use default value in constructor
    vector< vector<double> > neuronWeights=
            vector< vector<double> >(neuronCount, vector<double>(neuronCount,0));

    for (unsigned int i=0; i<neuronCount; i++)
    {
        for (unsigned int j=0; j<neuronCount; j++)
        {
            if (i==j)
            {
                neuronWeights[i][j]=1.0L;
            }
            else
            {
                // If i, j are neurons in the same row or column
                if ( (i%boardSize==j%boardSize) || (i/boardSize == j/boardSize) )
                {
                    neuronWeights[i][j]=-2.0L;
                }
            }
        }
    }
    return HopfieldNetwork(neuronWeights, neuronValues, neuronCount);
}

/**
  * @brief Updates given row and column so that they correspond to the next element on an boardSize times boardSize board.
  * The board is considered to be indexed left to right, top to bottom.
  * @param1 row
  * @param2 column
  * @param3 board size
  */
inline void next(unsigned int& row, unsigned int& column, const unsigned int boardSize)
{
    if (++column==boardSize)
    {
        row++;
        column=0;
    }
}

HopfieldNetwork problems::createQueenProblem(const unsigned int boardSize)
{
    const unsigned int neuronCount=boardSize*boardSize;
    const vector<bool> neuronValues=vector<bool>(neuronCount, true); // for clarity, could just use default value in constructor
    vector< vector<double> > neuronWeights=
            vector< vector<double> >(neuronCount, vector<double>(neuronCount,0));

    unsigned int iRow=0;
    unsigned int iColumn=0;
    for (unsigned int i=0; i<neuronCount; i++)
    {
        unsigned int jRow=0;
        unsigned int jColumn=0;
        for (unsigned int j=0; j<neuronCount; j++)
        {
            if (i==j) neuronWeights[i][j]=1L;
            else if (iRow==jRow || iColumn==jColumn)
            {
                // if i, j are neurons in the same row or column
                neuronWeights[i][j]=-2L;
            }
            else if (iRow-jRow==iColumn-jColumn || iRow-jRow==jColumn-iColumn)
            {
                // if i, j  are neurons on the same diagonal
                neuronWeights[i][j]=-2L;
            }

            // proceed to the next j
            next (jRow, jColumn, boardSize);
        }

        // proceed to the next i
        next (iRow, iColumn, boardSize);

    }
    return HopfieldNetwork(neuronWeights, neuronValues, neuronCount);
}

HopfieldNetwork problems::createTSP(std::string fileName, double delta){

    std::ifstream inFile(fileName.c_str());
    if (inFile.is_open()) {
        unsigned int cityCount;
        inFile >> cityCount;
        unsigned int neuronCount = cityCount * cityCount;
        const vector<bool> neuronValues=vector<bool>(neuronCount, false); // for clarity, could just use default value in constructor
        vector< vector<double> > neuronWeights=
                vector< vector<double> >(neuronCount, vector<double>(neuronCount,0));
        //coordinates
        vector< vector<int> > coordinates = vector< vector<int> >(cityCount, vector<int>(2,0));


        for (unsigned int i=0; i < cityCount; i++){
            int xCoordinate;
            inFile >> xCoordinate;
            int yCoordinate;
            inFile >> yCoordinate;
            coordinates[i][0] = xCoordinate;
            coordinates[i][1] = yCoordinate;
        }

        //compute distances
        vector< vector<double> > distances = vector< vector<double> >(cityCount, vector<double>(cityCount, 0.0));
        for (unsigned int i=0; i<cityCount; i++){
            for (unsigned int j=0; j<cityCount; j++){
                distances[i][j] = sqrt(pow(coordinates[i][0]-coordinates[j][0], 2.0)
                        + pow(coordinates[i][1]-coordinates[j][1], 2.0));
            }
        }

        //compute weights
        for (unsigned int cityIndex=0; cityIndex<cityCount; cityIndex++){
            for (unsigned int step=0; step<cityCount; step++){
                for (unsigned int nextCityIndex=0; nextCityIndex<cityCount; nextCityIndex++){
                    if (cityIndex == nextCityIndex){
                        for (unsigned int stepToSelf = 0; stepToSelf<cityCount; stepToSelf++){
                            if (step == stepToSelf) neuronWeights[cityIndex*cityCount + step][cityIndex*cityCount + step] =
                                    delta / 2.0;
                            else neuronWeights[cityIndex*cityCount + step][cityIndex*cityCount + stepToSelf] =
                                    -delta;
                        }
                    }
                    else{
                        neuronWeights[cityIndex*cityCount + step][nextCityIndex*cityCount + step] =
                                -delta;
                        neuronWeights[cityIndex*cityCount + step][nextCityIndex*cityCount + ((step+1)%cityCount)] =
                                neuronWeights[nextCityIndex*cityCount + ((step+1)%cityCount)][cityIndex*cityCount + step] =
                                - distances[cityIndex][nextCityIndex];
                    }
                }
            }
        }

        inFile.close();
        return HopfieldNetwork(neuronWeights, neuronValues, neuronCount);
    }

    return HopfieldNetwork();
}

