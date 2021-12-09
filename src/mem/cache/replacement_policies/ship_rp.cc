/**
 * Copyright (c) 2018 Inria
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met: redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer;
 * redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution;
 * neither the name of the copyright holders nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "mem/cache/replacement_policies/brrip_rp.hh"
#include "mem/cache/replacement_policies/ship_rp.hh"
#include <cassert>
#include <memory>

#include "base/logging.hh" // For fatal_if
#include "base/random.hh"
#include "params/SHIPRP.hh"

SHIPRP::SHIPRP(const Params *p)
    :BRRIPRP(p), shct(p->shctSize,SatCounter(p->shct_num_bits))
{
    fatal_if(numRRPVBits <= 0, "There should be at least one bit per RRPV.\n");
}



//Touch = hit
void
SHIPRP::touch(const std::shared_ptr<ReplacementData>& replacement_data) 
{
		/////////////////////SHiP///////////////////
		
		//Increment entry -- mem
		++shct[replacement_data->signature_m];

		//Increment entry -- pc
		//++shct[replacement_data->signature_pc];
		
		//Has been re-referenced
		replacement_data->outcome = true;
		
		////////////////////SHiP/////////////////
		
    std::shared_ptr<SHIPReplData> casted_replacement_data =
        std::static_pointer_cast<SHIPReplData>(replacement_data);
        

		
    // Update RRPV if not 0 yet
    // Every hit in HP mode makes the entry the last to be evicted, while
    // in FP mode a hit makes the entry less likely to be evicted
    if (hitPriority) {
        casted_replacement_data->rrpv.reset();
    } else {
        casted_replacement_data->rrpv--;
    }
}


//Reset is insertion -- check:make SHiP changes here
/*
Comes here on a miss. Check SHCT[signature] to predict rrpv
*/
void
SHIPRP::reset(const std::shared_ptr<ReplacementData>& replacement_data) const
{
    std::shared_ptr<SHIPReplData> casted_replacement_data =
        std::static_pointer_cast<SHIPReplData>(replacement_data);
        
    //////////////////////////SHiP//////////////
    
    //shct[sig] == 0 => distant re-ref -- mem
    casted_replacement_data->rrpv.saturate();
    if (shct[replacement_data->signature_m] > 0)
    	casted_replacement_data->rrpv--;
    	
     //shct[sig] == 0 => distant re-ref -- pc
    /*casted_replacement_data->rrpv.saturate();
    if (shct[replacement_data->signature_pc] > 0)
    	casted_replacement_data->rrpv--;*/
    
    //////////////////SHiP////////////

    // Mark entry as ready to be used
    casted_replacement_data->valid = true;
}


//Block eviction
ReplaceableEntry*
SHIPRP::getVictim(const ReplacementCandidates& candidates) 
{
    // There must be at least one replacement candidate
    assert(candidates.size() > 0);

    // Use first candidate as dummy victim
    ReplaceableEntry* victim = candidates[0];

    // Store victim->rrpv in a variable to improve code readability
    int victim_RRPV = std::static_pointer_cast<SHIPReplData>(
                        victim->replacementData)->rrpv;

    // Visit all candidates to find victim
    for (const auto& candidate : candidates) {
        std::shared_ptr<SHIPReplData> candidate_repl_data =
            std::static_pointer_cast<SHIPReplData>(
                candidate->replacementData);

        // Stop searching for victims if an invalid entry is found
        if (!candidate_repl_data->valid) {
            return candidate;
        }

        // Update victim entry if necessary
        int candidate_RRPV = candidate_repl_data->rrpv;
        if (candidate_RRPV > victim_RRPV) {
            victim = candidate;
            victim_RRPV = candidate_RRPV;
        }
    }

    // Get difference of victim's RRPV to the highest possible RRPV in
    // order to update the RRPV of all the other entries accordingly
    int diff = std::static_pointer_cast<SHIPReplData>(
        victim->replacementData)->rrpv.saturate();

    // No need to update RRPV if there is no difference
    if (diff > 0){
        // Update RRPV of all candidates
        for (const auto& candidate : candidates) {
            std::static_pointer_cast<SHIPReplData>(
                candidate->replacementData)->rrpv += diff;
        }
    }
    
    ////////////////////SHiP///////////////
    
    //Not been referenced since insertion -- mem
    if (!victim->replacementData->outcome){
        shct[victim->replacementData->signature_m]--;
    }  
    
    //Not been referenced since insertion -- pc
    /*if (!victim->replacementData->outcome){
        shct[victim->replacementData->signature_pc]--;
    } */
    	
    ////SHiP//////////////  

    return victim;
}

//ship -- check
std::shared_ptr<ReplacementData>
SHIPRP::instantiateEntry()
{
    return std::shared_ptr<ReplacementData>(new SHIPReplData(numRRPVBits));
}

SHIPRP*
SHIPRPParams::create()
{
    return new SHIPRP(this);
}
