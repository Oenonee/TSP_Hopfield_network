#include <iostream> /* cerr, cout, ostream */

#include "network.h"
#include "problems.h"

using std::cerr;
using std::cout;
using std::ostream;

using namespace problems;

enum networkMode {SEQUENTIAL = 0, RANDOM, RANDOMSEQ};

inline unsigned long testNumberOfSteps(HopfieldNetwork network, networkMode mode, ostream& out = cout)
{
    unsigned long steps=0;
    switch (mode)
    {
    case SEQUENTIAL:
        network.computeSequentially(&steps);
        break;
    case RANDOM:
        network.computeRandomly(&steps);
        break;
    case RANDOMSEQ:
        network.computeRandomSeq(&steps);
        break;
    default:
        // should never get here
        cerr<<"Step test for unknown mode. Ignoring";
    }

    return steps;
}

int main(int argc, char *argv[])
{
    HopfieldNetwork network;
    //network.loadFromFile("HopfieldNetwork.txt");

    //network=createQueenProblem(8);
    network = createTSP("tsp_input.txt",20);
    LogTemperatureModule module(30);
    //ExpTemperatureModule module(0.995, network.getNeuronCount(), 40);
    network.uploadTemperatureModule(&module);
    network.computeRandomly();
    cout<<network<<endl;
    network.printPath();
    network.printEnergy2();
    //network.printWeights();

//    ExpTemperatureModule module(0.9, network.getNeuronCount(), 100);
//    network.uploadTemperatureModule(&module);
//    cout<<testNumberOfSteps(network, static_cast<networkMode>(0))<<endl;
//    cout << network << endl;

//    network=createRookProblem(4);
//    cout<<testNumberOfSteps(network, static_cast<networkMode>(0))<<endl;
//    cout << network << endl;

//    network = createTSP("tsp_small.txt", 10.0);
//    network.printWeights();

    //for (unsigned int i=0;i<3;i++)
    //    cout<<testNumberOfSteps(network, static_cast<networkMode>(i))<<endl;

    //network.computeSequentially();
    //network.computeRandomly();
    //network.computeRandomSeq();
    //cout<<network;

    
    return 0;
}
