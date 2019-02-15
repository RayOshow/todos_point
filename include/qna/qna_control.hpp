#pragma once

class qna_control {

private:
	name self;
		
	
	int get_hexstr_int(char ch ) {
	
		switch(ch) {

			case 'A':
			case 'a':
				return 10;

			case 'B':
			case 'b':
				return 11;

			case 'C':
			case 'c':
				return 12;

			case 'D':
			case 'd':
				return 13;

			case 'E':
			case 'e':
				return 14;

			case 'F':
			case 'f':
				return 15;
			
		}
	
		return ch - '0';
	}	
	

	uint8_t check_hash(string hash_data, string raw_data) {
		
		char *hstr = new char[hash_data.length() + 1];
		char *rstr = new char[raw_data.length() + 1];
		
		strcpy(hstr, hash_data.c_str());
		strcpy(rstr, raw_data.c_str());
		
		eosio::checksum256 new_hash_data = eosio::sha256(rstr, raw_data.length());
		
		for(int index = 0; index < new_hash_data.extract_as_byte_array().size() ; index++) {
			int hash_index = index * 2;	
			int hash_value = get_hexstr_int(hstr[hash_index]) * 16 +  get_hexstr_int(hstr[hash_index+1]);
			 
			if(hash_value != new_hash_data.extract_as_byte_array()[index]) {
				return 0;
			}
		}
		return 1;			
	}		

	/**
		Send token by eosio.token contract outside.
		It is essential to have "eosio.code" permission.
	**/
	void inline_consume_token(uint32_t user, asset quantity, string memo) {
		action(
			permission_level{self, "active"_n},
			self, 
			"consume"_n,
			std::make_tuple(
				user, 
				quantity, 
				memo)

		).send();
	}

	/**
		Send token by eosio.token contract outside.
		It is essential to have "eosio.code" permission.
	**/
	void inline_refund_token(uint32_t user, asset quantity, string memo) {
		action(
			permission_level{self, "active"_n},
			self, 
			"refund"_n,
			std::make_tuple(
				user, 
				quantity, 
				memo)

		).send();
	}

public:
	qna_control(name _self) : self(_self) {
	}

