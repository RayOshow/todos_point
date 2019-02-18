#pragma once

class bucky_control {

private:
	name self;
	static uint32_t nonce;

	uint32_t get_random(uint32_t range) {

		eosio_assert(range > 1, BKS_ERROR_MSG_RANDOM_RANGE);

		// Transaction id 
		auto trx_size = transaction_size();
		uint32_t buf_sz = trx_size+4;
   		char buf[buf_sz];
		uint32_t read = read_transaction( buf, trx_size );	
		eosio_assert( trx_size == read, BKS_ERROR_TRANSACTION_FAILED);	

		// nonce 
		nonce = now() + nonce + 1;
		
		// Transaction id + nonce => make source.
		for(int index=trx_size; index < (trx_size+4) ;index++) {
			buf[index] = (nonce >> (index-trx_size)) & 0xFF;
		}

		eosio::checksum256 hash_data = eosio::sha256(buf, buf_sz);

		// range for random val = 0 ~ 2^64
		uint64_t random_value = ((uint64_t)hash_data.extract_as_byte_array()[0] << 56) + ((uint64_t)hash_data.extract_as_byte_array()[1] << 48) + ((uint64_t)hash_data.extract_as_byte_array()[2] << 40);
		random_value += ((uint64_t)hash_data.extract_as_byte_array()[3] << 32) + ((uint64_t)hash_data.extract_as_byte_array()[4] << 24) + ((uint64_t)hash_data.extract_as_byte_array()[5] << 16) ;
		random_value += ((uint64_t)hash_data.extract_as_byte_array()[6] << 8) + (uint64_t)hash_data.extract_as_byte_array()[7];

		return (uint32_t)(random_value % range);			
	}		
		
public:
	bucky_control(name _self) : self(_self) {
	}

	void register_bucky(uint32_t bucky_seq, uint32_t max_participant, const std::vector<uint32_t>& picks) {
		
		require_auth(self);			
			
		buckies bks(self, self.value);
		auto it = bks.find(bucky_seq);
		eosio_assert(it == bks.end(),BKS_ERROR_MSG_ALREADY_EXIST_SEQ);
		eosio_assert(max_participant > 0, BKS_ERROR_MSG_NO_PARTICIPANT);
		
		bks.emplace(self, [&]( auto& row ) {
			row.bucky_seq = bucky_seq;
			row.max_participant = max_participant;
			row.start_time = now();
			row.end_time = 0;

			for(int index = 0; index < picks.size(); index++) {
				// Save lot data
				lotstb lot;
				lot.rank = index + 1;
				lot.count = picks[index];
				row.lots.push_back(lot);   
			}				   
		});				
	}
	
	void join_bucky(uint32_t bucky_seq, uint32_t user_seq) {
	
		require_auth(self);		
		
		buckies bks(self, self.value);
		auto it = bks.find(bucky_seq);
		eosio_assert(it != bks.end(), BKS_ERROR_MSG_NO_FIND_SEQ);

		bks.modify(it, self, [&]( auto& row ) {
			
			eosio_assert(row.max_participant > row.participant_total, BKS_ERROR_MSG_OVER_MAX_PARTICIPANTS);

			for (int index = 0; index < row.participant.size(); index++) {			
				eosio_assert(row.participant[index] != user_seq, BKS_ERROR_MSG_USER_ALREADY_EXISTS);	
			} 
			
			row.participant.push_back(user_seq);	
			row.participant_total++;
		});
	}

	void finish_bucky(uint32_t bucky_seq) {

		require_auth(self);

		buckies bks(self, self.value);
		auto it = bks.find(bucky_seq);
		eosio_assert(it != bks.end(), BKS_ERROR_MSG_NO_FIND_SEQ);

		bks.modify(it, self, [&]( auto& row ) {
			
			eosio_assert(row.end_flag < 1, BKS_ERROR_MSG_ALREADY_END);

			for (int index = 0; index < row.lots.size(); index++) {	
				
				uint32_t winner;

				for (int index2 = 0; index2 < row.lots[index].count; index2++) {

						winner = get_random(row.participant.size());
						
						row.lots[index].winners.push_back(row.participant[winner]);
						row.participant.erase(row.participant.begin()+winner);
				}
			}
			
			row.end_flag = 1;
			row.end_time = now();
		});
	}		

	void delete_info(uint32_t bucky_seq) 
	{
		require_auth(self);

		buckies bks(self, self.value);

		auto it = bks.find(bucky_seq);
		eosio_assert(it != bks.end(), BKS_ERROR_MSG_NO_FIND_SEQ);
		bks.erase(it);
	}

};
