#include "network.h"
#include "math.h"

errorCode HopfieldNetwork::isInconsistent(const vector< vector<double> > neuronWeights, const vector<bool> neuronValues,
                       const unsigned long neuronCount)
{

    // check consistency of sizes
    if (neuronCount!=neuronValues.size()) return INCONSISTENCY;
    if (neuronCount!=neuronWeights.size()) return INCONSISTENCY;
    for (unsigned long i;i<neuronCount;i++) if (neuronCount!=neuronWeights[i].size()) return INCONSISTENCY;

    // check symmetry of weights
    for (unsigned long i=0; i<neuronCount; i++)
    {
        for (unsigned long j=i; j<neuronCount; j++)
        {
            if (neuronWeights[i][j]!=neuronWeights[j][i])
            {
                return NON_SYMMETRIC;
            }
        }
    }
    return NO_ERROR;
}

bool HopfieldNetwork::updateNetwork(const vector< vector<double> > neuronWeights, const vector<bool> inNeuronValues,
                                    const unsigned long inNeuronCount)
{
    // get neuronCount if left default
    const unsigned long neuronCount = inNeuronCount ? inNeuronCount : neuronWeights.size();

    // get neuronValues if left default
    const vector<bool> neuronValues = inNeuronValues.empty() ? vector<bool>(neuronWeights.size(), true) : inNeuronValues;

    // check consistency of input
    errorCode error=isInconsistent(neuronWeights, neuronValues, neuronCount);

    if (error)
    {
        // if inconsistent, raise an error and do nothing
        raiseError(error);
        return false;
    }
    else
    {
        // if consistent, update the network
        m_neuronWeights=neuronWeights;
        m_neuronValues=neuronValues;
        m_neuronCount=neuronCount;
        return true;
    }

    raiseError(UNKNOWN_ERROR);
    return false;
}

HopfieldNetwork::HopfieldNetwork(const vector< vector<double> > neuronWeights,
                const vector<bool> neuronValues, const unsigned long neuronCount):
    m_neuronWeights(), m_neuronValues(), m_neuronCount(0), m_temperatureModule(NULL)
{
    static bool seedUsed=false;
    if (!seedUsed)
    {
        srand (time(NULL));
        seedUsed=true;
    }
    // attempt to update the network, checking consistency
    updateNetwork(neuronWeights, neuronValues, neuronCount);
}

double HopfieldNetwork::calculatePotential(const unsigned long neuron) const
{
    double potential=0;

    for (unsigned long i=0; i<m_neuronCount; i++)
    {
        if (i==neuron)
        {
            // weight[i][i] is -bias
            potential+=m_neuronWeights[neuron][i];
        }
        else
        {
            // addend of the scalar product of weights and values vectors
            potential+=m_neuronValues[i]*m_neuronWeights[neuron][i];
        }
    }
    return potential;
}


// return the number of bits of rand() result
unsigned short randBits()
{
    unsigned short result=0;
    bool done=false;

    unsigned long tempMaxRand=RAND_MAX;
    unsigned long bitPos=1;
    while (bitPos)
    {
        if (tempMaxRand&bitPos)
        {
            if (done)
            {
                raiseError(RANDOMIZATION);
                return 0;
            }
            else
            {
                // still a bit that is being randomized
                result++;
            }
        }
        bitPos=bitPos<<1;
    }
    return result;
}

// return a random number of any size
template<typename T>
inline T randomNumber()
{
    const static unsigned short bitCount=randBits();
    unsigned short bitsLeft=8*sizeof(T);
    T result=0;
    unsigned long temp;

    while (bitsLeft>0)
    {
        result=result<<bitCount;
        bitsLeft-=bitCount;

        temp=rand();
        result=result|temp;
    }
    return result;
}

/**
  * Perform a Bernoulli trial.
  * @param1 probability of success or its multiplicative inverse.
  * @return result of the trial
  */
bool BernoulliTrial(double input)
{
    unsigned long threshold=0;
    unsigned long random=randomNumber<unsigned long>();

    // probability 1 to succeed and 1 out of 1 is the same
    if (input==1)
    {
        return true;
    }
    else
    {
        if (input>1)
        {
            // we want one try out of 'input' to be successful
            threshold=ULONG_MAX/input;
        }
        else
        {
            threshold=ULONG_MAX*input;
            // we want the probability to succeed to be 'input'
        }
    }

    return (threshold>=random);
}


