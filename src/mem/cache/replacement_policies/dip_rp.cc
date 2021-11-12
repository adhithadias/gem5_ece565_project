#include "mem/cache/replacement_policies/dip_rp.hh"

#include <cassert>
#include <memory>

#include "params/DIPRP.hh"

DIPRP::DIPRP(const Params *p)
    : BaseReplacementPolicy(p)
{
}

void
DIPRP::invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
const
{
    // Reset last touch timestamp
    std::static_pointer_cast<LRUReplData>(
        replacement_data)->lastTouchTick = Tick(0);
}

void
DIPRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Update last touch timestamp
    std::static_pointer_cast<LRUReplData>(
        replacement_data)->lastTouchTick = curTick();
}

void
DIPRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    // Set last touch timestamp
    std::static_pointer_cast<LRUReplData>(
        replacement_data)->lastTouchTick = curTick();
}

// TODO: Directly copied from LRU -- this needs to be changed 
// according to set dueling
ReplaceableEntry*
DIPRP::getVictim(const ReplacementCandidates& candidates) const
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Visit all candidates to find victim
    ReplaceableEntry* victim = candidates[0];
    for (const auto& candidate : candidates) {
        // Update victim entry if necessary
        if (std::static_pointer_cast<DIPReplData>(
                    candidate->replacementData)->lastTouchTick <
                std::static_pointer_cast<DIPReplData>(
                    victim->replacementData)->lastTouchTick) {
            victim = candidate;
        }
    }

    return victim;
}

std::shared_ptr<ReplacementData>
DIPRP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new DIPReplData());
}

DIPRP*
DIPRPParams::create()
{
    return new DIPRP(this);
}
