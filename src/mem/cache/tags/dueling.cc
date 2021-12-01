#include "mem/cache/tags/dueling.hh"

#include <iomanip>
#include <iostream>

#include "base/bitfield.hh"
#include "base/logging.hh"
#include "params/DIPRP.hh"
#include "sim/sim_object.hh"

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
    double high_threshold, unsigned assoc, unsigned block_offset,
    unsigned set_offset, unsigned num_sets)
  : id(1 << numInstances), constituencySize(constituency_size),
    teamSize(team_size), lowThreshold(low_threshold),
    highThreshold(high_threshold), selector(num_bits), regionCounter(0),
    winner(true), assoc(assoc), blockOffset(block_offset),
    setOffset(set_offset), numSets(num_sets), pselLogTick(0)
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
    std::cout << "initializing new dueling monitor instance: " << numInstances
        << ", id: " << id << ", " << (1<<numInstances) << std::endl;
    selector.saturate();
    selector >>= 1;
    if (selector.calcSaturation() < lowThreshold) {
        winner = false;
    }
}

void
DuelingMonitor::sample(const ReplaceableEntry* rd)
{
    bool team;
    bool is_sample = isSample(rd, team);

    // // uncomment to see how the PSEL changes
    std::cout
        // << "saturation: " << selector.calcSaturation()
        << selector.getCounter()
        // << ", maxVal: " << selector.getMaxVal()
        << ", " << id << ", " << team
        << ", " << is_sample
        << ", " << curTick()
        << ", " << winner
        << std::endl
        ;

    if (is_sample) {
        if (team) { // a miss in LRU increments the PSEL
            selector++;

            if (selector.calcSaturation() >= highThreshold) {
                winner = true; // select BIP policy
            }
        } else {
            selector--;

            if (selector.calcSaturation() < lowThreshold) {
                winner = false; // select BIP policy
            }
        }
    }
}

void
DuelingMonitor::sample(const Addr addr) {
    bool team;
    bool is_sample = isSample(addr, team);
    uint64_t setIndex = (addr >> blockOffset) & (numSets-1);
    uint64_t constituencyIndex = setIndex & (constituencySize-1);

    // std::cout << std::hex
        // << "sample at replace addr: " << addr << std::endl;
    // std::cout << std::dec;

    Tick t = curTick();
    if (t-pselLogTick > 100000000) {
        // std::cout << t << ", " << selector.getCounter() << std::endl;
        std::cout
            << t
            << ", " << selector.getCounter()
            << ", " << team
            << ", " << winner
            << ", " << setIndex
            << ", " << constituencyIndex
            << ", " << is_sample
            << std::endl
        ;
        pselLogTick = t;
    }

    // PSEL counter is the Policy Selector
    if (is_sample) {
        if (team) { // a miss in LRU increments the PSEL
            selector++;
            if (selector.calcSaturation() >= highThreshold) {
                // select BIP policy !winner should be selected
                winner = true;
            }
        } else {
            selector--;
            if (selector.calcSaturation() < lowThreshold) {
                // select LRU policy !winner should be selected
                winner = false;
            }
        }

    }
}

bool
DuelingMonitor::isSample(const Addr addr, bool& team)
{

    uint64_t shifted = (addr >> blockOffset);
    uint64_t setIndex = shifted & (numSets-1);
    uint64_t constituencyIndexMask = constituencySize-1;
    // uint64_t constituency = setIndex
        // & (~constituencyIndexMask); // 5 high order bits

    uint64_t constituencyIndex = setIndex
        & constituencyIndexMask; // 5 low order bits
    // complement of the 5 low order bits
    // uint64_t complementaryOfConstituencyIndex =
        // (~setIndex) & constituencyIndexMask;

    // std::cout << "blockOffset: " << blockOffset << std::endl;
    // std::cout << "constituencySize: " << constituencySize << std::endl;
    // std::cout << "teamSize: " << teamSize << std::endl;
    // // accessed block will be always
    // // zero because first block addr is sent here
    // std::cout << "accessed block: " << (addr & blockOffset) << std::endl;
    // std::cout << std::hex << "addr: " << addr << std::endl;
    // std::cout << std::hex << "shifted: " << shifted << std::endl;

    // std::cout
    //     << "addr: " << addr
    //     << ", set: " << setIndex
    //     << ", constituencyIndex: " << constituencyIndex
    //     << std::endl;

    std::cout << std::dec;
    if (constituencyIndex < teamSize) { // LRU master sets at the begninng
        team = true;
        return true;
    } else if (constituencySize - teamSize <= constituencyIndex) {
        team = false;
        return true;
    }

    // if (constituency == constituencyIndex) {
    //     // std::cout << "policy 1 set\n";
    //     team = true;
    //     return true;
    // } else if (constituency == complementaryOfConstituencyIndex) {
    //     // std::cout << "policy 2 set\n";
    //     team = false;
    //     return true;
    // }
    // std::cout << "not a master set\n";

    return false;
}

bool
DuelingMonitor::isSample(const ReplaceableEntry* rd, bool& team) const
{
    uint64_t setIndex = rd->getSet();
    uint64_t constituencyIndexMask = constituencySize-1;
    // uint64_t constituency = setIndex
    //     & (~constituencyIndexMask); // 5 high order bits

    uint64_t constituencyIndex = setIndex
        & constituencyIndexMask; // 5 low order bits
    // complement of the 5 low order bits
    // uint64_t complementaryOfConstituencyIndex = (~setIndex)
    //     & constituencyIndexMask;
    std::cout << std::dec;

    std::cout << std::dec;
    if (constituencyIndex < teamSize) { // LRU master sets at the begninng
        team = true;
        return true;
    } else if (constituencySize - teamSize <= constituencyIndex) {
        team = false;
        return true;
    }

    // if (constituency == constituencyIndex) {
    //     team = true;
    //     return true;
    // } else if (constituency == complementaryOfConstituencyIndex) {
    //     team = false;
    //     return true;
    // }

    team = true; // just to initialize the value
    return false;
    // return dueler->isSample(id, team);
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
