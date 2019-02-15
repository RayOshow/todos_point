#pragma once

class account_control {

private:
	name self;

public:
	account_control(name _self) : self(_self) {
	}

	void signup(uint32_t user_seq)
	{	 
		require_auth( self );

		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it == acnts.end(), ERROR_MSG_ID_ALREADY_EXIST);

		acnts.emplace( self, [&]( auto& row ){
			row.user_seq = user_seq;
		});
	}
	
	void retire(uint32_t user_seq)
	{	 
		require_auth(self);

		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.erase(it);
	}
	
	void charge_token(uint32_t user_seq, asset quantity) 
	{		
		accounts acnts(self, self.value);
   		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);
		
		acnts.modify(it, self, [&]( auto& row ) {
			int index;
			for(index = 0; index < row.tokens.size(); index++) {
				if(row.tokens[index].symbol.code() == quantity.symbol.code()) {
					row.tokens[index] += quantity;							
					break;
				}
			}											
			
			if(index == row.tokens.size()) {
				row.tokens.push_back(quantity);
			}
		});	
	}	

	void uncharge_token(uint32_t user_seq, asset quantity) 
	{
		require_auth(self);

		accounts acnts(self, self.value);
	   	auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);
		
		acnts.modify(it, self, [&]( auto& row ) {
			int index;
			for(index = 0; index < row.tokens.size(); index++) {
				if(row.tokens[index].symbol.code() == quantity.symbol.code()) {
					eosio_assert( row.tokens[index].amount >= quantity.amount, ERROR_MSG_NO_ENOUGH_TOKEN);
					row.tokens[index].amount -= quantity.amount;
					break;
				}
			}											
			eosio_assert(index != row.tokens.size(), ERROR_MSG_NO_ENOUGH_TOKEN);				
		});	
	}
			
	void set_blockchain_id(name blockchain_id, uint32_t user_seq) {
		
		require_auth(self);

		accounts acnts(self, self.value);

		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto& row ) {														
			row.blockchain_id = blockchain_id.value;
		});
	}

	void withdraw_token(name requestor, uint32_t user_seq, asset quantity) {
		
		require_auth(requestor);
			
		accounts acnts(self, self.value);
		auto it = acnts.find( user_seq );
		eosio_assert( it != acnts.end(), ERROR_MSG_ID_NOT_EXIST);

		acnts.modify(it, self, [&]( auto& row ) {
			// Setting blockcahin id in advance is needed .
			eosio_assert( row.blockchain_id != 0, ERROR_MSG_ID_NOT_EXIST);
			eosio_assert( requestor.value == row.blockchain_id, ERROR_MSG_ID_NOT_MATCH);
			
			int index;
			for(index = 0; index < row.tokens.size(); index++) {
				if(row.tokens[index].symbol.code() == quantity.symbol.code()) {
					eosio_assert( row.tokens[index].amount >= quantity.amount, ERROR_MSG_NO_ENOUGH_TOKEN);
					row.tokens[index].amount -= quantity.amount;
					print(" withdraw: ", row.tokens[index]);
					break;
				}
			}	
			
			eosio_assert(index != row.tokens.size(), ERROR_MSG_NO_ENOUGH_TOKEN);	
		});				
	}	
};
