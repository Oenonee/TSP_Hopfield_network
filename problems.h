#ifndef PROBLEMS_H
#define PROBLEMS_H

#include "network.h"

/**
  * Namespace which sets up a Hopfield network to solve specific problems.
  */
namespace problems
{

/**
  * Creates a Hopfield network for the 'Rook problem' of a given board size.
  * @param1 board_size
  * @param2 progress mode of the network
  * @return network for the 'Rook problem'
  */
HopfieldNetwork createRookProblem(const unsigned int);

/**
  * Creates a Hopfield network for the 'Queen problem' of a given board size.
  * @param1 board_size
  * @param2 progress mode of the network
  * @return network for the 'Queen problem'
  */
HopfieldNetwork createQueenProblem(const unsigned int);

HopfieldNetwork createTSP(std::string fileName, double delta);

}


#endif // PROBLEMS_H
