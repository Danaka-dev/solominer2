#pragma once

// Copyright (c) 2023-2024 The solominer developers
// Distributed under the MIT software license, see the accompanying
// file LICENSE or http://www.opensource.org/licenses/mit-license.php.

//////////////////////////////////////////////////////////////////////////////
#ifndef SOLOMINER_STATS_H
#define SOLOMINER_STATS_H

//////////////////////////////////////////////////////////////////////////////
#include <tiny.h>

#include <cstdlib>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
template <typename T>
struct StatValue_ {
    void Zero() {
        sum = mini = maxi = 0; n = 0;
    }

    void Add( T v ) {
        sum += v;
        mini = (n==0 || v < mini) ? v : mini;
        maxi = (n==0 || v > maxi) ? v : maxi;
        ++n;
    }

    T getAvg() { return (n > 0) ? (sum / n) : 0; }

    T sum ,mini ,maxi;

    int n;
};

typedef StatValue_<double>  StatValue;

//////////////////////////////////////////////////////////////////////////////
class Stats {
public:
    enum TimeUnit {
        MilliSecond ,Second ,Minute ,Hour ,Day ,Week ,Month ,year
    };

protected:
    StatValue *m_values;
    int m_nValues; //! number of stat value in chart

    TimeUnit m_timeUnit; //! smallest fraction

    int m_currentTime;

public:
    Stats( int n=0 ,TimeUnit timeUnit=TimeUnit::Second ) :
        m_values(nullptr) ,m_nValues(0) ,m_timeUnit(timeUnit) ,m_currentTime(0)
    {
        Reset(n);
    }

    ~Stats() {
        delete [] m_values;
    }

    int getStatCount() {
        return m_nValues;
    }

    StatValue &getStat( int t ) {
        int i = (m_nValues + t) % m_nValues;

        return m_values[ CLAMP( i ,0 ,m_nValues ) ];
    }

    int getCurrentTime() {
        return m_currentTime;
    }

public: ///-- recording
    void Start( int t ) {
        m_currentTime = t;
    }

    void Record( double v ,int t ) {
        if( m_values && m_nValues ) {} else return;

        int t0 = MIN( t ,getCurrentTime() );
        int n = (t - t0) + 1;

        //! spreading mean of value from t0 to t
        for( int i=t0; i<=t; ++i ) {
            StatValue &value = getStat(i);

            if( i > m_currentTime ) {
                value.Zero();
            }

            value.Add( v/n );
        }

        m_currentTime = t;
    }

    double getAvg() {
        if( m_values && m_nValues ) {} else return 0.;

        double sum = 0.; int n=0;

        for( int i=0; i<m_nValues; ++i ) if( m_values[i].sum > 0 ) {
            sum += m_values[i].sum; ++n;
        }

        return n ? sum / n : 0.;
    }

public:
    void Reset( int n ) {
        if( m_values ) {
            delete[] m_values; m_values = nullptr; m_nValues = 0;
        }

        if( (m_nValues = n) > 0  ) {
            m_values = new StatValue[m_nValues];

            for( int i=0; i<n; ++i ) {
                m_values[i].Zero();
            }
        }
    }

    /* void Record( double v ,int t ) {
        if( m_values && m_nValues ) {} else return;

        StatValue &value = getStat(t);

        if( t != m_currentTime ) {
            value.Zero(); m_currentTime = t;
        }

        value.Add( v );
    } */

    void findBounds( double &mini ,double &maxi ) {
        for( int i=0; i<m_nValues; ++i ) {
            double avg = m_values[i].sum; // getAvg(); // => both MODE //TODO sort that out

            mini = (i==0 || mini > avg) ? avg : mini;
            maxi = (i==0 || maxi < avg) ? avg : maxi;
        }
    }

    void findLimits( double &mini ,double &maxi ) {
        for( int i=0; i<m_nValues; ++i ) {
            StatValue &v = m_values[i];

            mini = (i==0 || mini > v.mini) ? v.mini : mini;
            maxi = (i==0 || maxi < v.maxi) ? v.maxi : maxi;
        }
    }

public:
    // ... void convertTimeUnit( Stats &stats ) {}
};

//////////////////////////////////////////////////////////////////////////////
#endif //SOLOMINER_STATS_H