#include "temperaturemodule.h"

#define COMPUTED_LOGARITHMS_MAX_INDEX 2

void ExpTemperatureModule::coolDown()
{
    if (++m_timeElapsed==m_nextCoolDown)
    {
        m_nextCoolDown+=m_qValue; // coolDown happens every q time steps
        m_temperature*=m_nValue; // raise exponenent of n
    }
}

void LogTemperatureModule::coolDown()
{
    m_temperature=m_initialTemperature / log1p(++m_timeElapsed); // new temperature value
}

vector<double> initializeComputedLogarithms(const unsigned short size = COMPUTED_LOGARITHMS_MAX_INDEX)
{
    vector<double> result;
    for (unsigned short i=0;i<=size;i++) result.push_back(log1p(i));
    return result;
}

void LogTemperatureModuleOpt::coolDown()
{
    static vector<double> computedLogarithms=initializeComputedLogarithms();
    // for the elements with indices <=computedCount we have data at the moment
    static unsigned long long int computedCount=COMPUTED_LOGARITHMS_MAX_INDEX;

    //bool first_run;


    m_timeElapsed++; // raise time

    if (m_timeElapsed>computedCount)
    {
        // we don't have the data yet
        if (m_timeElapsed&1)
        {
            // t=2k+1
            computedLogarithms.push_back(log1p(m_timeElapsed));
        }
        else
        {
            // t=2k
            // log t=log2 + log k
            static long long k=COMPUTED_LOGARITHMS_MAX_INDEX/2; // the k that was last required
            computedLogarithms.push_back(computedLogarithms[++k]+computedLogarithms[2]);
        }
    }

    m_temperature=m_initialTemperature / computedLogarithms[m_timeElapsed]; // new temperature value
}
