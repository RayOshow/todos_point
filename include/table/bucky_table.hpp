#pragma once

struct lotstb {
	uint32_t rank ;
	uint32_t count ;
	vector<uint32_t> winners;
};

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] bucky {
	uint32_t bucky_seq; // seq
	uint32_t max_participant;
	uint32_t participant_total;

	vector<uint32_t> participant;
	vector<lotstb> lots;
	
	uint8_t end_flag;
	//uint8_t ovelap_flag;
		
	// time
	uint32_t start_time;
	uint32_t end_time;
		
	uint64_t primary_key() const { return bucky_seq; }
	
	EOSLIB_SERIALIZE(
		bucky,
	    (bucky_seq)
	    (max_participant)
	    (participant_total)
	    (participant)
	    (lots)
	    (end_flag)
		(start_time)
		(end_time)		
	)    
};

typedef eosio::multi_index< "bucky"_n, bucky > buckies;