	void end(uint64_t qna_seq, string enc_pwd)
	{
		require_auth(self);			
			
		qnainfos qfs(self, self.value);

		auto it = qfs.find(qna_seq);
		eosio_assert(it != qfs.end(), QNA_ERROR_MSG_NO_FIND_SEQ);
			
		uint16_t y_cnt;
		uint16_t n_cnt;
		
		string winner_str;				
		
		asset y_total_token;
		asset n_total_token;	

		eosio_assert(enc_pwd.length() > 0, QNA_ERROR_MSG_NO_ENC_PWD);
					
		qfs.modify(it, self, [&]( auto& row ) {	
			eosio_assert((row.end_flag <= 0), QNA_ERROR_MSG_QNA_CLOSED);

			y_total_token = row.std_bet_token;
			y_total_token.amount = 0;
			n_total_token = row.std_bet_token;
			n_total_token.amount = 0;

			for (int index = 0; index < row.qna_vote_result.size(); index++) {
					
				if(check_hash(row.qna_vote_result[index].answer,std::to_string(row.qna_vote_result[index].user_seq)+"Y"+enc_pwd) > 0) {
					y_cnt++;
					y_total_token += row.qna_vote_result[index].bet_token;
					row.qna_vote_result[index].answer = "Y";
				}else {
					n_cnt++;
					n_total_token += row.qna_vote_result[index].bet_token;
					row.qna_vote_result[index].answer = "N";
				}
					
			} 
		
			print(" y_cnt",y_total_token," n_cnt",n_total_token);

			row.end_flag = 1;
									
			if((y_cnt!=0) & (n_cnt!=0) &  (y_cnt > n_cnt)) {
				winner_str = QNA_NEGATIVE_SYMBOL;
			}
			else if((y_cnt!=0) & (n_cnt!=0) & (y_cnt < n_cnt)) {
				winner_str = QNA_POSITIVE_SYMBOL;
			}
			else {
				winner_str = QNA_NO_DECISION_SYMBOL;
				
				// same 
				for (int index = 0; index < row.qna_vote_result.size(); index++) {						
					//send_token(_self, row.qna_vote_result[index].bc_id, row.qna_vote_result[index].bet_token, ");	
					inline_refund_token(row.qna_vote_result[index].user_seq,row.qna_vote_result[index].bet_token, QNA_TIE_COMMENT);
					row.qna_vote_result[index].result = QNA_TIE_SYMBOL;
				} 
				
				return;
			}	

			row.win_symbol = winner_str;
			
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			// Refund money!!!
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////					
			for (int index = 0; index < row.qna_vote_result.size(); index++) {
					
				if (row.qna_vote_result[index].answer == winner_str) {
					
					asset win_token;
																//  																																		
					if(winner_str == QNA_POSITIVE_SYMBOL) {
						row.qna_vote_result[index].win_token = n_total_token * (row.qna_vote_result[index].bet_token / y_total_token);								
					} else {
						row.qna_vote_result[index].win_token = y_total_token * (row.qna_vote_result[index].bet_token / n_total_token);																	
					}
					
					print(" 1: ", row.qna_vote_result[index].win_token);
					// 수수료 3%
					row.qna_vote_result[index].win_token = row.qna_vote_result[index].win_token * QNA_WINNER_PROFIT_RATE;
					print(" 2: ", QNA_WINNER_PROFIT_RATE);
					row.qna_vote_result[index].win_token = row.qna_vote_result[index].bet_token + row.qna_vote_result[index].win_token;
					print(" 3: ", row.qna_vote_result[index].win_token);

					inline_refund_token(row.qna_vote_result[index].user_seq,row.qna_vote_result[index].win_token, QNA_WIN_COMMENT);
					row.qna_vote_result[index].result = QNA_WIN_SYMBOL;
					
				}else {
					row.qna_vote_result[index].result = QNA_LOSE_SYMBOL;									
				}											
			} 
			//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
							
		});
	}

	void response(uint32_t qna_seq, uint32_t user_seq, asset quantity, string answer_str) 
	{
		require_auth(self);

		inline_consume_token(user_seq,quantity,"QNA BET COST!");

		qnainfos qfs(self, self.value);
		auto it = qfs.find(qna_seq);
		
		if (it == qfs.end()) {

			qfs.emplace(self, [&]( auto& row ) {
				row.qna_seq = qna_seq;
				row.end_flag = 0;
				row.std_bet_token = quantity;
				
				// user vote info
				votetb qv;
				qv.user_seq = user_seq;
				qv.answer = answer_str;
				qv.bet_token = quantity;					
				row.qna_vote_result.push_back(qv);     
			});						
		}
		else {

			qfs.modify(it, self, [&]( auto& row ) {
				
				eosio_assert(row.end_flag <= 0, QNA_ERROR_MSG_QNA_CLOSED);

				eosio_assert(row.std_bet_token.symbol == quantity.symbol, QNA_ERROR_MSG_SYMBOL_DIFF);												
				eosio_assert(row.std_bet_token.amount <= quantity.amount, QNA_ERROR_MSG_AMOUNT_IS_LOW);
						 			
				for (int index = 0; index < row.qna_vote_result.size(); index++) {
											
					if (row.qna_vote_result[index].user_seq == user_seq) {
						eosio_assert(false, QNA_ERROR_MSG_ALREADY_VOTE);
						break;
					}
				} 

				row.std_bet_token = quantity;
				
		 	   	votetb qv;
				qv.user_seq = user_seq;
				qv.answer = answer_str;
				qv.bet_token = quantity;
				row.qna_vote_result.push_back(qv);
			});
		}		
	}
	
	void delete_info(uint32_t qna_seq) 
	{
		require_auth(self);

		qnainfos qfs(self, self.value);

		auto it = qfs.find(qna_seq);
		eosio_assert(it != qfs.end(), QNA_ERROR_MSG_NO_FIND_SEQ);
		qfs.erase(it);
	}

};
