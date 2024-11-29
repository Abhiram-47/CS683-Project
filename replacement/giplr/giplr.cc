#include <algorithm>
#include <cassert>
#include <map>
#include <utility>

#include "cache.h"
// #include <unordered_map>

namespace
{
constexpr int ipv[8][2][17] = {
							  { {0, 0, 0, 0, 1, 3, 4, 3, 0, 2, 3, 0, 7, 3, 1, 15, 0},
                              {0, 0, 1, 1, 1, 4, 3, 0, 4, 0, 3, 5, 4, 10, 12, 15, 15} },

							  { {0, 0, 0, 0, 1, 2, 3, 2, 4, 0, 2, 4, 8, 0, 1, 15, 14},
                              {0, 0, 0, 0, 1, 4, 2, 4, 5, 5, 10, 3, 12, 13, 7, 15, 15} },

							  { {0, 0, 1, 3, 0, 0, 5, 0, 6, 3, 0, 6, 6, 12, 8, 7, 10},
                              {0, 0, 1, 2, 1, 0, 2, 0, 2, 0, 1, 10, 11, 13, 10, 10, 13} },

							  { {0, 0, 0, 0, 1, 0, 0, 6, 0, 5, 0, 9, 6, 5, 1, 14, 15},
                              {0, 0, 0, 0, 1, 5, 3, 5, 7, 2, 0, 9, 0, 4, 1, 15, 15} },

							  { {0, 0, 0, 0, 2, 2, 3, 0, 1, 0, 7, 5, 6, 4, 2, 11, 12},
                              {0, 0, 0, 0, 0, 2, 2, 0, 1, 3, 5, 2, 5, 3, 2, 15, 15} },

							  { {0, 0, 2, 0, 3, 5, 3, 0, 6, 9, 7, 3, 6, 4, 4, 10, 13},
                              {0, 0, 0, 1, 1, 2, 2, 1, 2, 0, 0, 9, 10, 12, 1, 13, 15} },

							  { {0, 0, 0, 1, 1, 4, 4, 5, 0, 2, 2, 5, 7, 5, 1, 15, 15},
                              {0, 0, 0, 0, 4, 0, 1, 6, 6, 7, 4, 11, 10, 10, 2, 5, 11} },

							  { {0, 0, 2, 3, 1, 0, 5, 6, 3, 7, 2, 5, 0, 9, 4, 7, 2},
                              {0, 0, 0, 1, 4, 2, 1, 5, 3, 4, 1, 7, 1, 5, 9, 15, 0} }			
							};


// constexpr int ipv1[2][17] = { {0, 0, 0, 0, 1, 3, 4, 3, 0, 2, 3, 0, 7, 3, 1, 15, 0},
//                               {0, 0, 1, 1, 1, 4, 3, 0, 4, 0, 3, 5, 4, 10, 12, 15, 15} };
// constexpr int ipv2[2][17] = { {0, 0, 0, 0, 1, 2, 3, 2, 4, 0, 2, 4, 8, 0, 1, 15, 14},
//                               {0, 0, 0, 0, 1, 4, 2, 4, 5, 5, 10, 3, 12, 13, 7, 15, 15} };
// constexpr int ipv3[2][17] = { {0, 0, 1, 3, 0, 0, 5, 0, 6, 3, 0, 6, 6, 12, 8, 7, 10},
//                               {0, 0, 1, 2, 1, 0, 2, 0, 2, 0, 1, 10, 11, 13, 10, 10, 13} };
// constexpr int ipv4[2][17] = { {0, 0, 0, 0, 1, 0, 0, 6, 0, 5, 0, 9, 6, 5, 1, 14, 15},
//                               {0, 0, 0, 0, 1, 5, 3, 5, 7, 2, 0, 9, 0, 4, 1, 15, 15} };
// constexpr int ipv5[2][17] = { {0, 0, 0, 0, 2, 2, 3, 0, 1, 0, 7, 5, 6, 4, 2, 11, 12},
//                               {0, 0, 0, 0, 0, 2, 2, 0, 1, 3, 5, 2, 5, 3, 2, 15, 15} };
// constexpr int ipv6[2][17] = { {0, 0, 2, 0, 3, 5, 3, 0, 6, 9, 7, 3, 6, 4, 4, 10, 13},
//                               {0, 0, 0, 1, 1, 2, 2, 1, 2, 0, 0, 9, 10, 12, 1, 13, 15} };
// constexpr int ipv7[2][17] = { {0, 0, 0, 1, 1, 4, 4, 5, 0, 2, 2, 5, 7, 5, 1, 15, 15},
//                               {0, 0, 0, 0, 4, 0, 1, 6, 6, 7, 4, 11, 10, 10, 2, 5, 11} };
// constexpr int ipv8[2][17] = { {0, 0, 2, 3, 1, 0, 5, 6, 3, 7, 2, 5, 0, 9, 4, 7, 2},
//                               {0, 0, 0, 1, 4, 2, 1, 5, 3, 4, 1, 7, 1, 5, 9, 15, 0} };


constexpr std::size_t NUM_POLICY = 8;
constexpr std::size_t SDM_SIZE = 32;
constexpr std::size_t TOTAL_SDM_SETS = NUM_POLICY * SDM_SIZE;       // This replacement is implemented only for single core systems
constexpr unsigned PSEL_WIDTH = 12;
constexpr unsigned COUNTER_MAX = 2047;

std::map<CACHE*, std::vector<std::size_t>> rand_sets;
std::map<CACHE*, std::vector<unsigned>> recency;
std::map<CACHE*, std::vector<unsigned>> counter;

} // namespace

