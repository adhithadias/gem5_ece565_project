#include "mem/cache/replacement_policies/dip_rp.hh"

#include <cassert>
#include <iomanip>
#include <iostream>
#include <memory>

#include "params/DIPRP.hh"

DIPRP::DIPRP(const Params *p)
    : BaseReplacementPolicy(p),
    replacementPolicy1(p->replacement_policy_1),
    replacementPolicy2(p->replacement_policy_2),
    params(p),
    duelingMonitor(p->constituency_size, p->team_size,
        10, 0.5, 0.5,
        p->assoc, p->block_offset, p->set_offset,
        p->num_sets)
{
    std::cout << "initiating new DIP repl policy instance\n";
    fatal_if((replacementPolicy1 == nullptr) ||
             (replacementPolicy2 == nullptr),
            "Replacement policies passed must be instantiated");
}

void
DIPRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    // Reset last touch timestamp and selected policy cache line
    std::shared_ptr<DIPReplData> dip_replacement_data =
        std::static_pointer_cast<DIPReplData>(replacement_data);
    // dip_replacement_data->lastTouchTick = Tick(0);
    replacementPolicy1->invalidate(dip_replacement_data->replacementData1);
    replacementPolicy2->invalidate(dip_replacement_data->replacementData2);
}

void
DIPRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update last touch timestamp and selected cacheline data
    std::shared_ptr<DIPReplData> dip_replacement_data =
        std::static_pointer_cast<DIPReplData>(replacement_data);
    // dip_replacement_data->lastTouchTick = curTick();
    replacementPolicy1->touch(dip_replacement_data->replacementData1);
    replacementPolicy2->touch(dip_replacement_data->replacementData2);
}


void
DIPRP::reset(const std::shared_ptr<ReplacementData>& replacement_data,
    const PacketPtr pkt)
{
    // std::cout << "resetting the data ?insertion\n";
    // std::cout
    //     << " Addr : " << std::hex << std::setw(8) << std::setfill('0')
    //     << pkt->getAddr() << std::endl;
    // std::cout << std::dec;

    // calculate the set from the address set is remove block offset
    // then the next few bits denotes the set
    // check if the set belongs to LRU or BIP
    // depending on the set either increase or decrease the sat counter
    // inside the dueling monitor, address could be passed to dueling
    // monitor, keep the block offset and masks inside the dueling
    // monitor for ease of calculatin of isSample and set

    // reset, touch and victim selection should be according to the
    // selected policy? we can just keep one data point in the DIPRP

    std::shared_ptr<DIPReplData> dip_replacement_data =
        std::static_pointer_cast<DIPReplData>(replacement_data);
    replacementPolicy1->reset(dip_replacement_data->replacementData1, pkt);
    replacementPolicy2->reset(dip_replacement_data->replacementData2, pkt);


    // A miss in a set is a sample to the duel. A call to this function
    // implies in the replacement of an entry, which was either caused by
    // a miss, an external invalidation, or the initialization of the table
    // entry (when warming up)
    duelingMonitor.sample(pkt->getAddr());
    // duelingMonitor.sample(replacement_data.get());
}

/*
 * insert timestamp is set in the reset method
 * LRU method sets the timestamp to MRU timestamp
 * BIP sets 3% time to MRU (like in LRU) and sets
 * timestamp to LRU the rest of the time
 */
void
DIPRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Set last touch timestamp
    std::shared_ptr<DIPReplData> dip_replacement_data =
        std::static_pointer_cast<DIPReplData>(replacement_data);
    dip_replacement_data->lastTouchTick = curTick();
    replacementPolicy1->reset(dip_replacement_data->replacementData1);
    replacementPolicy2->reset(dip_replacement_data->replacementData2);

    // duelingMonitor.sample(pkt->getAddr());
    // duelingMonitor.sample(dip_replacement_data.get());
    // duelingMonitor.sample(
    //     static_cast<ReplacementData*>(dip_replacement_data.get()));
}

// TODO: Directly copied from LRU -- this needs to be changed 
// according to set dueling
// candidates size is the associativity of the cache
ReplaceableEntry*
DIPRP::getVictim(const ReplacementCandidates& candidates, const Addr addr)
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);
    // std::cout << "candidates size: " << candidates.size() << ", team_size: "
    //     << params->team_size << std::endl;
    // panic_if(candidates.size() != params->team_size,
    //     "We currently only "
    //     "support team sizes that match the number of "
    //     "replacement candidates");
    panic_if(candidates.size() != params->assoc, "We currently only "
        "support team sizes that match the number of replacement candidates");

    bool winner = !duelingMonitor.getWinner();
    bool team;

    bool is_sample = duelingMonitor.isSample(addr, team);
    // std::cout << " with address DIPRP::getVictim winner: "
    //     << !winner << std::endl;

    is_sample = duelingMonitor.isSample(candidates[0], team);

    bool team_a;
    // if ((is_sample && !team) || (!is_sample && !winner)) {
    //     team_a = true;
    // } else {
    //     team_a = false;
    // }
    if (is_sample) {
        team_a = team;
    } else {
        team_a = winner;
    }
    // std::cout << "team: " << team_a << std::endl;
    // if (is_sample) {
    //     team_a = !team;
    // } else {
    //     team_a = !winner;
    // }

    std::vector<std::shared_ptr<ReplacementData>> dip_replacement_data;
    for (auto& candidate : candidates) {
        std::shared_ptr<DIPReplData> dip_repl_data =
            std::static_pointer_cast<DIPReplData>(
            candidate->replacementData);

        // As of now we assume that all candidates are either part of
        // the same sampled team, or are not samples.
        // class ReplaceableEntry;
        bool candidate_team;
        panic_if(
            duelingMonitor.isSample(candidate, candidate_team) &&
            (team != candidate_team),
            "Not all sampled candidates belong to the same set");

        // Copy the original entry's data, re-routing its replacement data
        // to the selected one
        dip_replacement_data.push_back(dip_repl_data);
        candidate->replacementData = team_a ? dip_repl_data->replacementData1 :
            dip_repl_data->replacementData2;
    }

    // // uncomment to see how the PSEL changes
    // std::cout
    //     << "" << team_a
    //     << ", " << curTick()
    //     ;
    ReplaceableEntry* victim = team_a ?
        replacementPolicy1->getVictim(candidates) :
        replacementPolicy2->getVictim(candidates);

    for (int i = 0; i < candidates.size(); i++) {
        candidates[i]->replacementData = dip_replacement_data[i];
    }

    return victim;
}


ReplaceableEntry*
DIPRP::getVictim(const ReplacementCandidates& candidates) const
{
    panic_if(true, "DIP getVictim method must be called with the address");
    return nullptr;
}


std::shared_ptr<ReplacementData>
DIPRP::instantiateEntry()
{
    DIPReplData* replacement_data = new DIPReplData(
        replacementPolicy1->instantiateEntry(),
        replacementPolicy2->instantiateEntry());
    duelingMonitor.initEntry(static_cast<Dueler*>(replacement_data));
    return std::shared_ptr<DIPReplData>(replacement_data);
}

DIPRP*
DIPRPParams::create()
{
    return new DIPRP(this);
}