errorCode HopfieldNetwork::processNeuron(const unsigned long neuron)
{
    if (neuron<m_neuronCount)
    {
        const double potential=calculatePotential(neuron);
        // if potential is zero, keep the value of the neuron
        if (potential)
        {
            // if temperature module is set up
            if (m_temperatureModule&&m_temperatureModule->isHot())
            {
                // the chance is oneInX
                const double oneInX=1+exp((-2)*potential / m_temperatureModule->getTemperature());
                m_neuronValues[neuron]=BernoulliTrial(oneInX);
                m_temperatureModule->coolDown();
            }
            else
            {
                m_neuronValues[neuron]=(potential>=0);
            }
        }
        return NO_ERROR;
    }
    else
    {
        return OUT_OF_BOUNDS;
    }

    // should never get here
    return UNKNOWN_ERROR;
}

bool HopfieldNetwork::computeSequentially(unsigned long* const maxSteps)
{
    errorCode error=UNKNOWN_ERROR;

    unsigned long currentSteps=0;
    unsigned long currentNeuron=0;
    unsigned long lastChangedNeuron=0;
    bool priorValue;

    // process all neurons sequentially
    while (maxSteps==NULL || *maxSteps==0 || currentSteps<*maxSteps)
    {
        currentSteps++;
        priorValue=m_neuronValues[currentNeuron]; // get original value
        error = processNeuron(currentNeuron); // process the neuron

        if (error)
        {
            // processing raised an error
            raiseError(error);
        }
        else
        {
            if (priorValue!=m_neuronValues[currentNeuron])
            {
                // value of a neuron has changed
                lastChangedNeuron=currentNeuron;
            }
            else
            {
                // if equilibrium attained
                if (currentNeuron==lastChangedNeuron)
                {
                    if (maxSteps!=NULL && *maxSteps==0) *maxSteps=currentSteps;
                    return true;
                }
            }
        }
        currentNeuron++; // proceed to next neuron
        currentNeuron%=m_neuronCount;
    }

    // maxSteps used up
    return false;
}

bool HopfieldNetwork::computeRandomly(unsigned long* const maxSteps)
{
    unsigned long unchangedCount=0;
    errorCode error=UNKNOWN_ERROR;

    unsigned long currentSteps=0;
    unsigned long randomNeuron=0;
    bool priorValue;

    // CAN BE A SUBJECT OF OPTIMIZATION
    // defines when to suspect the network is stalling
    const unsigned long CHECK_THRESHOLD=2*m_neuronCount;

    vector<bool> neuronsToCheck=vector<bool>(m_neuronCount, true);

    // CAN BE A SUBJECT OF OPTIMIZATION
    // defines when to check whether all neurons have been checked
    unsigned long nextCheck=2*m_neuronCount;
    unsigned long checkSteps=0;

    // process all neurons in random order
    while (maxSteps==NULL || *maxSteps==0 || currentSteps<*maxSteps)
    {
        currentSteps++;
        randomNeuron=rand()%m_neuronCount;
        priorValue=m_neuronValues[randomNeuron]; // get original value
        error = processNeuron(randomNeuron); // process the neuron

        if (error)
        {
            // processing raised an error
            raiseError(error);
        }
        else
        {
            if (priorValue!=m_neuronValues[randomNeuron])
            {
                // value of a neuron has changed
                if (unchangedCount>CHECK_THRESHOLD)
                {
                    neuronsToCheck.assign(m_neuronCount, false);
                    checkSteps=0;
                    // CAN BE A SUBJECT OF OPTIMIZATION
                    // defines when to check whether all neurons have been checked
                    nextCheck=2*m_neuronCount;
                }
                unchangedCount=0;
            }
            else
            {
                unchangedCount++;
                if (unchangedCount>CHECK_THRESHOLD)
                {
                    neuronsToCheck[randomNeuron]=false;
                    checkSteps++;
                    if (checkSteps==nextCheck)
                    {
                        checkSteps=nextCheck=0;
                        for (unsigned long int i=0;i<m_neuronCount;i++) if (neuronsToCheck[i]) nextCheck++;
                        if (nextCheck)
                        {
                            // CAN BE A SUBJECT OF OPTIMIZATION
                            // defines when to check again whether we are at equilibrium
                            nextCheck=m_neuronCount;
                        }
                        else
                        {
                            // equilibrium attained
                            if (maxSteps!=NULL && *maxSteps==0) *maxSteps=currentSteps;
                            return true;
                        }
                    }
                }
            }
        }
    }

    // maxSteps used up
    return false;
}

inline vector<unsigned long int> createRandomPermutation(const unsigned long elementsCount)

{
    vector<unsigned long int> result;
    result.reserve(elementsCount);
    for (unsigned long int i=0;i<elementsCount;i++) result.push_back(i); // identity permutation
    random_shuffle(result.begin(), result.end()); // randomize to obtain a random permutation

    return result;
}

