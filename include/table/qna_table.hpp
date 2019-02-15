#pragma once
struct  votetb {
	uint32_t user_seq ;	
	string answer;
	asset bet_token;
	asset win_token;			
	string result;		
};

struct [[eosio::table, eosio::contract(CONTRACT_NAME)]] qnainfotb {
	uint64_t qna_seq;
	vector<votetb> qna_vote_result;
	uint8_t end_flag;
	asset std_bet_token;
	string win_symbol;

	uint64_t primary_key() const { return qna_seq; }
	EOSLIB_SERIALIZE(
	    qnainfotb,
	    (qna_seq)
	    (qna_vote_result)
	    (end_flag)
	    (std_bet_token)
	    (win_symbol)
   )    
};

typedef eosio::multi_index< "qnainfo"_n, qnainfotb > qnainfos;
