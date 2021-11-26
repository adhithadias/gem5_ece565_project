#ifndef __MEM_CACHE_REPLACEMENT_POLICIES_DIP_RP_HH__
#define __MEM_CACHE_REPLACEMENT_POLICIES_DIP_RP_HH__

#include <memory>

#include "base/compiler.hh"
#include "base/statistics.hh"
#include "mem/cache/replacement_policies/base.hh"
#include "mem/cache/tags/dueling.hh"

struct DIPRPParams;

class DIPRP : public BaseReplacementPolicy
{
    protected:
        /** DIP-specific implementation of replacement data. */
        struct DIPReplData : ReplacementData, Dueler
        {
            /** Tick on which the entry was last touched. */
            Tick lastTouchTick;
            std::shared_ptr<ReplacementData> replacementData1;
            std::shared_ptr<ReplacementData> replacementData2;

            /**
            * Default constructor. Invalidate data.
            */
            DIPReplData(
                const std::shared_ptr<ReplacementData>& replacement_data_1,
                const std::shared_ptr<ReplacementData>& replacement_data_2)
            :   ReplacementData(), Dueler(),
                lastTouchTick(0), replacementData1(replacement_data_1),
                replacementData2(replacement_data_2)
            {}
        };

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

        /**
        * Invalidate replacement data to set it as the next probable victim.
        * Sets its last touch tick as the starting tick.
        *
        * @param replacement_data Replacement data to be invalidated.
        */
        void invalidate(const std::shared_ptr<ReplacementData>& replacement_data)
                                                                const override;

        /**
        * Touch an entry to update its replacement data.
        * Sets its last touch tick as the current tick.
        *
        * @param replacement_data Replacement data to be touched.
        */
        void touch(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                        override;


        /**
        * Reset replacement data. Used when an entry is inserted.
        * Sets its last touch tick as the current tick.
        *
        * @param replacement_data Replacement data to be reset.
        */
        void reset(const std::shared_ptr<ReplacementData>& replacement_data) const
                                                                        override;       

        /**
        * Find replacement victim using LRU timestamps.
        *
        * @param candidates Replacement candidates, selected by indexing policy.
        * @return Replacement entry to be replaced.
        */
        ReplaceableEntry* getVictim(const ReplacementCandidates& candidates) const
                                                                        override;


        /**
        * Instantiate a replacement data entry.
        *
        * @return A shared pointer to the new replacement data.
        */
        std::shared_ptr<ReplacementData> instantiateEntry() override;

};

#endif // __MEM_CACHE_REPLACEMENT_POLICIES_DIP_RP_HH__