bool HopfieldNetwork::computeRandomSeq(unsigned long* const maxSteps)
{
    errorCode error=UNKNOWN_ERROR;

    unsigned long currentSteps=0;

    bool changed=false;
    bool priorValue;

    unsigned long elementIndex=0;
    unsigned long element=0;
    vector<unsigned long int> permutation=createRandomPermutation(m_neuronCount);

    // process all neurons in random permutations
    while (maxSteps==NULL || *maxSteps==0 || currentSteps<*maxSteps)
    {
        currentSteps++;
        if (elementIndex==m_neuronCount)
        {
            if (!changed)
            {
                if (maxSteps!=NULL && *maxSteps==0) *maxSteps=currentSteps;
                return true;
            }
            changed=false;
            elementIndex=0;
            permutation=createRandomPermutation(m_neuronCount);
        }
        element=permutation[elementIndex];
        priorValue=m_neuronValues[element]; // get original value
        error = processNeuron(element); // process the neuron

        if (error)
        {
            // processing raised an error
            raiseError(error);
        }
        else if (priorValue!=m_neuronValues[element])
            {
                // value of a neuron has changed
                changed=true;
            }
        elementIndex++; // proceed to next neuron
    }

    // maxSteps used up
    return false;

}


istream& operator>> (istream& in, HopfieldNetwork& network)
{
    in.exceptions(std::istream::failbit | std::istream::badbit);

    try
    {
        // read neuronCount
        unsigned long tempNeuronCount;
        in>>tempNeuronCount;

        // read neuronValues
        vector<bool> tempNeuronValues;
        tempNeuronValues.reserve(tempNeuronCount);
        bool tempBool;
        for (unsigned long i;i<tempNeuronCount;i++)
        {
            in>>tempBool;
            tempNeuronValues.push_back(tempBool);
        }

        // read neuronweights
        vector< vector<double> > tempNeuronWeights(tempNeuronCount, vector<double>());
        double tempDouble;
        for (unsigned long i=0;i<tempNeuronCount;i++)
        {
            tempNeuronWeights[i].reserve(tempNeuronCount);
            for (unsigned long j=0; j<tempNeuronCount;j++)
            {
                in>>tempDouble;
                tempNeuronWeights[i].push_back(tempDouble);
            }
        }

        // try to update the network with the data read
        network.updateNetwork(tempNeuronWeights, tempNeuronValues);
    }
    catch (std::istream::failure e)
    {
        raiseError(READ_FAILURE);
    }
    return in;
}

ostream& operator<< (ostream& out, HopfieldNetwork& network)
{
    // print neuronCount
    out<<network.m_neuronCount;
    out<<endl<<endl;
    unsigned long sqr = (unsigned long)sqrt((double)network.m_neuronCount);
    // print neuronValues
    for (unsigned long i=0;i<network.m_neuronCount;i++)
    {
        out<<network.m_neuronValues[i]<<"\t";
        if (i % sqr == sqr-1) out << endl;
    }
    out<<endl;
    return out;
}



bool HopfieldNetwork::loadFromFile(const char* inFile)
{
    ifstream file(inFile);
    if (file.is_open())
    {
        // load the network
        file>>(*this);
        return true;
    }
    else
    {
        // could not open the file
        raiseError(FILE_NOT_OPEN);
        return false;
    }

    raiseError(UNKNOWN_ERROR);
    return false;
}

void HopfieldNetwork::printWeights (ostream& out) const
{
    for (unsigned long i=0; i < m_neuronCount;i++)
    {
        for (unsigned long j=0; j < m_neuronCount;j++)
        {
            out<<m_neuronWeights[i][j]<<"\t";
        }
        out<<endl;
    }
    out<<endl;
}

void HopfieldNetwork::printPath (ostream& out) const
{
    unsigned long sqr = (unsigned long)sqrt((double)m_neuronCount);
    for (unsigned long i=0; i < sqr; i++)
    {
        for (unsigned long j=0; j< sqr; j++)
        {
            if (m_neuronValues[i + j* sqr] == 1) out<< j << "\t";
        }
    }
    out<<endl;
}

void HopfieldNetwork::printEnergy (ostream& out) const
{
    double E =0.0;
    for (unsigned long i=0; i< m_neuronCount; i++)
    {
        for (unsigned long j=0; j<m_neuronCount; j++)
        {
            if (i!=j) E -=0.5 * m_neuronWeights[i][j]*m_neuronValues[i]*m_neuronValues[j];
        }
        E-= m_neuronWeights[i][i] * m_neuronValues[i];
    }
    out<<E<<endl;

}

void HopfieldNetwork::printEnergy2(ostream& out) const
{
    double result=0.0L;

    for (unsigned long i=0;i<m_neuronCount;i++)
    {
        for (unsigned long j=i;j<m_neuronCount;j++)
        {
            result-=m_neuronWeights[i][j]*m_neuronValues[i]*m_neuronValues[j];
        }
    }
    out<<result<<endl;
}
