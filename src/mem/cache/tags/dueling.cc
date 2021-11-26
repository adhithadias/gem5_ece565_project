#include "mem/cache/tags/dueling.hh"

#include <iostream>

#include "base/bitfield.hh"
#include "base/logging.hh"

unsigned DuelingMonitor::numInstances = 0;

Dueler::Dueler()
  : _isSample(false), _team(0)
{
}

void
Dueler::setSample(uint64_t id, bool team)
{
    panic_if(popCount(id) != 1, "The id must have a single bit set.");
    panic_if(_isSample & id,
        "This dueler is already a sample for id %llu", id);
    _isSample |= id;
    if (team) {
        _team |= id;
    }
}

bool
Dueler::isSample(uint64_t id, bool& team) const
{
    team = _team & id;
    return _isSample & id;
}

DuelingMonitor::DuelingMonitor(std::size_t constituency_size,
    std::size_t team_size, unsigned num_bits, double low_threshold,
    double high_threshold)
  : id(1 << numInstances), constituencySize(constituency_size),
    teamSize(team_size), lowThreshold(low_threshold),
    highThreshold(high_threshold), selector(num_bits), regionCounter(0),
    winner(true)
{
    fatal_if(constituencySize < (NUM_DUELERS * teamSize),
        "There must be at least team size entries per team in a constituency");
    fatal_if(numInstances > 63, "Too many Dueling instances");
    fatal_if((lowThreshold <= 0.0) || (highThreshold >= 1.0),
        "The low threshold must be within the range ]0.0, 1.0[");
    fatal_if((highThreshold <= 0.0) || (highThreshold >= 1.0),
        "The high threshold must be within the range ]0.0, 1.0[");
    fatal_if(lowThreshold > highThreshold,
        "The low threshold must be below the high threshold");
    numInstances++;

    // Start selector around its middle value
    selector.saturate();
    selector >>= 1;
    if (selector.calcSaturation() < lowThreshold) {
        winner = false;
    }
}

void
DuelingMonitor::sample(const Dueler* dueler)
{
    bool team;

    // // uncomment to see how the PSEL changes
    // std::cout
    //     // << "saturation: " << selector.calcSaturation()
    //     << ", " << selector.getCounter()
    //     // << ", maxVal: " << selector.getMaxVal()
    //     << ", " << id << ", " << team
    //     << ", " << dueler->isSample(id, team)
    //     << std::endl
    //     ;

    if (dueler->isSample(id, team)) {

        if (team) {
            selector++;

            if (selector.calcSaturation() >= highThreshold) {
                winner = true;
            }
        } else {
            selector--;

            if (selector.calcSaturation() < lowThreshold) {
                winner = false;
            }
        }
    }
}

bool
DuelingMonitor::isSample(const Dueler* dueler, bool& team) const
{
    return dueler->isSample(id, team);
}

bool
DuelingMonitor::getWinner() const
{
    return winner;
}

void
DuelingMonitor::initEntry(Dueler* dueler)
{
    // The first entries of the constituency belong to one team, and the
    // last entries to the other
    // std::cout << "initEntry of DuelingMonitor\n";
    // std::cout << "regionCounter: " << regionCounter
    //     << ", teamSize: " << teamSize
    //     << ", constituencySize: " << constituencySize
    //     << ", constituencySize - teamSize: " << constituencySize - teamSize
    //     << std::endl;

    assert(dueler);
    if (regionCounter < teamSize) {
        // std::cout << "setting sample to id: " << id << " false\n";
        dueler->setSample(id, false);
    } else if (regionCounter >= constituencySize - teamSize) {
        // std::cout << "setting sample to id: " << id << " true\n";
        dueler->setSample(id, true);
    }

    // Check if we changed constituencies
    if (++regionCounter >= constituencySize) {
        // std::cout << "we changed constituencies\n";
        regionCounter = 0;
    } else {
        // std::cout << "we did not change constituencies\n";
    }
}
