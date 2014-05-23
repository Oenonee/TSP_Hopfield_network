#ifndef NETWORK_H
#define NETWORK_H

#include <iostream>     /* cerr, endl, istream, ostream */
#include <fstream>      /* ifstream */

#include <vector>       /* vector */
#include <algorithm>    /* random_shuffle */

#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */

#include <math.h>       /* log */
#include <limits.h>     /* ULONG_MAX */

#include "temperaturemodule.h"


using std::cout;
using std::cerr;
using std::endl;

using std::istream;
using std::ostream;
using std::ifstream;

using std::vector;
/**
  * Error codes of errors raised by raiseError
  */
enum errorCode
{
    NO_ERROR = 0,
    INCONSISTENCY,
    NON_SYMMETRIC,
    OUT_OF_BOUNDS,
    UNKNOWN_MODE,
    READ_FAILURE,
    FILE_NOT_OPEN,
    RANDOMIZATION,
    UNKNOWN_ERROR
};

/**
  * Error messages corresponding to enum errorCode
  */
const static char* errorMessages[UNKNOWN_ERROR+1]={
    "No error detected. Proceeding.",
    "Inconsistency detected when updating the network. Ignoring.",
    "Attempt to create a network with nonsymmetric weights matrix. Ignoring.",
    "Attempt to activate a neuron out of bounds. Ignoring",
    "Step by unknown mode requested. Ignoring.",
    "Exception when reading the network from the input. Ignoring.",
    "Unable to open the file. Ignoring.",
    "Could not set up a probability distribution. Exiting.",
    "Uknown error. Exiting."
};

/**
  * Raises an error by printing the corresponding error message to the given output.
  * @param1 error message
  * @param2 error output (default cerr)
  */
inline void raiseError(const errorCode error, ostream& out = cerr)
{out<<errorMessages[error]<<endl;if (error==UNKNOWN_ERROR) exit(EXIT_FAILURE);}

class HopfieldNetwork
{
private:

    vector< vector<double> > m_neuronWeights; // weights of links between neurons
    vector<bool> m_neuronValues; // values of neurons
    unsigned long m_neuronCount;

    TemperatureModule* m_temperatureModule;

    /**
      * Checks whether a network with given neuronWeights, neuronValues and neuronCount will be inconsistent.
      * @param1 neuronWeights
      * @param2 neuronValues
      * @param3 neuronCount
      * @return inconsistency errorCode (0=NO_ERROR)
      */
    static errorCode isInconsistent(const vector< vector<double> >, const vector<bool>, const unsigned long);


public:

    // TEMPORARY
    unsigned long getNeuronValueSum() const
    {
        unsigned long result=0;
        for (unsigned long i=0; i<m_neuronCount;i++)
        {
            if (m_neuronValues[i]) result++;
        }
        return result;
    }

    inline unsigned long getNeuronCount() const {return m_neuronCount;}

    /**
      * Constructor of class HopfieldNetwork
      * @param1 neuronWeights (if left default, creates an empty network)
      * @param2 neuronValues (if left default, all TRUE)
      * @param3 neuronCount (if left default, is derived)
      */
    HopfieldNetwork(const vector< vector<double> > = vector< vector<double> >(),
                    const vector<bool> = vector<bool>(), const unsigned long = 0);

    /**
      * Updates the network if the input is consistent, otherwise raises and error and does nothing.
      * @param1 neuronWeights (if left default, creates an empty network)
      * @param2 neuronValues (if left default, all TRUE)
      * @param3 neuronCount (if left default, is derived)
      * @return whether the network has been updated
      */
    bool updateNetwork(const vector< vector<double> > = vector< vector<double> >(),
                       const vector<bool> = vector<bool>(), const unsigned long = 0);

    /**
      * Uploads a TemperatureModule for the network to use.
      */
    void uploadTemperatureModule(TemperatureModule* const module) {m_temperatureModule=module;}

    /**
      * Sets temperature on the installed TemperatureModule. If none is installed, ignores.
      */
    inline void setTemperature(const unsigned long temperature)
    {if (m_temperatureModule) m_temperatureModule->setTemperature(temperature);}


    /**
      * Calculates the potential of a given neuron, including bias.
      * @param1 position of the neuron
      * @return potential
      */
    double calculatePotential(const unsigned long) const;

    /**
      * Processes a neuron by calculating potential and changing its value accordingly.
      * @param1 position of the neuron
      * @return error code (NO_ERROR=0)
      */
    errorCode processNeuron(const unsigned long);

    /**
      * Computes the network sequentially until an equilibrium is achieved or the (optional) maximum number of steps is reached.
      * @param1 (pointer to) maximum number of steps - infinite if NULL(default) - stores the number of steps taken if 0
      * @return whether an equilibrium has been achieved
      */
    bool computeSequentially(unsigned long* const = NULL);

    /**
      * Computes the network in random order until an equilibrium is achieved or the (optional) maximum number of steps is reached.
      * @param1 (pointer to) maximum number of steps - infinite if NULL(default) - stores the number of steps taken if 0
      * @return whether an equilibrium has been achieved
      */
    bool computeRandomly(unsigned long* const= NULL);

    /**
      * Computes the network by processing neurons in random permutations
      * @param1 (pointer to) maximum number of steps - infinite if NULL(default) - stores the number of steps taken if 0
      * @return whether an equilibrium has been achieved
      */
    bool computeRandomSeq(unsigned long* const= NULL);


    /**
      * Loads the network from a given file.
      * @param1 path to the file
      * @param2 whether the network has been succesfully loaded
      */
    bool loadFromFile(const char*);

    /**
      * Prints weigthts of the network to the given output.
      * @param given output (default cout)
      */
    void printWeights (ostream& out = cout) const;

    void printPath(ostream& out = cout) const;

    void printEnergy(ostream& out = cout) const;

    void printEnergy2(ostream& out = cout) const;

    /**
      * Loads the network from the given input.
      * @param1 input
      * @param2 network
      * @return input
      */
    friend istream& operator>> (istream&, HopfieldNetwork&);

    /**
      * Prints the neuronCount and neuronValues to the given output.
      * @param1 output
      * @param2 network
      * @return output
      */
    friend ostream& operator<< (ostream&, HopfieldNetwork&);

};



#endif // NETWORK_H
