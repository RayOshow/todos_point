#pragma once

struct [[eosio::table , eosio::contract(CONTRACT_NAME)] ] account {
	uint32_t user_seq; // user seq 
	uint64_t blockchain_id; // eos , side chain , fork chain account value . [a-z, 1~5]
	std::vector<asset> tokens; // token info

	uint32_t primary_key()const { return user_seq ;}

	EOSLIB_SERIALIZE(
			 account,
			 (user_seq)
			 (blockchain_id)
			 (tokens)
			 )
};


typedef eosio::multi_index< "account"_n, account > accounts;
