#ifndef TEMPERATUREMODULE_H
#define TEMPERATUREMODULE_H

#include <vector> /* vector */
#include <math.h> /* log1p, pow */

#include <iostream>

// CAN BE A SUBJECT OF OPTIMIZATION
// defines when we consider the system temperature to be nonzero
#define ZERO_THRESHOLD 0.5L

using std::vector;

/**
  * Base class of all temperature modules
  */
class TemperatureModule
{
protected:
    double m_temperature; // current temperature

public:
    TemperatureModule(const double temperature = 0.0L):m_temperature(temperature){}

    // returns whether we consider the network cooled down
    virtual inline bool isHot() const {return m_temperature>ZERO_THRESHOLD;}

    virtual inline double getTemperature() const {return m_temperature;}
    virtual inline void setTemperature(double temperature) {m_temperature=temperature;}

    /**
      * Perform one step of cooling down. To be implemented properly in children.
      */
    virtual void coolDown(){}
};


/**
  * Base class for modules that are not time independent.
  */
class TimeBasedTemperatureModule: public TemperatureModule
{
protected:
    unsigned long long int m_timeElapsed; // time elapsed since the temperature was set
public:
    TimeBasedTemperatureModule(const double temperature = 0.0L): TemperatureModule(temperature), m_timeElapsed(0){}
    virtual inline void setTemperature(double temperature) {TemperatureModule::setTemperature(temperature); m_timeElapsed=0;}
};

/**
  * A module that implements temperature cooling according to the formula: T(t) = n^c T(0)
  */
class ExpTemperatureModule: public TimeBasedTemperatureModule
{
private:
    double m_nValue; // value of n parameter
    unsigned int m_qValue; // value of q parameter

    unsigned int m_nextCoolDown;

public:
    ExpTemperatureModule(const double nValue, const unsigned int qValue, const double temperature = 0):
        TimeBasedTemperatureModule(temperature), m_nValue(nValue), m_qValue(qValue), m_nextCoolDown(qValue){}

    inline void setTemperature(double temperature)
    {TimeBasedTemperatureModule::setTemperature(temperature); m_nextCoolDown=m_qValue;}

    /**
      * Perform one step of cooling down.
      */
    void coolDown();



};

/**
  * A module that implements temperature cooling according to the formula: T(t) = T(0) / log (1+t)
  */
class LogTemperatureModule: public TimeBasedTemperatureModule
{
protected:
    double m_initialTemperature;

public:
    LogTemperatureModule(const double temperature):TimeBasedTemperatureModule(temperature), m_initialTemperature(temperature){}

    inline void setTemperature(double temperature)
    {TimeBasedTemperatureModule::setTemperature(temperature); m_initialTemperature=temperature;}

    /**
      * Perform one step of cooling down.
      */
    virtual void coolDown();

};

class LogTemperatureModuleOpt: public LogTemperatureModule
{
public:
    LogTemperatureModuleOpt(const double temperature):LogTemperatureModule(temperature){}
    /**
      * Perform one step of cooling down.
      */
    void coolDown();
};

#endif // TEMPERATUREMODULE_H