// initialize replacement state
void CACHE::initialize_replacement() { 
	// randomly selected sampler sets
	std::size_t rand_seed = 1103515245 + 12345;
	for (std::size_t i = 0; i < ::TOTAL_SDM_SETS; i++) {
		std::size_t val = (rand_seed / 65536) % NUM_SET;
		auto loc = std::lower_bound(std::begin(::rand_sets[this]), std::end(::rand_sets[this]), val);

		while (loc != std::end(::rand_sets[this]) && *loc == val) {
			rand_seed = rand_seed * 1103515245 + 12345;
			val = (rand_seed / 65536) % NUM_SET;
			loc = std::lower_bound(std::begin(::rand_sets[this]), std::end(::rand_sets[this]), val);
    	}

		::rand_sets[this].insert(loc, val);
	}

	::recency.insert({this, std::vector<unsigned>(NUM_SET * NUM_WAY)});
	::counter.insert({this, std::vector<unsigned>(NUM_POLICY, 0)});

	for (std::size_t i = 0; i < NUM_SET; i++)
		for (std::size_t j = 0; j < NUM_WAY; j++)
			::recency[this][i * NUM_WAY + j] = j;
}

// find replacement victim
uint32_t CACHE::find_victim(uint32_t triggering_cpu, uint64_t instr_id, uint32_t set, const BLOCK* current_set, uint64_t ip, uint64_t full_addr, uint32_t type)
{
	// look for the max recency line
	auto begin = std::next(std::begin(::recency[this]), set * NUM_WAY);
	auto end = std::next(begin, NUM_WAY);

	auto victim = std::max_element(begin, end);


	assert(begin <= victim);
	assert(victim < end);
	assert(*victim == NUM_WAY - 1);
	return static_cast<uint32_t>(std::distance(begin, victim)); // cast protected by assertions
}

// called on every cache hit and cache fill
void CACHE::update_replacement_state(uint32_t triggering_cpu, uint32_t set, uint32_t way, uint64_t full_addr, uint64_t ip, uint64_t victim_addr, uint32_t type,
                                     uint8_t hit)
{

	auto begin = ::rand_sets[this].begin();
	auto end = ::rand_sets[this].end();
	auto leader = std::find(begin, end, set);

	if (leader == end) {			// follower set
		int g = min_element(::counter[this].begin(), ::counter[this].end()) - ::counter[this].begin();

		bool prefetched = block[set * NUM_WAY + way].prefetch;

		int k = prefetched ? 1 : 0;

		if (hit) {
			std::size_t i = ::recency[this][set * NUM_WAY + way];
			std::size_t newi = ipv[g][k][i];
			if (i < newi) {
				for (std::size_t l = 0; l < NUM_WAY; l++) {
					if (::recency[this][set * NUM_WAY + l] > i && ::recency[this][set * NUM_WAY + l] <= newi)
						::recency[this][set * NUM_WAY + l]--;
				}
			} else if (i > newi) {
				for (std::size_t l = 0; l < NUM_WAY; l++) {
					if (::recency[this][set * NUM_WAY + l] < i && ::recency[this][set * NUM_WAY + l] >= newi)
						::recency[this][set * NUM_WAY + l]++;
				}
			}
			::recency[this][set * NUM_WAY + way] = newi;
			return;
		}

		else {
			for (std::size_t l = 0; l < NUM_WAY; l++) {
				if (::recency[this][set * NUM_WAY + l] >= ipv[g][k][16] && ::recency[this][set * NUM_WAY + l] < NUM_WAY - 1)
					::recency[this][set * NUM_WAY + l]++;
			}
			::recency[this][set * NUM_WAY + way] = ipv[g][k][16];
			return;
		}
	}
	
	else {							// leader set
		int g = distance(begin, leader) / SDM_SIZE;			// group

		bool prefetched = block[set * NUM_WAY + way].prefetch;
		
		int k = prefetched ? 1 : 0;

		if (hit) {
			std::size_t i = ::recency[this][set * NUM_WAY + way];
			std::size_t newi = ipv[g][k][i];
			if (i < newi) {
				for (std::size_t l = 0; l < NUM_WAY; l++) {
					if (::recency[this][set * NUM_WAY + l] > i && ::recency[this][set * NUM_WAY + l] <= newi)
						::recency[this][set * NUM_WAY + l]--;
				}
			} else if (i > newi) {
				for (std::size_t l = 0; l < NUM_WAY; l++) {
					if (::recency[this][set * NUM_WAY + l] < i && ::recency[this][set * NUM_WAY + l] >= newi)
						::recency[this][set * NUM_WAY + l]++;
				}
			}
			::recency[this][set * NUM_WAY + way] = newi;
			return;
		}
			
		else {
			::counter[this][g]++;

			if (::counter[this][g] == COUNTER_MAX) {
				for (std::size_t l = 0; l < NUM_POLICY; l++) {
					::counter[this][l] = (::counter[this][l] >> 1);
				}
			}

			for (std::size_t l = 0; l < NUM_WAY; l++) {
				if (::recency[this][set * NUM_WAY + l] >= ipv[g][k][16] && ::recency[this][set * NUM_WAY + l] < NUM_WAY - 1)
					::recency[this][set * NUM_WAY + l]++;
			}
			::recency[this][set * NUM_WAY + way] = ipv[g][k][16];
			return;
		}
	}
}

// use this function to print out your own stats at the end of simulation
void CACHE::replacement_final_stats() {}
