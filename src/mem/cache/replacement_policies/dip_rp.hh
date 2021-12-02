#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_DIP_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_DIP_RP_HH__

#include <memory>

#include "base/compiler.hh"
#include "base/statistics.hh"
#include "mem/cache/replacement_policies/base.hh"
#include "mem/cache/replacement_policies/bip_rp.hh"
#include "mem/cache/tags/dueling.hh"

struct DIPRPParams;

class DIPRP : public BIPRP
{
    protected:
        /** DIP-specific implementation of replacement data. */

        BaseReplacementPolicy* const replacementPolicy1;
        BaseReplacementPolicy* const replacementPolicy2;
        const DIPRPParams* params;

        mutable DuelingMonitor duelingMonitor;


    public:
        /** Convenience typedef. */
        typedef DIPRPParams Params;

        /**
        * Construct and initiliaze this replacement policy.
        */
        // PARAMS(DIPRP);
        DIPRP(const Params *p);

        /**
        * Destructor.
        */
        ~DIPRP() {}

        // /**
        // * Invalidate replacement data to set it as the next probable victim.
        // * Sets its last touch tick as the starting tick.
        // *
        // * @param replacement_data Replacement data to be invalidated.
        // */
        // void invalidate(const std::shared_ptr<ReplacementData>&
        //     replacement_data) const override;

        // /**
        // * Touch an entry to update its replacement data.
        // * Sets its last touch tick as the current tick.
        // *
        // * @param replacement_data Replacement data to be touched.
        // */
        // void touch(const std::shared_ptr<ReplacementData>&
        //     replacement_data) const override;


        /**
        * Reset replacement data. Used when an entry is inserted.
        * Sets its last touch tick as the current tick.
        *
        * @param replacement_data Replacement data to be reset.
        */
        void reset(const std::shared_ptr<ReplacementData>& replacement_data,
            const PacketPtr pkt) override;
        void reset(
            const std::shared_ptr<ReplacementData>& replacement_data) const
            override;

        // /**
        // * Find replacement victim using LRU timestamps.
        // *
        // * @param candidates Replacement candidates,
        // * selected by indexing policy.
        // * @return Replacement entry to be replaced.
        // */
        // ReplaceableEntry* getVictim(const ReplacementCandidates& candidates,
        //     const Addr addr) override;
        // ReplaceableEntry* getVictim(
        //     const ReplacementCandidates& candidates) const override;


        // /**
        // * Instantiate a replacement data entry.
        // *
        // * @return A shared pointer to the new replacement data.
        // */
        // std::shared_ptr<ReplacementData> instantiateEntry() override;

};

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_DIP_RP_HH__
