/* Copyright 2021 Franz Poeschel
 *
 * This file is part of openPMD-api.
 *
 * openPMD-api is free software: you can redistribute it and/or modify
 * it under the terms of of either the GNU General Public License or
 * the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * openPMD-api is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License and the GNU Lesser General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License
 * and the GNU Lesser General Public License along with openPMD-api.
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include "openPMD/ReadIterations.hpp"

#include "openPMD/Series.hpp"

namespace openPMD
{

SeriesIterator::SeriesIterator() : m_series()
{
}

SeriesIterator::SeriesIterator( Series series )
    : m_series( std::move( series ) )
{
    auto it = series.get().iterations.begin();
    if( it == series.get().iterations.end() )
    {
        *this = end();
        return;
    }
    else
    {
        auto openIteration = []( Iteration & iteration )
        {
            /*
             * @todo
             * Is that really clean?
             * Use case: See Python ApiTest testListSeries:
             * Call listSeries twice.
             */
            if( *iteration.m_closed !=
                Iteration::CloseStatus::ClosedInBackend )
            {
                iteration.open();
            }
        };
        AdvanceStatus status{};
        switch( series.iterationEncoding() )
        {
        case IterationEncoding::fileBased:
            /*
             * The file needs to be accessed before beginning a step upon it.
             * In file-based iteration layout it maybe is not accessed yet,
             * so do that now. There is only one step per file, so beginning
             * the step after parsing the file is ok.
             */

            openIteration( series.iterations.begin()->second );
            status = it->second.beginStep();
            for( auto const & pair : m_series.get().iterations )
            {
                m_iterationsInCurrentStep.push_back( pair.first );
            }
            break;
        case IterationEncoding::groupBased:
        case IterationEncoding::variableBased: {
            /*
             * In group-based iteration layout, we have definitely already had
             * access to the file until now. Better to begin a step right away,
             * otherwise we might get another step's data.
             */
            Iteration::BeginStepStatus::AvailableIterations_t
                availableIterations;
            std::tie( status, availableIterations ) = it->second.beginStep();
            if( availableIterations.has_value() )
            {
                m_iterationsInCurrentStep = availableIterations.get();
                if( !m_iterationsInCurrentStep.empty() )
                {
                    openIteration( series.iterations.at(
                        m_iterationsInCurrentStep.at( 0 ) ) );
                }
            }
            else if ( !series.iterations.empty() )
            {
                /*
                 * Fallback implementation: Assume that each step corresponds
                 * with an iteration in ascending order.
                 */
                m_iterationsInCurrentStep =
                    { series.iterations.begin()->first };
                openIteration( series.iterations.begin()->second );
            }
            else
            {
                // this is a no-op, but let's keep it explicit
                m_iterationsInCurrentStep = {};
            }

            break;
        }
        }

        if( status == AdvanceStatus::OVER )
        {
            *this = end();
            return;
        }
        if( !setCurrentIteration() )
        {
            *this = end();
            return;
        }
        it->second.setStepStatus( StepStatus::DuringStep );
    }
}

SeriesIterator & SeriesIterator::operator++()
{
    if( !m_series.has_value() )
    {
        *this = end();
        return *this;
    }
    Series & series = m_series.get();
    auto & iterations = series.iterations;
    auto & currentIteration = iterations[ m_currentIteration ];
    auto oldIterationIndex = m_currentIteration;

    m_iterationsInCurrentStep.pop_front();
    if( !m_iterationsInCurrentStep.empty() )
    {
        m_currentIteration = *m_iterationsInCurrentStep.begin();
        switch( series.iterationEncoding() )
        {
        case IterationEncoding::groupBased:
        case IterationEncoding::variableBased:
        {
            if( !currentIteration.closed() )
            {
                *currentIteration.m_closed =
                    Iteration::CloseStatus::ClosedInFrontend;
            }
            auto begin = series.iterations.find( oldIterationIndex );
            auto end = begin;
            ++end;
            series.flush_impl(
                begin,
                end,
                FlushLevel::UserFlush,
                /* flushIOHandler = */ true );

            series.iterations[ m_currentIteration ].open();
            return *this;
        }
        case IterationEncoding::fileBased:
            if( !currentIteration.closed() )
            {
                currentIteration.close();
            }
            series.iterations[ m_currentIteration ].open();
            series.iterations[ m_currentIteration ].beginStep();
            return *this;
        }
        throw std::runtime_error( "Control flow error!" );
    }

    if( series.iterationEncoding() == IterationEncoding::fileBased )
    {
        // this one is handled above, stream is over once it proceeds to here
        *this = end();
        return *this;
    }

    if( !currentIteration.closed() )
    {
        currentIteration.close();
    }

    // since we are in group-based iteration layout, it does not
    // matter which iteration we begin a step upon
    AdvanceStatus status;
    Iteration::BeginStepStatus::AvailableIterations_t availableIterations;
    std::tie( status, availableIterations ) = currentIteration.beginStep();
    if( availableIterations.has_value()
        && status != AdvanceStatus::RANDOMACCESS )
    {
        m_iterationsInCurrentStep = availableIterations.get();
    }
    else
    {
        /*
         * Fallback implementation: Assume that each step corresponds
         * with an iteration in ascending order.
         */
        auto it = iterations.find( m_currentIteration );
        auto itEnd = iterations.end();
        if( it == itEnd )
        {
            *this = end();
            return *this;
        }
        ++it;
        if( it == itEnd )
        {
            *this = end();
            return *this;
        }
        m_iterationsInCurrentStep = { it->first };
    }
    setCurrentIteration();

    if( status == AdvanceStatus::OVER )
    {
        *this = end();
        return *this;
    }
    currentIteration.setStepStatus( StepStatus::DuringStep );

    auto iteration = iterations.at( m_currentIteration );
    if( *iteration.m_closed != Iteration::CloseStatus::ClosedInBackend )
    {
        iteration.open();
    }
    return *this;
}

IndexedIteration SeriesIterator::operator*()
{
    return IndexedIteration(
        m_series.get().iterations[ m_currentIteration ], m_currentIteration );
}

bool SeriesIterator::operator==( SeriesIterator const & other ) const
{
    return this->m_currentIteration == other.m_currentIteration &&
        this->m_series.has_value() == other.m_series.has_value();
}

bool SeriesIterator::operator!=( SeriesIterator const & other ) const
{
    return !operator==( other );
}

SeriesIterator SeriesIterator::end()
{
    return SeriesIterator{};
}

ReadIterations::ReadIterations( Series series )
    : m_series( std::move( series ) )
{
}

ReadIterations::iterator_t ReadIterations::begin()
{
    return iterator_t{ m_series };
}

ReadIterations::iterator_t ReadIterations::end()
{
    return SeriesIterator::end();
}
} // namespace openPMD
